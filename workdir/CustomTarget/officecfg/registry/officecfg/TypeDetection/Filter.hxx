#pragma once

#include "sal/config.h"

#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "comphelper/configuration.hxx"

namespace officecfg
{
namespace TypeDetection
{
namespace Filter
{
struct Filters : public comphelper::ConfigurationSet<Filters>
{
    static OUString path() { return OUString("/org.openoffice.TypeDetection.Filter/Filters"); }

private:
    Filters(); // not defined
    ~Filters(); // not defined
};
}
}
}
