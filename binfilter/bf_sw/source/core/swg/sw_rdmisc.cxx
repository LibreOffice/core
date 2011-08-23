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


#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _JOBSET_HXX //autogen
#include <vcl/jobset.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <bf_sfx2/docinf.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <bf_svtools/macitem.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _RDSWG_HXX
#include <rdswg.hxx>
#endif
#ifndef _SWGPAR_HXX
#include <swgpar.hxx>       // SWGRD_xxx-Flags
#endif
#ifndef _FRMIDS_HXX
#include <frmids.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // Zeichen-Konversion
#endif
namespace binfilter {

//using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

// FlyFrames

// Ein FlyFrame ist ein normales Frame-Format. Die Hints RES_CNTNT und
// RES_FLY_ANCHOR muessen allerdings noch verbunden werden. Wenn der
// ANCHOR-Hint eingelesen wird, wird die globale Variable nCntntCol
// besetzt.

 void SwSwgReader::InFlyFrame( const SwNodeIndex* pNdIdx )
 {
    BOOL bAtCnt = FALSE;

    nCntntCol = 0;
    if( r.cur() != SWG_FLYFMT )
    {
        Error();
        return;
    }

    USHORT eSave_StartNodeType = eStartNodeType;
    eStartNodeType = SwFlyStartNode;

    SwFrmFmt* pFmt = (SwFrmFmt*) InFormat( NULL );
    RegisterFmt( *pFmt );
    if( pNdIdx )
    {
        // Content und Anchor miteinander verbinden, wenn Node angegeben ist
        SwFmtAnchor aAnchor = pFmt->GetAnchor();
        USHORT nId = aAnchor.GetAnchorId();
        switch( nId )
        {
            case FLY_AT_CNTNT:
                bAtCnt = TRUE;
            case FLY_IN_CNTNT:
                {
                    SwCntntNode *pNode = pDoc->GetNodes() [ *pNdIdx ]->GetCntntNode();
                    if( pNode )
                    {
                        SwPosition aPos( *pNdIdx, SwIndex( pNode, nCntntCol ) );
                        aAnchor.SetAnchor( &aPos );
                        pFmt->SetAttr( aAnchor );
                    }
                    // Layout-Frames im Insert Mode fuer absatzgebundene
                    // Flys erzeugen
                    if( !bNew && bAtCnt )
                        pFmt->MakeFrms();
                }
                break;
        }
    }
    //JP 01.04.97: wird nach dem Lesen durchs SwReader::Read eindeutig gemacht!
    if( pFmt->GetName().EqualsAscii("Fly") )
        pFmt->SetName( aEmptyStr );

    eStartNodeType = eSave_StartNodeType;
 }

 void SwSwgReader::InFlyFrames( const SwNodeIndex* pNdIdx )
 {
    USHORT nFrm;
    r >> nFrm;
    r.next();
    for( USHORT i = 0; i < nFrm && r.good(); i++)
        InFlyFrame( pNdIdx );

 }

/////////////////////////////////////////////////////////////////////////////

// Makros


 void SwSwgReader::InGlobalMacroTbl()
 {
    short nMacro;
    r >> nMacro;
    for( short i = 0; i < nMacro; i++ )
    {
        USHORT nEvent;
        r >> nEvent;
        String aLib = GetText();
        String aMac = GetText();
        pDoc->SetGlobalMacro( nEvent, SvxMacro( aMac, aLib, STARBASIC ) );
    }
    r.next();

 }

/////////////////////////////////////////////////////////////////////////////

// Job-Setup

 void SwSwgReader::InJobSetup()
 {
    BYTE recid = r.cur();
    // Der alte Job-Setup ist nicht mehr zu erstellen.
    if( recid == SWG_JOBSETUP )
        r.skipnext();
    else
    {
        JobSetup aJobSetup;
        BOOL bDfltPrn = FALSE;
        r.Strm() >> bDfltPrn >> aJobSetup;

        pDoc->SetJobsetup( aJobSetup );
 //JP 25.04.95: das Flag gibts nicht mehr:
 //     pDoc->UseDfltPrt( (BOOL)bDfltPrn );
        r.skipnext();
    }
 }

/////////////////////////////////////////////////////////////////////////////

// Dokument-Info

 void SwSwgReader::InDocInfo()
 {
    while( r.good() )
    {
        switch( r.next() )
        {
            case SWGINF_DBNAME:
                {
                    String aName = ParseText();
                    if( bNew )
                    {
                        if( aName.EqualsIgnoreCaseAscii( "ADRESSEN" ) )
                            aName.AssignAscii( "Address" );
                        SwDBData aData;
                        aData.sDataSource = aName;
                        pDoc->ChgDBData( aData );
                    }
                }
                break;
            case SWGINF_DOCSTAT:
                if( bNew )
                {
                    USHORT nPage, nPara;
                    SwDocStat aStat;
                    r >> aStat.nTbl >> aStat.nGrf >> aStat.nOLE
                      >> nPage >> nPara >> aStat.nWord
                      >> aStat.nChar;
                    aStat.nPage = nPage;
                    aStat.nPara = nPara;
                    aStat.bModified = FALSE;
                    aStat.pInternStat = NULL;
                    pDoc->SetDocStat( aStat );
                    break;
                } else r.skip(); // wird bei Einfuegen ignoriert
                break;
            case SWGINF_END:
                return;
            case SWGINF_LAYOUTPR: {
 //JP 25.04.95: SetLayoutPrtName gibts nicht mehr
                ParseText();
                } break;
            default:
                // wird ignoriert
                r.skip();
        }
    }

 }

/////////////////////////////////////////////////////////////////////////////


