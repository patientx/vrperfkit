#include "config.h"

#include "logging.h"
#include "yaml-cpp/yaml.h"

#include <fstream>

namespace fs = std::filesystem;

namespace vrperfkit {
	namespace {
		UpscaleMethod MethodFromString(std::string s) {
			std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
			if (s == "fsr") {
				return UpscaleMethod::FSR;
			}
			if (s == "nis") {
				return UpscaleMethod::NIS;
			}
			LOG_INFO << "Unknown upscaling method " << s << ", defaulting to FSR";
			return UpscaleMethod::FSR;
		}

		std::string MethodToString(UpscaleMethod method) {
			switch (method) {
			case UpscaleMethod::FSR:
				return "FSR";
			case UpscaleMethod::NIS:
				return "NIS";
			}
		}

		std::string PrintToggle(bool toggle) {
			return toggle ? "enabled" : "disabled";
		}
	}

	Config g_config;

	void LoadConfig(const fs::path &configPath) {
		g_config = Config();

		if (!exists(configPath)) {
			LOG_ERROR << "Config file not found, falling back to defaults";
			return;
		}

		try {
			std::ifstream cfgFile (configPath);
			YAML::Node cfg = YAML::Load(cfgFile);

			YAML::Node upscaleCfg = cfg["upscaling"];
			UpscaleConfig &upscaling= g_config.upscaling;
			upscaling.enabled = upscaleCfg["enabled"].as<bool>(upscaling.enabled);
			upscaling.method = MethodFromString(upscaleCfg["method"].as<std::string>(MethodToString(upscaling.method)));
			upscaling.renderScale = upscaleCfg["renderScale"].as<float>(upscaling.renderScale);
			upscaling.sharpness = upscaleCfg["sharpness"].as<float>(upscaling.sharpness);
			upscaling.radius = upscaleCfg["radius"].as<float>(upscaling.radius);
			upscaling.applyMipBias = upscaleCfg["applyMipBias"].as<bool>(upscaling.applyMipBias);

			g_config.debugMode = cfg["debugMode"].as<bool>(g_config.debugMode);
		}
		catch (const YAML::Exception &e) {
			LOG_ERROR << "Failed to load configuration file: " << e.msg;
		}
	}

	void PrintCurrentConfig() {
		LOG_INFO << "Current configuration:";
		LOG_INFO << "  Upscaling (" << MethodToString(g_config.upscaling.method) << ") is " << PrintToggle(g_config.upscaling.enabled);
		if (g_config.upscaling.enabled) {
			LOG_INFO << "    * Render scale: " << std::setprecision(2) << g_config.upscaling.renderScale;
			LOG_INFO << "    * Sharpness:    " << std::setprecision(2) << g_config.upscaling.sharpness;
			LOG_INFO << "    * Radius:       " << std::setprecision(2) << g_config.upscaling.radius;
			LOG_INFO << "    * MIP bias:     " << PrintToggle(g_config.upscaling.applyMipBias);
		}
		LOG_INFO << "  Debug mode is " << PrintToggle(g_config.debugMode);
		FlushLog();
	}
}
