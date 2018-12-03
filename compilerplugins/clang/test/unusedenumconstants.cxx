/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <o3tl/typed_flags_set.hxx>

namespace test1
{
void test1()
{
    enum NameClashMode
    {
        NONE,
        NO_CLASH // expected-error {{write NO_CLASH [loplugin:unusedenumconstants]}}
    };
    NameClashMode eNameClashMode = NO_CLASH;
    (void)eNameClashMode;
}
};

enum class BrowseMode
{
    Modules = 0x01, // expected-error {{read Modules [loplugin:unusedenumconstants]}}
    Top = 0x02, // expected-error {{write Top [loplugin:unusedenumconstants]}}
    Bottom = 0x04, // expected-error {{read Bottom [loplugin:unusedenumconstants]}}
    Left = 0x04, // expected-error {{write Left [loplugin:unusedenumconstants]}}
};
namespace o3tl
{
template <> struct typed_flags<BrowseMode> : is_typed_flags<BrowseMode, 0xf>
{
};
}
BrowseMode g_flags;
int test2(BrowseMode nMode)
{
    if (nMode & BrowseMode::Modules)
        return 1;
    g_flags |= BrowseMode::Top;
    return 0;
}
bool test2b(BrowseMode nMode) { return bool(nMode & BrowseMode::Bottom); }
BrowseMode test2c() { return BrowseMode::Left; }

enum class Enum3
{
    One = 0x01, // expected-error {{write One [loplugin:unusedenumconstants]}}
    Two = 0x02 // expected-error {{write Two [loplugin:unusedenumconstants]}}
};
namespace o3tl
{
template <> struct typed_flags<Enum3> : is_typed_flags<Enum3, 0x3>
{
};
}
void test3_foo(Enum3);
void test3() { test3_foo(Enum3::One | Enum3::Two); }

namespace test4
{
enum Enum4
{
    ONE, // expected-error {{write ONE [loplugin:unusedenumconstants]}}
    TWO
};
struct Test4Base
{
    Test4Base(Enum4) {}
};
struct Test4 : public Test4Base
{
    Test4()
        : Test4Base(Enum4::ONE)
    {
    }
};
};

//-----------------------------------------------------------------------------------

// check that conditional operator walks up the tree
namespace test5
{
enum Enum
{
    ONE, // expected-error {{write ONE [loplugin:unusedenumconstants]}}
    TWO // expected-error {{write TWO [loplugin:unusedenumconstants]}}
};

Enum foo(int x) { return x == 1 ? Enum::ONE : Enum::TWO; }
};

//-----------------------------------------------------------------------------------
// Ignore a common pattern that does not introduce any new information, merely removes
// information.
enum class Enum6
{
    Modules = 0x01, // expected-error {{write Modules [loplugin:unusedenumconstants]}}
    Top = 0x02,
};
namespace o3tl
{
template <> struct typed_flags<Enum6> : is_typed_flags<Enum6, 0x03>
{
};
}
void test6()
{
    Enum6 foo = Enum6::Modules;
    foo &= ~Enum6::Top;
    foo &= (~Enum6::Top);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
