/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_BASIC_QA_CPPUNIT_BASICTEST_HXX
#define INCLUDED_BASIC_QA_CPPUNIT_BASICTEST_HXX

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>
#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>

class MacroSnippet
{
private:
    bool mbError;
    BasicDLL maDll; // we need a dll instance for resource manager etc.
    SbModuleRef mpMod;
    StarBASICRef mpBasic;

    void InitSnippet();
    void MakeModule( const OUString& sSource );

public:
    explicit MacroSnippet( const OUString& sSource );
    MacroSnippet();

    void LoadSourceFromFile( const OUString& sMacroFileURL );

    SbxVariableRef Run( const css::uno::Sequence< css::uno::Any >& rArgs );

    SbxVariableRef Run();

    bool Compile();

    DECL_LINK_TYPED( BasicErrorHdl, StarBASIC *, bool );

    bool HasError();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