 Color SwSwgReader::InColor()
 {
    USHORT red, green, blue;
    r >> red >> green >> blue;
    Color aClr( BYTE(red >> 8), BYTE(green >> 8), BYTE( blue >> 8) );
    return aClr;

 }


 void SwSwgReader::InPen(USHORT& nWidth, Color& rCol)
 {
    BYTE cStyle;
    r >> cStyle >> nWidth;
    rCol = InColor();
 }


 Color SwSwgReader::InBrush()
 {
    BYTE cStyle, cTransparent;
    r >> cStyle >> cTransparent;

 // Brush aBr( (BrushStyle) cStyle );
 // aBr.SetTransparent( BOOL( cTransparent ) );
 // aBr.SetColor( InColor() );
    Color aCol(InColor());
    Color aTmpFillColor(InColor());

    return aCol;

 }

/////////////////////////////////////////////////////////////////////////////


 void SwSwgReader::InFtnInfo()
 {
    SwFtnInfo aFtn;
    aFtn = pDoc->GetFtnInfo();
    aFtn.aQuoVadis = GetText();
    aFtn.aErgoSum = GetText();
    //Ab der 3.0 sind nur noch max. 30 Zeichen erlaubt.
    aFtn.aQuoVadis.Erase( 30 );
    aFtn.aErgoSum. Erase( 30 );

    BYTE ePos, eNum, eType;
    USHORT nDesc, nCollIdx;
    if( ( aHdr.nVersion >= SWG_VER_COMPAT ) && ( r.next() != SWG_DATA ) )
    {
        Error(); return;
    }
    r >> ePos >> eNum >> eType >> nDesc;
    if( aHdr.nVersion >= SWG_VER_COMPAT )
    {
        r >> nCollIdx;
        r.skip();
        if( nCollIdx != IDX_NO_VALUE )
        {
            nCollIdx |= IDX_COLLECTION;
            SwTxtFmtColl* pColl = (SwTxtFmtColl*) FindFmt( nCollIdx, 0 );
            if( pColl )
                aFtn.SetFtnTxtColl( *pColl );
        }
    }
    if( nLay )
        nLay = pLayIdx[ nDesc ].nActualIdx;
    aFtn.ChgPageDesc( (SwPageDesc*)&pDoc->GetPageDesc( nLay ) );

    aFtn.ePos = (SwFtnPos) ePos;
    aFtn.eNum = (SwFtnNum) eNum;
    aFtn.aFmt.SetNumberingType(eType);
    pDoc->SetFtnInfo( aFtn );
    r.next();
 }

// Aufdroeseln des Comment-Records fuer kuenftige Erweiterungen

 void SwSwgReader::InComment()
 {
    BYTE cType;
    USHORT nVal;
    r >> cType;
    switch( cType ) {
        case SWG_XFTNCOLL: {
            // Fussnoten-Erweiterung: Nummer der TxtColl
            r >> nVal;
            nVal &= IDX_COLLECTION;
            SwTxtFmtColl* pColl = (SwTxtFmtColl*) FindFmt( nVal, 0 );
            if( pColl )
            {
                SwFtnInfo aFtn;
                aFtn = pDoc->GetFtnInfo();
                aFtn.SetFtnTxtColl( *pColl );
                pDoc->SetFtnInfo( aFtn );
            }
        } break;
        default:
            r.skip();
    }
    r.next();
 }

///////////////////////////////////////////////////////////////////////////

// Einlesen der statischen DocInfo

 static void InSfxStamp( swistream& r, SfxStamp& rStamp,
                        rtl_TextEncoding eCharSet )
 {
    r.long4();
    long nDate, nTime;
    sal_Char buf[ 32 ];
    r >> nDate >> nTime;
    r.get( buf, 32 );
    r.long3();
    Date d( nDate );
    Time t( nTime );
    String aName( buf, eCharSet );
    rStamp.SetTime( DateTime( d, t ) );
    rStamp.SetName( aName );
 }

 static void InSfxDocString
 ( swistream& r, String& rText, short nLen, rtl_TextEncoding eCharSet )
 {
    sal_Char buf[ 256 ];
    r.get( buf, nLen );
    String sTmp( buf, eCharSet );
    rText = sTmp;
 }

