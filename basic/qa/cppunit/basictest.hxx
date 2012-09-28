/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _BASICTEST_HXX
#define _BASICTEST_HXX

#include <sal/types.h>
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <test/bootstrapfixture.hxx>
#include "basic/sbstar.hxx"
#include "basic/basrdll.hxx"

class BasicTestBase : public test::BootstrapFixture
{
    private:
    bool mbError;
    public:
    BasicTestBase() : BootstrapFixture(true, false), mbError(false) {};


    DECL_LINK( BasicErrorHdl, StarBASIC * );
    bool HasError() { return mbError; }
    void ResetError()
    {
        StarBASIC::SetGlobalErrorHdl( Link() );
        mbError = false;
    }
    BasicDLL& basicDLL()
    {
        static BasicDLL maDll; // we need a dll instance for resouce manager etc.
        return maDll;
    }
};

IMPL_LINK( BasicTestBase, BasicErrorHdl, StarBASIC *, /*pBasic*/)
{
    fprintf(stderr,"Got error: \n\t%s!!!\n", rtl::OUStringToOString( StarBASIC::GetErrorText(), RTL_TEXTENCODING_UTF8 ).getStr() );
    mbError = true;
    return 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
