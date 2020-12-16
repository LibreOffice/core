/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basictest.hxx"
#include <cppunit/plugin/TestPlugIn.h>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <osl/file.hxx>

void MacroSnippet::InitSnippet()
{
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

void MacroSnippet::LoadFileToBuffer(const OUString& fileURL, OUStringBuffer& buf, sal_Int32 offset)
{
    osl::File aFile(fileURL);
    if (aFile.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None)
    {
        sal_uInt64 size;
        if (aFile.getSize(size) == osl::FileBase::E_None)
        {
            void* buffer = calloc(1, size + 1);
            CPPUNIT_ASSERT(buffer);
            sal_uInt64 size_read;
            if (aFile.read(buffer, size, size_read) == osl::FileBase::E_None)
            {
                if (size == size_read)
                {
                    OUString sCode(static_cast<char*>(buffer), size, RTL_TEXTENCODING_UTF8);
                    buf.insert(offset, sCode);
                }
            }

            free(buffer);
        }
    }
}

void MacroSnippet::LoadSourceFromFile(const OUString& sMacroFileURL)
{
    OUStringBuffer sSource;
    fprintf(stderr, "loadSource opening macro file %s\n",
            OUStringToOString(sMacroFileURL, RTL_TEXTENCODING_UTF8).getStr());
    LoadFileToBuffer(sMacroFileURL, sSource, 0);

    // Support simple include statements
    OUString includePrefix = "\n'%%include%% ";
    int directoryUrlLen = sMacroFileURL.lastIndexOf((sal_Unicode)'/') + 1;
    OUString directoryUrl = sMacroFileURL.copy(0, directoryUrlLen);
    int numIncludes = 0;
    for (sal_Int32 preProcIndex = sSource.indexOf(includePrefix, 0); preProcIndex >= 0;
         preProcIndex = sSource.indexOf(includePrefix, preProcIndex + 1))
    {
        sal_Int32 filenameIndex = preProcIndex + 14;
        sal_Int32 eol = sSource.indexOf((sal_Unicode)'\n', filenameIndex);
        CPPUNIT_ASSERT_MESSAGE("Invalid include statement in VB test", eol >= 0);
        if (eol < 0)
            break;

        const sal_Unicode* includeFilename = sSource.getStr() + filenameIndex;
        OUString includeFilenameStr = OUString(includeFilename, eol - filenameIndex);
        OUString includeFilenameURL = directoryUrl + includeFilenameStr;

        LoadFileToBuffer(includeFilenameURL, sSource, eol + 1);
        CPPUNIT_ASSERT_MESSAGE("Too many include statements in VB test", ++numIncludes < 100);
    }

    CPPUNIT_ASSERT_MESSAGE("Source is empty", sSource.getLength() > 0);
    MakeModule(sSource.toString());
}

SbxVariableRef MacroSnippet::Run( const css::uno::Sequence< css::uno::Any >& rArgs )
{
    SbxVariableRef pReturn;
    if ( !Compile() )
        return pReturn;
    SbMethod* pMeth = mpMod.is() ? static_cast<SbMethod*>(mpMod->Find( "doUnitTest",  SbxClassType::Method )) : nullptr;
    if ( pMeth )
    {
        if ( rArgs.hasElements() )
        {
            SbxArrayRef aArgs = new SbxArray;
            for ( int i=0; i < rArgs.getLength(); ++i )
            {
                SbxVariable* pVar = new SbxVariable();
                unoToSbxValue( pVar, rArgs[ i ] );
                aArgs->Put32(  pVar, i + 1 );
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
    CPPUNIT_ASSERT_MESSAGE("module is NULL", mpMod );
    mpMod->Compile();
    return !mbError;
}

bool MacroSnippet::HasError() const { return mbError; }

const ErrCode& MacroSnippet::getError() const { return maErrCode; }

IMPL_LINK( MacroSnippet, BasicErrorHdl, StarBASIC *, /*pBasic*/, bool)
{
    fprintf(stderr,"(%d:%d)\n",
            StarBASIC::GetLine(), StarBASIC::GetCol1());
    fprintf(stderr,"Basic error: %s\n", OUStringToOString( StarBASIC::GetErrorText(), RTL_TEXTENCODING_UTF8 ).getStr() );
    mbError = true;
    maErrCode = StarBASIC::GetErrorCode();
    return false;
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
