/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#pragma clang diagnostic ignored "-Wunknown-warning-option" // for Clang < 13
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

namespace test1
{
void f(OUString s1)
{
    OUString s2 = s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
    s2 = s2.getToken(0, '(');
}
}

namespace test2
{
OUString s2;
void f(OUString s1)
{
    s2 = s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
    s2 = s2.getToken(0, '(');
}
}

namespace test3
{
struct VolumeInfo
{
    OUString getFileSystemName();
};
OUString aNullURL("");
void f(VolumeInfo _aVolumeInfo)
{
    OUString aFileSysName(aNullURL);
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
    aFileSysName = _aVolumeInfo.getFileSystemName();
}
}

namespace test4
{
struct B3DVector
{
    B3DVector getPerpendicular();
};
void f(B3DVector aNewVPN)
{
    B3DVector aNewToTheRight = aNewVPN;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
    aNewToTheRight = aNewToTheRight.getPerpendicular();
}
}

namespace test5
{
void f(OUString s, int x)
{
    int nPos = x;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
    nPos = s.indexOf("x", nPos);
}
}

namespace test6
{
void f()
{
    OUString s2("xxxx");
    s2 = s2.getToken(0, '('); // no warning expected
}
}

namespace test7
{
class Class1
{
    OUString m_field1;
    void foo(Class1& rOther)
    {
        OUString s = rOther.m_field1;
        // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:sequentialassign]}}
        s = s.copy(1, 2);
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
