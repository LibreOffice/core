#pragma once

#pragma warning(push, 1) /* disable warnings within system headers */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#pragma warning(pop)

#include <string>

std::string GetOfficeInstallationPath(MSIHANDLE handle);
std::string GetOfficeProductName(MSIHANDLE handle);
std::string GetQuickstarterLinkName(MSIHANDLE handle);
std::string GetProcessImagePath( DWORD dwProcessId );
