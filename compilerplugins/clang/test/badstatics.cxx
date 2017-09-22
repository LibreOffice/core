/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vector>
#include <map>
#include <cstdlib>

class Image { public: ~Image() { ::std::abort(); } };
class Bitmap { public: ~Bitmap() { ::std::abort(); } };

struct WithImage
{
    Image image;
};

struct WithBitmap
{
    Bitmap bitmap;
};

WithImage g_bug1; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}
WithBitmap g_bug2; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

static WithImage g_bug3; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

void foo() {
    static Image s_bug1; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}
    Image nobug;
}

::std::unique_ptr<WithImage> g_bug4; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::shared_ptr<WithImage> g_bug5; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::weak_ptr<WithImage> g_nobug;

struct DerivedWithImage : WithImage
{
};

DerivedWithImage g_bug6; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::vector<Image> g_bug7; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::vector<::std::unique_ptr<Image>> g_bug8; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::map<Image, int> g_bug9; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::map<int, Image> g_bug10; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class UserDefinedDtor
{
public:
    ~UserDefinedDtor();
};

UserDefinedDtor g_bug11; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

static UserDefinedDtor g_bug12; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::unique_ptr<UserDefinedDtor> g_bug13; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::shared_ptr<UserDefinedDtor> g_bug14; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::weak_ptr<UserDefinedDtor> g_nobug2;

::std::vector<::std::unique_ptr<UserDefinedDtor>> g_bug15; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::map<UserDefinedDtor, int> g_bug16; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

::std::map<int, UserDefinedDtor> g_bug17; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class WithUserDefinedDtor
{
    UserDefinedDtor udd;
};

WithUserDefinedDtor g_bug18; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class FromUserDefinedDtor : UserDefinedDtor
{
};

FromUserDefinedDtor g_bug19; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class VFromUserDefinedDtor : virtual UserDefinedDtor
{
};

VFromUserDefinedDtor g_bug20; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class UserDefinedEmptyDtor
{
public:
    ~UserDefinedEmptyDtor() {}
};

UserDefinedEmptyDtor g_nobug3;

::std::unique_ptr<UserDefinedEmptyDtor> g_nobug4;

::std::shared_ptr<UserDefinedEmptyDtor> g_nobug5;

class FromUserDefinedDtor2 : UserDefinedDtor
{
public:
    ~FromUserDefinedDtor2() {}
};

FromUserDefinedDtor2 g_bug21; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

class PureVirtualDtor
{
public:
    virtual ~PureVirtualDtor() = 0;
};

class FromPureVirtualDtor : PureVirtualDtor
{
};

FromPureVirtualDtor g_nobug6;

class PureVirtualDtor2
{
    UserDefinedDtor udd;
public:
    virtual ~PureVirtualDtor2() = 0;
};

class FromPureVirtualDtor2 : PureVirtualDtor2
{
};

FromPureVirtualDtor2 g_bug22; // expected-error {{bad static variable causes crash on shutdown [loplugin:badstatics]}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
