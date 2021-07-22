#pragma once

#include "sal/config.h"

#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "comphelper/configuration.hxx"

namespace officecfg
{
namespace TypeDetection
{
namespace UISort
{
struct ModuleDependendFilterOrder : public comphelper::ConfigurationSet<ModuleDependendFilterOrder>
{
    static OUString path()
    {
        return OUString("/org.openoffice.TypeDetection.UISort/ModuleDependendFilterOrder");
    }

private:
    ModuleDependendFilterOrder(); // not defined
    ~ModuleDependendFilterOrder(); // not defined
};
}
}
}
