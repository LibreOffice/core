/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
// expected-note@rtl/ustring.hxx:* 2 {{the presumed corresponding negated operator is declared here [loplugin:simplifybool]}}
#include <rtl/string.hxx>
// expected-note@rtl/string.hxx:* {{the presumed corresponding negated operator is declared here [loplugin:simplifybool]}}
#include <basegfx/vector/b3dvector.hxx>
// expected-note@basegfx/tuple/b3dtuple.hxx:* {{the presumed corresponding negated operator is declared here [loplugin:simplifybool]}}

#include <map>

namespace group1
{
void f1(int a, int b)
{
    if (!(a < b))
    { // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
        a = b;
    }
};

void f2(float a, float b)
{
    // no warning expected
    if (!(a < b))
    {
        a = b;
    }
};
};

// Consistently either warn about all or none of the below occurrences of "!!":
namespace group2
{
enum E1
{
    E1_1 = 1
};

enum E2
{
    E2_1 = 1
};
E2 operator&(E2 e1, E2 e2);
bool operator!(E2 e);

enum class E3
{
    E1 = 1
};
struct W
{
    operator bool();
};
W operator&(E3 e1, E3 e2);

bool f0(int n) { return !!(n & 1); }

bool f1(E1 e) { return !!(e & E1_1); }

bool f2(E2 e) { return !!(e & E2_1); }

bool f3(E3 e) { return !!(e & E3::E1); }
};

// record types
namespace group3
{
struct Record1
{
    bool operator==(const Record1&) const;
};

struct Record2
{
    bool operator==(const Record2&) const;
    bool operator!=(const Record2&) const;
    // expected-note@-1 {{the presumed corresponding negated operator is declared here [loplugin:simplifybool]}}
};

struct Record3
{
};

bool operator==(const Record3&, const Record3&);
bool operator!=(const Record3&, const Record3&);
// expected-note@-1 {{the presumed corresponding negated operator is declared here [loplugin:simplifybool]}}

void testRecord()
{
    Record1 a1;
    Record1 a2;
    // no warning expected, because a negated operator does not exist
    bool v = !(a1 == a2);
    Record2 b1;
    Record2 b2;
    v = !(b1 == b2);
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
    Record3 c1;
    Record3 c2;
    v = !(c1 == c2);
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
    OUString d1;
    OUString d2;
    v = !(d1 == d2);
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
    OString e1;
    OString e2;
    v = !(e1 == e2);
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}

    // the operator != is in a base-class, and the param is a base-type
    basegfx::B3DVector f1;
    basegfx::B3DVector f2;
    v = !(f1 == f2);
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
}

struct Record4
{
    bool operator==(Record4 const&) const;
    bool operator!=(Record4 const& other) const
    {
        // no warning expected
        bool v = !operator==(other);
        v = !(*this == other);
        OUString c1;
        OUString c2;
        v = !(c1 == c2);
        // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
        return v;
    }
};
};

namespace group4
{
bool foo1(bool a, bool b)
{
    return !(!a && !b);
    // expected-error@-1 {{logical negation of logical op containing negation, can be simplified [loplugin:simplifybool]}}
}
bool foo2(int a, bool b)
{
    return !(a != 1 && !b);
    // expected-error@-1 {{logical negation of logical op containing negation, can be simplified [loplugin:simplifybool]}}
}
bool foo3(int a, bool b)
{
    // no warning expected
    return !(a != 1 && b);
}
};

namespace group5
{
bool foo1(std::map<int, int>* pActions, int aKey)
{
    auto aIter = pActions->find(aKey);
    //TODO this doesn't work yet because I'd need to implement conversion operators during method/func lookup
    return !(aIter == pActions->end());
    // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
