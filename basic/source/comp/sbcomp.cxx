/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "image.hxx"


// For debugging only
// #define DBG_SAVE_DISASSEMBLY

#ifdef DBG_SAVE_DISASSEMBLY
static bool dbg_bDisassemble = true;
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

using namespace comphelper;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;

void dbg_SaveDisassembly( SbModule* pModule )
{
    bool bDisassemble = dbg_bDisassemble;
    if( bDisassemble )
    {
        Reference< XSimpleFileAccess3 > xSFI;
        Reference< XTextOutputStream > xTextOut;
        Reference< XOutputStream > xOut;
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            Reference< XSimpleFileAccess3 > xSFI = Reference< XSimpleFileAccess3 >( xSMgr->createInstance
                ( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
            if( xSFI.is() )
            {
                String aFile( RTL_CONSTASCII_USTRINGPARAM("file:///d:/BasicAsm_") );
                StarBASIC* pBasic = (StarBASIC*)pModule->GetParent();
                if( pBasic )
                {
                    aFile += pBasic->GetName();
                    aFile.AppendAscii( "_" );
                }
                aFile += pModule->GetName();
                aFile.AppendAscii( ".txt" );

                // String aFile( RTL_CONSTASCII_USTRINGPARAM("file:///d:/BasicAsm.txt") );
                if( xSFI->exists( aFile ) )
                    xSFI->kill( aFile );
                xOut = xSFI->openFileWrite( aFile );
                Reference< XInterface > x = xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.io.TextOutputStream" ) );
                Reference< XActiveDataSource > xADS( x, UNO_QUERY );
                xADS->setOutputStream( xOut );
                xTextOut = Reference< XTextOutputStream >( x, UNO_QUERY );
            }
        }

        if( xTextOut.is() )
        {
            String aDisassemblyStr;
            pModule->Disassemble( aDisassemblyStr );
            xTextOut->writeString( aDisassemblyStr );
        }
        xOut->closeOutput();
    }
}
#endif

// Diese Routine ist hier definiert, damit der Compiler als eigenes Segment
// geladen werden kann.

BOOL SbModule::Compile()
{
    if( pImage )
        return TRUE;
    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( !pBasic )
        return FALSE;
    SbxBase::ResetError();
    // Aktuelles Modul!
    SbModule* pOld = pCMOD;
    pCMOD = this;

    SbiParser* pParser = new SbiParser( (StarBASIC*) GetParent(), this );
    while( pParser->Parse() ) {}
    if( !pParser->GetErrors() )
        pParser->aGen.Save();
    delete pParser;
    // fuer den Disassembler
    if( pImage )
        pImage->aOUSource = aOUSource;

    pCMOD = pOld;

    // Beim Compilieren eines Moduls werden die Modul-globalen
    // Variablen aller Module ungueltig
    BOOL bRet = IsCompiled();
    if( bRet )
    {
        pBasic->ClearAllModuleVars();
        // clear all method statics
        for( USHORT i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
            if( p )
                p->ClearStatics();
        }

        // #i31510 Init other libs only if Basic isn't running
        if( pINST == NULL )
        {
            SbxObject* pParent_ = pBasic->GetParent();
            if( pParent_ )
                pBasic = PTR_CAST(StarBASIC,pParent_);
            if( pBasic )
                pBasic->ClearAllModuleVars();
        }
    }

#ifdef DBG_SAVE_DISASSEMBLY
    dbg_SaveDisassembly( this );
#endif

    return bRet;
}

/**************************************************************************
*
*   Syntax-Highlighting
*
**************************************************************************/

void StarBASIC::Highlight( const String& rSrc, SbTextPortions& rList )
{
    SbiTokenizer aTok( rSrc );
    aTok.Hilite( rList );
}

