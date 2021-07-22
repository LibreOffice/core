#pragma once

#include "sal/config.h"

#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "comphelper/configuration.hxx"

namespace officecfg
{
namespace TypeDetection
{
namespace Misc
{
struct FrameLoaders : public comphelper::ConfigurationSet<FrameLoaders>
{
    static OUString path() { return OUString("/org.openoffice.TypeDetection.Misc/FrameLoaders"); }

private:
    FrameLoaders(); // not defined
    ~FrameLoaders(); // not defined
};

struct ContentHandlers : public comphelper::ConfigurationSet<ContentHandlers>
{
    static OUString path()
    {
        return OUString("/org.openoffice.TypeDetection.Misc/ContentHandlers");
    }

private:
    ContentHandlers(); // not defined
    ~ContentHandlers(); // not defined
};

struct Defaults : public comphelper::ConfigurationGroup<Defaults>
{
    static OUString path() { return OUString("/org.openoffice.TypeDetection.Misc/Defaults"); }

    struct DefaultFrameLoader
        : public comphelper::ConfigurationProperty<DefaultFrameLoader, OUString>
    {
        static OUString path()
        {
            return OUString("/org.openoffice.TypeDetection.Misc/Defaults/DefaultFrameLoader");
        }

    private:
        DefaultFrameLoader(); // not defined
        ~DefaultFrameLoader(); // not defined
    };

    struct ShowAlienFilterWarning
        : public comphelper::ConfigurationProperty<ShowAlienFilterWarning, bool>
    {
        static OUString path()
        {
            return OUString("/org.openoffice.TypeDetection.Misc/Defaults/ShowAlienFilterWarning");
        }

    private:
        ShowAlienFilterWarning(); // not defined
        ~ShowAlienFilterWarning(); // not defined
    };

private:
    Defaults(); // not defined
    ~Defaults(); // not defined
};
}
}
}
