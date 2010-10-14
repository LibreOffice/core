/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef VOSAPP_HXX
#define VOSAPP_HXX

#include <sal/main.h>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <vos/process.hxx>

// Mininmal vcl/svapp compatibility without vcl dependence
class Application
{
public:
    USHORT GetCommandLineParamCount();
    XubString GetCommandLineParam( USHORT nParam );
    virtual void Main() = 0;
};

// Urg: Cut & Paste from svapp.cxx: we don't want to depend on vcl
USHORT Application::GetCommandLineParamCount()
{
    vos::OStartupInfo aStartInfo;
    return (USHORT)aStartInfo.getCommandArgCount();
}

XubString Application::GetCommandLineParam( USHORT nParam )
{
    vos::OStartupInfo aStartInfo;
    rtl::OUString aParam;
    aStartInfo.getCommandArg( nParam, aParam );
    return XubString( aParam );
}

#endif /* VOSAPP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
