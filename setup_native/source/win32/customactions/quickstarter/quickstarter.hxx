/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#pragma once

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>

std::string GetOfficeInstallationPath(MSIHANDLE handle);
std::string GetOfficeProductName(MSIHANDLE handle);
std::string GetQuickstarterLinkName(MSIHANDLE handle);
std::string GetProcessImagePath( DWORD dwProcessId );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
