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
    sal_uInt16 GetCommandLineParamCount();
    XubString GetCommandLineParam( sal_uInt16 nParam );
    virtual void Main() = 0;
};

// Urg: Cut & Paste from svapp.cxx: we don't want to depend on vcl
sal_uInt16 Application::GetCommandLineParamCount()
{
    vos::OStartupInfo aStartInfo;
    return (sal_uInt16)aStartInfo.getCommandArgCount();
}

XubString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    vos::OStartupInfo aStartInfo;
    rtl::OUString aParam;
    aStartInfo.getCommandArg( nParam, aParam );
    return XubString( aParam );
}

#endif /* VOSAPP_HXX */
