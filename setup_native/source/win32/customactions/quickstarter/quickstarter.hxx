#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <string>

std::string GetOfficeInstallationPath(MSIHANDLE handle);
std::string GetOfficeProductName(MSIHANDLE handle);
std::string GetProcessImagePath( DWORD dwProcessId );
