#pragma once

#include "sal/config.h"

#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "comphelper/configuration.hxx"

namespace officecfg
{
namespace ucb
{
namespace Configuration
{
struct ContentProviders : public comphelper::ConfigurationSet<ContentProviders>
{
    static OUString path()
    {
        return OUString("/org.openoffice.ucb.Configuration/ContentProviders");
    }

private:
    ContentProviders(); // not defined
    ~ContentProviders(); // not defined
};
}
}
}
