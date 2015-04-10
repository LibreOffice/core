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
#include <osl/file.hxx>

class MacroSnippet
{
    private:
    bool mbError;
    BasicDLL maDll; // we need a dll instance for resource manager etc.
    SbModuleRef mpMod;
    StarBASICRef mpBasic;

    void InitSnippet()
    {
        CPPUNIT_ASSERT_MESSAGE( "No resource manager", maDll.GetBasResMgr() != NULL );
        mpBasic = new StarBASIC();
        StarBASIC::SetGlobalErrorHdl( LINK( this, MacroSnippet, BasicErrorHdl ) );
    }
    void MakeModule( const OUString& sSource )
    {
        mpMod = mpBasic->MakeModule( OUString( "TestModule" ), sSource );
    }
    public:
    struct ErrorDetail
    {
        OUString sErrorText;
        int nLine;
        int nCol;
        ErrorDetail() : nLine(0), nCol(0) {}
    };

    MacroSnippet( const OUString& sSource ) : mbError(false)
    {
        InitSnippet();
        MakeModule( sSource );
    };
    MacroSnippet() : mbError(false)
    {
        InitSnippet();
    };
    void LoadSourceFromFile( const OUString& sMacroFileURL )
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

    SbxVariableRef Run( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs )
    {
        SbxVariableRef pReturn = NULL;
        if ( !Compile() )
            return pReturn;
        SbMethod* pMeth = mpMod ? static_cast<SbMethod*>(mpMod->Find( OUString("doUnitTest"),  SbxCLASS_METHOD )) : NULL;
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
                pMeth->SetParameters( aArgs );
            }
            pReturn = new SbxMethod( *((SbxMethod*)pMeth));
        }
        return pReturn;
    }

    SbxVariableRef Run()
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aArgs;
        return Run( aArgs );
    }

    bool Compile()
    {
        CPPUNIT_ASSERT_MESSAGE("module is NULL", mpMod != NULL );
        mpMod->Compile();
        return !mbError;
    }

    DECL_LINK( BasicErrorHdl, StarBASIC * );

    static ErrorDetail GetError()
    {
        ErrorDetail aErr;
        aErr.sErrorText = StarBASIC::GetErrorText();
        aErr.nLine = StarBASIC::GetLine();
        aErr.nCol = StarBASIC::GetCol1();
        return aErr;
    }

    bool HasError() { return mbError; }

    void ResetError()
    {
        StarBASIC::SetGlobalErrorHdl( Link() );
        mbError = false;
    }
};

IMPL_LINK( MacroSnippet, BasicErrorHdl, StarBASIC *, /*pBasic*/)
{
    fprintf(stderr,"(%d:%d)\n",
            StarBASIC::GetLine(), StarBASIC::GetCol1());
    fprintf(stderr,"Basic error: %s\n", OUStringToOString( StarBASIC::GetErrorText(), RTL_TEXTENCODING_UTF8 ).getStr() );
    mbError = true;
    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