 void SwSwgReader::InStaticDocInfo( SfxDocumentInfo& rInfo )
 {
    long pos;
    USHORT n, i;
    BYTE cGUIType, cCharSet;
    // TODO: unicode: is this correct?
    rtl_TextEncoding eCharSet = gsl_getSystemTextEncoding();
    SfxStamp aStamp;
    String aText;

    while( r.good() )
    {
        switch( r.next() )
        {
            case SWGINF_END:
                return;
            case SWGINF_SAVEINFO:
                pos = r.tell();
                r.skip();
                if( r.peek() == SWGINF_EXTINFO )
                {
                    r.next();
                    r >> cGUIType
                      >> cCharSet;
                    eCharSet = (rtl_TextEncoding) cCharSet;
                }
                r.seek( pos );
                InSfxStamp( r, aStamp, eCharSet );
                rInfo.SetCreated( aStamp );
                InSfxStamp( r, aStamp, eCharSet );
                rInfo.SetChanged( aStamp );
                InSfxStamp( r, aStamp, eCharSet );
                rInfo.SetPrinted( aStamp );
                // SwSwgInfo-Felder II: Titel, Autor etc
                InSfxDocString( r, aText, 64, eCharSet );
                rInfo.SetTitle( aText );
                InSfxDocString( r, aText, 64, eCharSet );
                rInfo.SetTheme( aText );
                InSfxDocString( r, aText, 256, eCharSet );
                rInfo.SetComment( aText );
                InSfxDocString( r, aText, 128, eCharSet );
                rInfo.SetKeywords( aText );
                // SwSwgInfo-Felder III: User-Keys
                r >> n;
                for( i = 0; i < n; i++ )
                {
                    String aKeys;
                    InSfxDocString( r, aText, 20, eCharSet );
                    InSfxDocString( r, aKeys, 20, eCharSet );
                    SfxDocUserKey aKey( aText, aKeys );
                    rInfo.SetUserKey( aKey, i );
                }
                break;
            default:
                // wird ignoriert
                r.skip();
        }
    }

 }

 void SwSwgReader::InDynamicDocInfo( SfxDocumentInfo& rInfo )
 {
    // Record suchen:
    if( r.next() == SWG_DOCUMENT )
    {
        r.skip();
        if( r.peek() == SWG_DYNDOCINFO )
        {
            r.next();
            long nextrec = r.getskip();
            long pos, d, t;
            BYTE cFlags = 0;
            while( r.tell() < nextrec && r.good() )
            {
                String aText;
                switch( r.next() )
                {
                    case SWGINF_TEMPLATE:
                        pos = r.getskip();
                        aText = GetText();
                        r.long4();
                        r >> d >> t;
                        r.long3();
                        if( r.tell() < pos ) r >> cFlags;
                        rInfo.SetTemplateDate( DateTime( Date( d ), Time( t ) ) );
                        rInfo.SetTemplateName( aText );
                        rInfo.SetTemplateFileName( aFileName );
                        rInfo.SetQueryLoadTemplate( BOOL( ( cFlags & 0x01 ) != 0 ) );
                        r.skip( pos ); break;
                    default:
                        r.skip();
                }
            }
        }
    }
 }

 BOOL SwSwgReader::LoadDocInfo( SfxDocumentInfo& rInfo )
 {
    FileHeader aFile;
    long pos0 = r.tell();

    memset( &aFile, 0, sizeof aFile );
    r.get( &aFile, 4 );
    // Die Signatur sollte schon stimmen!!!
    // Aber bitte nur die ersten drei Zeichen, um nicht abwaertskompatible
    // Versionen erkennen zu koennen.
    if( memcmp( ( const void*) &aFile.nSignature, SWG_SIGNATURE, 3 ) )
        return FALSE;
    r.long4();
    r >> aFile.nVersion
      >> aFile.nFlags
      >> aFile.nFree1
      >> aFile.nDocInfo;
    r.get( aFile.cPasswd, 16 );
    r.long3();
 // rInfo.SetPasswd( BOOL( ( aFile.nFlags & SWGF_HAS_PASSWD ) != 0 ) );
    rInfo.SetPortableGraphics( BOOL( ( aFile.nFlags & SWGF_PORT_GRAF ) != 0 ) );

    // Passwort in Stream eintragen
    if( aFile.nFlags & SWGF_HAS_PASSWD )
        r.copypasswd( aFile.cPasswd );
    // Die statische DocInfo lesen
    // Hot fix fuer Bug #4955 (Textbausteine mit geloeschten Bereichen)
    if( !aFile.nDocInfo ) aFile.nDocInfo = 0x5B;
    if( aFile.nVersion >= SWG_VER_FMTNAME )
    {
        r.seek( aFile.nDocInfo - 4 );
        InStaticDocInfo( rInfo );
    }
    if( aFile.nVersion >= SWG_VER_DOCINFO )
    {
        r.seek( pos0 + 32 /* sizeof( FileHeader ) in Datei */ );
        InDynamicDocInfo( rInfo );
        r.seek( pos0 );
    }
    return BOOL( r.good() );
 }
}
