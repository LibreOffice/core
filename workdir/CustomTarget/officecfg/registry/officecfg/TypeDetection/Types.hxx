#pragma once

#include "sal/config.h"

#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "comphelper/configuration.hxx"

namespace officecfg
{
namespace TypeDetection
{
namespace Types
{
struct Types : public comphelper::ConfigurationSet<Types>
{
    static OUString path() { return OUString("/org.openoffice.TypeDetection.Types/Types"); }

private:
    Types(); // not defined
    ~Types(); // not defined
};
}
}
}
