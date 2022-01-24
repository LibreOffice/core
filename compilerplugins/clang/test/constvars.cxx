/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined _WIN32 //TODO, see corresponding TODO in compilerplugins/clang/writeonlyvars.cxx
// expected-no-diagnostics
#else

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <map>
#include <list>
#include <vector>
#include <rtl/ustring.hxx>

namespace test1
{
int const aFormalArgs[] = { 1, 2 };
// expected-error@+1 {{var can be const [loplugin:constvars]}}
static sal_uInt16 nMediaArgsCount = std::size(aFormalArgs);
sal_uInt16 foo()
{
    (void)aFormalArgs;
    return nMediaArgsCount;
}
};

// no warning expected
namespace test2
{
static char const* ar[] = { "xxxx" };
static const char* lcl_DATA_OTHERS = "localedata_others";
void foo()
{
    (void)ar;
    (void)lcl_DATA_OTHERS;
}
};

// no warning expected
namespace test3
{
static sal_uInt16 nMediaArgsCount = 1; // loplugin:constvars:ignore
sal_uInt16 foo() { return nMediaArgsCount; }
};

// no warning expected, we don't handle these destructuring assignments properly yet
namespace test4
{
void foo()
{
    std::map<OUString, OUString> aMap;
    for (auto & [ rName, rEntry ] : aMap)
    {
        rEntry.clear();
    }
}
};

// no warning expected
namespace test5
{
struct Struct1
{
};
void release(Struct1*);
void foo(std::list<Struct1*> aList)
{
    for (Struct1* pItem : aList)
    {
        release(pItem);
    }
}
};

// no warning expected
namespace test6
{
void foo(std::vector<std::vector<int>> aVecVec)
{
    for (auto& rVec : aVecVec)
        for (auto& rElement : rVec)
            rElement = 1;
}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
