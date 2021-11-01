/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace Enum
{
namespace
{
struct S
{
    enum E1
    {
        E11,
        E12
    };
    E1 e1;
    enum E2
    {
        E21,
        E22
    };
    E2 e2; // expected-error {{unused class member [loplugin:unusedmember]}}
    enum E3
    {
        E31,
        E32
    } e3;
    enum E4
    {
        E41,
        E42
    } e4; // expected-error {{unused class member [loplugin:unusedmember]}}
};
}
void f(S s)
{
    (void)s.e1;
    (void)s.e3;
}
}

namespace ElaboratedEnum
{
namespace
{
struct S
{
    S()
    {
        enum E1 e1 = E11;
        (void)e1;
    }
    enum E1
    {
        E11,
        E12
    };
    enum E2
    {
        E21,
        E22
    };
    enum E2 e2;
};
}
void f()
{
    S s;
    (void)s;
    (void)s.e2;
}
}

namespace UnusedEnum
{
namespace
{
struct S
{
    enum E // expected-error {{unused class member [loplugin:unusedmember]}}
    {
        E1,
        E2
    };
};
}
void f() { (void)S::E1; }
}

namespace UnusedDataMember
{
namespace
{
struct NT
{
    NT(int = 0) {}
    ~NT() {}
};
struct __attribute__((warn_unused)) T
{
    T(int = 0) {}
    ~T() {}
};
struct S
{
    int i1;
    int i2; // expected-error {{unused class member [loplugin:unusedmember]}}
    int const& i3; // expected-error {{unused class member [loplugin:unusedmember]}}
    NT nt;
    T t1;
    T t2; // expected-error {{unused class member [loplugin:unusedmember]}}
    T const& t3; // expected-error {{unused class member [loplugin:unusedmember]}}
    S()
        : i1(0)
        , i3(i1)
        , t1(0)
        , t3(t1)
    {
        (void)i1;
        (void)t1;
    }
};
}
void f()
{
    S s;
    (void)s;
}
}

namespace Alignof
{
namespace
{
struct S
{
    int i;
};
}
void f() { (void)alignof(S const(&)[][10]); }
}

namespace Aligned
{
namespace
{
struct S1
{
    int i;
};
struct S2
{
    int i __attribute__((aligned(__alignof__(S1))));
};
}
void f()
{
    S2 s;
    s.i = 0;
}
}

namespace Bases
{
namespace
{
struct S1
{
    int i1;
};
struct S2 : S1
{
    int i2;
};
struct S3 : S2
{
};
}
void f() { (void)sizeof(S3); }
}

namespace Unnamed
{
namespace
{
struct S
{
    struct
    {
        struct
        {
            int i;
        } s2;
        struct // anonymous struct extension (widely supported)
        {
            int j;
        };
        int k;
    } s1;
#if false //TODO: see corresponding TODO in compilerplugins/clang/unusedmember.cxx
    static constexpr struct
    {
        int l; // expected-error {{unused class member [loplugin:unusedmember]}}
    } s = {};
#endif
    typedef struct
    {
        int m; // expected-error {{unused class member [loplugin:unusedmember]}}
    } t; // expected-error {{unused class member [loplugin:unusedmember]}}
};
}
void f()
{
    (void)sizeof(S);
#if false //TODO: see corresponding TODO in compilerplugins/clang/unusedmember.cxx
    (void)S::s; // avoid "unused variable 's'" (non-loplugin) warning
#endif
}
}

int main()
{
    (void)&Enum::f;
    (void)&ElaboratedEnum::f;
    (void)&UnusedEnum::f;
    (void)&UnusedDataMember::f;
    (void)&Alignof::f;
    (void)&Aligned::f;
    (void)&Bases::f;
    (void)&Unnamed::f;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
