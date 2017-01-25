/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basictest.hxx"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <basic/sbstar.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <osl/file.hxx>

void MacroSnippet::InitSnippet()
{
    CPPUNIT_ASSERT_MESSAGE( "No resource manager", maDll.GetBasResMgr() != nullptr );
    mpBasic = new StarBASIC();
    StarBASIC::SetGlobalErrorHdl( LINK( this, MacroSnippet, BasicErrorHdl ) );
}

void MacroSnippet::MakeModule( const OUString& sSource )
{
    mpMod = mpBasic->MakeModule( "TestModule", sSource );
}

MacroSnippet::MacroSnippet( const OUString& sSource )
    : mbError(false)
{
    InitSnippet();
    MakeModule( sSource );
}

MacroSnippet::MacroSnippet()
    : mbError(false)
{
    InitSnippet();
}

void MacroSnippet::LoadSourceFromFile( const OUString& sMacroFileURL )
{
    OUString sSource;
    fprintf(stderr,"loadSource opening macro file %s\n", OUStringToOString( sMacroFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    osl::File aFile(sMacroFileURL);
    if(osl::FileBase::E_None == aFile.open(osl_File_OpenFlag_Read))
    {
        sal_uInt64 size;
        sal_uInt64 size_read;
        if(osl::FileBase::E_None == aFile.getSize(size))
        {
            void* buffer = calloc(1, size+1);
            CPPUNIT_ASSERT(buffer);
            if(osl::FileBase::E_None == aFile.read( buffer, size, size_read))
            {
                if(size == size_read)
                {
                    OUString sCode(static_cast<sal_Char*>(buffer), size, RTL_TEXTENCODING_UTF8);
                    sSource = sCode;
                }
            }

            free(buffer);
        }
    }
    CPPUNIT_ASSERT_MESSAGE( "Source is empty", ( sSource.getLength() > 0 ) );
    MakeModule( sSource );
}

SbxVariableRef MacroSnippet::Run( const css::uno::Sequence< css::uno::Any >& rArgs )
{
    SbxVariableRef pReturn = nullptr;
    if ( !Compile() )
        return pReturn;
    SbMethod* pMeth = mpMod.is() ? static_cast<SbMethod*>(mpMod->Find( "doUnitTest",  SbxClassType::Method )) : nullptr;
    if ( pMeth )
    {
        if ( rArgs.getLength() )
        {
            SbxArrayRef aArgs = new SbxArray;
            for ( int i=0; i < rArgs.getLength(); ++i )
            {
                SbxVariable* pVar = new SbxVariable();
                unoToSbxValue( pVar, rArgs[ i ] );
                aArgs->Put(  pVar, i + 1 );
            }
            pMeth->SetParameters( aArgs.get() );
        }
        pReturn = new SbxMethod( *static_cast<SbxMethod*>(pMeth));
    }
    return pReturn;
}

SbxVariableRef MacroSnippet::Run()
{
    css::uno::Sequence< css::uno::Any > aArgs;
    return Run( aArgs );
}

bool MacroSnippet::Compile()
{
    CPPUNIT_ASSERT_MESSAGE("module is NULL", mpMod.get() != nullptr );
    mpMod->Compile();
    return !mbError;
}

bool MacroSnippet::HasError() { return mbError; }

IMPL_LINK( MacroSnippet, BasicErrorHdl, StarBASIC *, /*pBasic*/, bool)
{
    fprintf(stderr,"(%d:%d)\n",
            StarBASIC::GetLine(), StarBASIC::GetCol1());
    fprintf(stderr,"Basic error: %s\n", OUStringToOString( StarBASIC::GetErrorText(), RTL_TEXTENCODING_UTF8 ).getStr() );
    mbError = true;
    return false;
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
