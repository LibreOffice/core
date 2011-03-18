/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef VOSAPP_HXX
#define VOSAPP_HXX

#include <sal/main.h>
#include <tools/solar.h>
#include <tools/string.hxx>

// Mininmal vcl/svapp compatibility without vcl dependence
class Application
{
public:
    sal_uInt16 GetCommandLineParamCount();
    XubString GetCommandLineParam( sal_uInt16 nParam );
    virtual void Main() = 0;
};

// Urg: Cut & Paste from svapp.cxx: we don't want to depend on vcl
sal_uInt16 Application::GetCommandLineParamCount()
{
    return osl_getCommandArgCount();
}

XubString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    rtl::OUString aParam;
    osl_getCommandArg( nParam, &aParam.pData );
    return XubString( aParam );
}

#endif /* VOSAPP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
