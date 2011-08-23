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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _ZFORLIST_HXX //autogen
#include <bf_svtools/zforlist.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <bf_so3/svstor.hxx>
#endif


#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _EXLPAR_HXX
#include <exlpar.hxx>
#endif
#ifndef _FLTGLBLS_HXX
#include <fltglbls.hxx>
#endif
#ifndef _EXCXFBUF_HXX
#include <excxfbuf.hxx>
#endif
#ifndef _EXCVFBFF_HXX
#include <excvfbff.hxx>
#endif
#ifndef _EXCFNTBF_HXX
#include <excfntbf.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

ExcGlob *pExcGlob = NULL;
//const double SwExcelParser::fExcToTwips = ( double ) TWIPS_PER_CHAR / 256;
const double SwExcelParser::fExcToTwips =
    ( double ) ((20.0 * 72.27) / 13.6) / 256;




//Diese Methode hier weil sie natuerlich nicht inline sein darf.
void SwFilterBase::Read( String &rS )
    {// liest 0-terminierten C-String!
    DBG_ASSERT( nReadBuffSize > 242,
        "-SwFilterBase::Read( String& ): Read-Buffer zu klein!!!!!!!!!" );

    sal_Char *pC = pReadBuff;
    register USHORT nCount = 0;	// Einer wird mindestens gelesen

    // String wird komplett mit 0-Terminator eingelesen
    do	{
        *pIn >> *pC;
        nCount++;
        }
    while( *(pC++) != 0 );

    nBytesLeft -= nCount;
    rS = String(pReadBuff, eQuellChar);
    }



ExcGlob::ExcGlob( SwDoc& rDoc, const SwPaM& rPam )
    : FilterGlobals( rDoc, rPam )
{
    pExcGlob = this;			// die Buffer greifen schon auf den Pointer zu
    pXF_Buff = new XF_Buffer;
    pFontBuff = new FontBuffer();
    pColorBuff = new ColorBuffer();

    Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
    pNumFormatter = new SvNumberFormatter( xMSF, LANGUAGE_SYSTEM );
}

ExcGlob::~ExcGlob()
{
    delete pXF_Buff;
    delete pFontBuff;
    delete pColorBuff;
    delete pNumFormatter;
}


SwExcelParser::SwExcelParser( SwDoc &rDoc, const SwPaM & rCrsr,
                                SvStream& rInInit, int bReadNewDoc,
                                CharSet eQ )
    : bNewDoc( bReadNewDoc )
{
    pIn = &rInInit;
    eQuellChar = eQ;
    pExcGlob = new ExcGlob( rDoc, rCrsr );

    pValueFormBuffer = new ValueFormBuffer;

    nReadBuffSize = 2048;
    pReadBuff = new sal_Char[ nReadBuffSize ];

    eDateiTyp = BiffX;
    nLastCol = nLastRow = 0;
    aColRowBuff.SetDefWidth( ( USHORT ) ( fExcToTwips * 1024 ) );	// 4 Zeichen Standardbreite

    bResultString = FALSE;
}

SwExcelParser::~SwExcelParser()
{
    delete pExcGlob;
    delete pValueFormBuffer;
    delete[] pReadBuff;
    pExcGlob = NULL;
}


ULONG SwExcelParser::CallParser()
{
    static const sal_Char* aNames[4] = {
        "Excel_Lotus/MinRow", "Excel_Lotus/MaxRow",
        "Excel_Lotus/MinCol", "Excel_Lotus/MaxCol"
    };
    sal_uInt32 aVal[4];
    SwFilterOptions aOpt( 4, aNames, aVal );

    USHORT nMinRow = ( USHORT ) aVal[ 0 ];
    USHORT nMaxRow = ( USHORT ) aVal[ 1 ];
    USHORT nMinCol = ( USHORT ) aVal[ 2 ];
    USHORT nMaxCol = ( USHORT ) aVal[ 3 ];

    USHORT nAnzNodes = 65000U - pExcGlob->pD->GetNodes().Count();

    if( nMaxRow < nMinRow )
    {
        USHORT nTemp = nMinRow;
        nMinRow = nMaxRow;
        nMaxRow = nTemp;
    }

    if( nMaxCol < nMinCol )
    {
        USHORT nTemp = nMinCol;
        nMinCol = nMaxCol;
        nMaxCol = nTemp;
    }

    if( nMaxRow - nMinRow == 0 )
        nMaxRow = nMinRow + 30;		// Default bei 0 Rows

    if( nMaxCol - nMinCol == 0 )
        nMaxCol = nMinCol + 15;		// Default bei 0 Cols

    if( nAnzNodes < ( nMaxRow - nMinRow ) * ( nMaxCol - nMinCol ) * 3 )
        return ERR_EXCLOT_WRONG_RANGE;

    pExcGlob->SetRange( nMinCol, nMaxCol, nMinRow, nMaxRow ); // Default-Einstellung
    Parse();
    return 0;
}



ULONG ExcelReader::Read( SwDoc &rDoc, SwPaM &rPam,
                    const String & /* FileName, falls benoetigt wird */ )
{
    if( rPam.GetNode()->FindTableNode() )
        return ERR_SWG_READ_ERROR;

    USHORT nOldBuffSize = 32768;

    ULONG nRet = 0;
    SvStorageStreamRef refStrm;			// damit uns keiner den Stream klaut
    SvStream* pIn = pStrm;
    if( pStg )
    {
        nRet = OpenMainStream( refStrm, nOldBuffSize );
        pIn = &refStrm;
    }
    else if( !pStrm )
    {
        ASSERT( FALSE, "ExcelReader-Read ohne Stream/Storage" );
        nRet = ERR_SWG_READ_ERROR;
    }

    if( !nRet )
    {
        SwExcelParser* pParser = new SwExcelParser( rDoc, rPam, *pIn, !bInsertMode, gsl_getSystemTextEncoding() );
        nRet = pParser->CallParser();	// 0 == kein Fehler aufgetreten

        delete pParser;

        if( refStrm.Is() )
            refStrm->SetBufferSize( nOldBuffSize );
    }

    return nRet;
}

int ExcelReader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}



}
