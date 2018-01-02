/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <sys/time.h>
#include <o3tl/cow_wrapper.hxx>
#include <vector>

struct S1 {
    OUString mv1;
    OUString const & get() const { return mv1; }
    OUString const & get2(bool) const { return mv1; }
};
struct S2 {
    OUString mv1;
    OUString mv2;
    OUString mv3[2];
    S1 child;
    static OUString gs1;
    o3tl::cow_wrapper<S1> mxCow;

    // make sure we ignore cases where the passed in parameter is std::move'd
    S2(OUString v1, OUString v2)
      : mv1(std::move(v1)), mv2((std::move(v2))) {}

    OUString get1() { return mv1; } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString get2(bool b) { return b ? mv1 : mv2; } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString get3() { return child.mv1; } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString get4() { return mv3[0]; } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString get5() { return gs1; } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString const & get6() { return gs1; }
    OUString get7() { return get6(); } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString & get8() { return gs1; }
    OUString get9() { return get8(); } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    // TODO
    OUString get10() { return OUString(*&get6()); } // todoexpected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    OUString get11() const { return mxCow->get(); } // expected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}
    // TODO anything takes a param is suspect because it might return the param by ref
    OUString get12() { return child.get2(false); } // todoexpected-error {{rather return class rtl::OUString by const& than by value, to avoid unnecessary copying [loplugin:passstuffbyref]}}

    // no warning expected
    OUString set1() { return OUString("xxx"); }
    OUString set2() { OUString v1("xxx"); return v1; }
    OUString set3() { S1 v1; return v1.get(); }
    OUString set4() { OUString v1[1]; return v1[0]; }
    OUString set5(OUString const & s) { return s; }
    OUString set6() { std::vector<OUString> v1(1); return v1[0]; }
    OUString set7(S1 const & s) { return s.get(); }
    OUString set8() { OUString * p = nullptr; return *p; }
};


// no warning expected

timeval &operator -= ( timeval &t1, const timeval &t2 );
timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval t0 = t1;
    return t0 -= t2;
}

void f()
{
    S2* s;
    OUString v1, v2;
    s = new S2(v1, v2);
}

struct S3 { S3(int); };

S3 f2() {
    static int n;
    return n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
