/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <hintids.hxx>
#include <bf_sfx2/docinf.hxx>
#include <bf_svx/fontitem.hxx>

#include <fmtpdsc.hxx>
#include <rdswg.hxx>

#include <errhdl.hxx>

#include <ndtxt.hxx>        // Zeichen-Konversion
#include <pagedesc.hxx>
#include <swgpar.hxx>       // SWGRD_xxx-Flags
#include <frmids.hxx>
#include <mdiexp.hxx>       // Progress

#include <horiornt.hxx>

#include <doc.hxx>
#include <pam.hxx>
#include <poolfmt.hxx>

#include <swerror.h>
#include <statstr.hrc>
namespace binfilter {



//////////////////////////////////////////////////////////////////////////////

// CodeSet-Konvertierung ab Version 22:

//////////////////////////////////////////////////////////////////////////////

 SwSwgReader::SwSwgReader
    ( SwDoc *pSwDoc, const SwPaM* pSwPaM, SvStream& rStream,
      const String& rFileName, BOOL bNewDoc )
    : pDoc( pSwDoc ), r( rStream ), aFileName( rFileName ), bNew(bNewDoc),
    pNdOrigTxt( 0 )
 {
    nCntntCol  = 0;
    nErrno     = 0;
    pCollIdx   = NULL;
    pLayIdx    = NULL;
    pFmts      = NULL;
    pPageLinks = NULL;
    pRules     = NULL;
    pFrms      = NULL;
    pSects     = NULL;
    pLastSect  = NULL;
    pLanguages = NULL;
    pTables    = NULL;
    nColl      =
    nLay       =
    nNamedFmt  =
    nFrm       =
    nFrmSize   =
    nSect      =
    nSectSize  =
    nTbl       =
    nTblSize   =
    nRules     =
    nLang      =
    nLangSize  =
    nFmtLvl    =
    nStatus    = 0;
    nTblBoxLvl = -1;
    nDocStart  = r.tell();
    nOptions   =
    nFrmFlags  =
    nFlyLevel  =
    nPage1     =
    nFlagBits  = 0;
    nHelpFileId= UCHAR_MAX;
    nNextDoc   = 0;
    pPaM       = NULL;
    pUpper     = NULL;
    pMaster    = NULL;
    pInfo      = new SfxDocumentInfo;

    eStartNodeType = SwNormalStartNode;

    if( pSwPaM )
    {
        pPaM = new SwPaM( *pSwPaM->GetPoint() );
        SwTxtNode *pTxtNode = pPaM->GetNode()->GetTxtNode();

        // Ist das ein SwPaM auf einen TextNode?
        if( !pTxtNode )
        {
            // Ist keiner, dann machen wir uns einen
            pTxtNode=pDoc->GetNodes().MakeTxtNode(pPaM->GetPoint()->nNode,
                     (SwTxtFmtColl*) pDoc->GetDfltTxtFmtColl() );
            // Da zeigt jetzt auch der StartIdx hin
            pPaM->GetPoint()->nNode-=1;
        }
        if( !bNewDoc ) nPage1 = pPaM->GetPageNum() - 1;
    }
 }

 SwSwgReader::~SwSwgReader()
 {
    while( pPageLinks )
    {
        PageDescLink* p = pPageLinks->pLink;
        delete pPageLinks;
        pPageLinks = p;
    }
    delete pInfo;
    delete pFmts;
    delete pFrms;
    delete pCollIdx;
    delete pLayIdx;
    delete pRules;
    delete pSects;
    delete pLanguages;
    delete pPaM;
    delete pTables;
 }

BOOL SwSwgReader::CheckPasswd( const String& rPass )
{
    return r.setpasswd( rPass ) &&
           BOOL( memcmp( aFile.cPasswd, r.getpasswd(), PASSWDLEN ) == 0 );
}

 void SwSwgReader::Error( ULONG nCode )
 {
    ASSERT( !this, "Formatfehler in Datei entdeckt" );
    nErrno = nCode ? (nCode | ERROR_SW_READ_BASE ) : ERR_SWG_FILE_FORMAT_ERROR;
    r.setbad();
 }

 short SwSwgReader::LayoutIdx( short n )
 {
    return( pLayIdx && (USHORT) n < nLay ) ? pLayIdx[ n ].nActualIdx : 0;
 }

//////////////////////////////////////////////////////////////////////////////

// Laden des Dateikopfes
// Vor Version 0x0011 wird nur die Signatur gelesen.

 void SwSwgReader::LoadFileHeader()
 {
    memset( &aFile, 0, sizeof aFile );
    r.seek ( 0 );
    r.get( &aFile.nSignature, 4 );
    // Eine Textbausteindatei darf auch leer sein
    if( r.eof() )
    {
        // in diesem Fall den Header initialisieren
        memset( &aFile, 0, sizeof aFile );
        memcpy( &aFile.nSignature, SWG_SIGNATURE, 4 );
        aFile.nVersion = SWG_VERSION;
        return;
    }
    // Die Signatur sollte schon stimmen!!!
    // Aber bittte nur die ersten drei Zeichen, um nicht abwaertskompatible
    // Versionen erkennen zu koennen.
    else if( memcmp( ( const void*) &aFile.nSignature, SWG_SIGNATURE, 3 ) )
    {
        Error(); return;
    }

    r.long4();
    r >> aFile.nVersion
      >> aFile.nFlags
      >> aFile.nFree1
      >> aFile.nDocInfo;
    r.get( aFile.cPasswd, 16 );
    r.long3();
    // Passwort in Stream eintragen
    if( aFile.nFlags & SWGF_HAS_PASSWD )
        r.copypasswd( aFile.cPasswd );
 //MA Erstmal abklemmen!!!!!!!!!!!!!!!!!!!!!!
 //JP 12.09.96: Bug 31264 - zum PageDesc Abpruefen brauchen wir das Flag aber
 //MIB: there is no flag to not load the layout any longer, it is loaded always.
 // aFile.nFlags |= SWGF_NO_FRAMES;
    r.seek ( 0 );
    LoadDocInfo( *pInfo );
    r.seek ( 0x20 );
    if( pDoc )
    {
        if( aFile.nFlags & SWGF_HAS_PGNUMS ) pDoc->SetPageNums();
    }
 }

// Laden der Header-Informationen
// Ab Version 0x0011 ist der Header Bestandtail eines SWG_DOCUMENT-Records.
// Die Position zeigt auf den Beginn der Infos, also entweder hinter die
// Version (s.o.) oder auf den Beginn des SWG_DOCUMENT-Records.

 void SwSwgReader::LoadHeader()
 {
    memset( &aHdr, 0, sizeof aHdr );
    r.next();
    // Evtl. den Dynamischen DocInfo-Record auch skippen
    nNextDoc = r.getskip();
    // Achtung: Es gibt tatsaechlich Docs, die hier was unsinniges haben!
    long siz = r.filesize();
    if( nNextDoc < 0 || nNextDoc > siz )
        nNextDoc = siz;
    r.skip();
    if( r.next() == SWG_DYNDOCINFO )
        nNextDoc = r.getskip();
    r.seek( nDocStart );
    r.next();
    r >> aHdr.nFlags
      >> aHdr.nVersion;
    r.get( aHdr.cAttrTab, 8 );
    r.get( aHdr.cAttrSiz, 8 );
    r.long4();
    r >> aHdr.cCodeSet
      >> aHdr.nNumRules
      >> aHdr.nDocVersion
      >> aHdr.cGUIType
      >> aHdr.cReserved[ 0 ]
      >> aHdr.cReserved[ 1 ]
      >> aHdr.cReserved[ 2 ]
      >> aHdr.nDocInfoOffset
      >> aHdr.nLayoutOffset
      >> aHdr.nDate
      >> aHdr.nTime;
    r.long3();
    if( aHdr.nDocInfoOffset ) aHdr.nDocInfoOffset += nDocStart;
    if( aHdr.nLayoutOffset )  aHdr.nLayoutOffset  += nDocStart;
    nRecStart = r.tell();

    // Hot fix fuer Tesxtbausteine, wo der DocInfo-Ptr im Dateikopf
    // verlorengegangen ist (Bugid 4955): Ptr aus Dokument-Hdr
    // restaurieren
    if( !aFile.nDocInfo ) aFile.nDocInfo = aHdr.nDocInfoOffset + 8;
 }

// Einlesen eines Strings mit Check und Konvertierung

 String SwSwgReader::GetText( BOOL bReq )
 {
    String aRes;
    if( r.next() != SWG_TEXT )
    {
        if( bReq )
            Error();
        else
            r.undonext();
    } else
        aRes = ParseText();
    return aRes;
 }

ByteString SwSwgReader::GetAsciiText( BOOL bReq )
{
    ByteString aRes;
    if( r.next() != SWG_TEXT )
    {
        if( bReq )
            Error();
        else
            r.undonext();
    } else
        aRes = r.text();
    return aRes;
}

// Einlesen und Konvertierung eines Strings in den Native-Zeichensatz
// Der Record-Header SWG_TEXT ist bereits eingelesen

 String SwSwgReader::ParseText()
 {
    String aOrig( r.text(), (rtl_TextEncoding)aHdr.cCodeSet );
    return aOrig;
 }


/////////////////////////////////////////////////////////////////////////////

 void _ReadFrm_Rect( swistream& r )
 {
    BYTE sizes;
    r >> sizes;
    int nSkip = 0;
    for( USHORT i = 0; i < 4; i++, sizes <<= 2 )
    {
        switch( sizes & 0xC0 )
        {
        case 0x40:
            nSkip += 2;
            break;
        case 0x80:
            nSkip += 3;
            break;
        case 0xC0:
            nSkip += 4;
            break;
        }
    }
    r.seek( r.tell() + nSkip );
 }

 void _ReadFrm_LayFrm( swistream& r, USHORT nVersion )
 {
    BYTE nB, nFrmFlag;
    USHORT nU;

    // Byte vom Frm
    r >> nFrmFlag >> nB;
    if( nVersion >= FRMVER_EXTINFO && ( nB & 0x80 ) )
        r >> nB;
    if( nVersion >= FRMVER_EXTINFO )
        // Eventuelle Zusatz-Bits entfernen
        while( nB & 0x80 ) r >> nB;

    // Frame-Rect einlesen (mit Addieren des Upper-Starts, falls vorhanden)
    _ReadFrm_Rect( r );
    if( nFrmFlag & FRMF_PRINT )
        _ReadFrm_Rect( r );

    // Dann Frame-ID eintragen
    // Version 1 hatte FRMF_HASID nicht bei FRMF_HASFOLLOW gesetzt
    if( nFrmFlag & ( FRMF_HASFOLLOW | FRMF_HASID ) )
        r >> nU;
    if( nFrmFlag & FRMF_ISFOLLOW )
        r >> nU;

    // Byte vom LayoutFrm
    r >> nU;
 }

/////////////////////////////////////////////////////////////////////////////

// Der Returnwert ist der errno-Fehlercode

 ULONG SwSwgReader::Read( USHORT nOpt )
 {
    nOptions = nOpt;
    if( !pPaM
     || ( aFile.nVersion < SWG_VER_PORTGRF )
     || ( aFile.nFlags & SWGF_BAD_FILE ) )
        return ERR_SWG_READ_ERROR;

    nDocStart  = r.tell();

    nErrno = 0;
    USHORT nInsFirstPara = 0;
    {
        SwTxtNode *pTxtNode=pDoc->GetNodes()[pPaM->GetPoint()->nNode]->GetTxtNode();
        // Ist das ein SwPaM auf einen TextNode?
        if( pTxtNode && pTxtNode->GetTxt().Len() )
            nInsFirstPara = 1;
    }

    LoadHeader();

    // Den Vorlagen-Namen aus der DocInfo am Doc registrieren,
    // falls ueberhaupt einer da ist
    if( pInfo->GetTemplateName().Len() )
        nHelpFileId = pDoc->SetDocPattern( pInfo->GetTemplateName() );

    // Es wird angenommen, dass das Layout 5% der Ladezeit belegt.
    long nEnd = aHdr.nLayoutOffset ? aHdr.nLayoutOffset : nNextDoc;
    nEnd = ( nEnd * 105 ) / 100;

    // Default-Collection registrieren
    SwTxtFmtColl* pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    pColl->nFmtId = IDX_COLLECTION + 0;
    RegisterFmt( *pColl );

    // Erst einmal die Link-Optionen weg
    nOptions &= 0xF0FF;

    // Auf gehts:
    BOOL bDone = FALSE;
    BOOL bNode1 = TRUE;
    r.next();
    while( !bDone && r.good() && !IsError( nErrno ) && ( r.tell() < nNextDoc ) )
    {
        BYTE ch = r.cur();
        switch( (int) ch ) {
            case SWG_EOF:
                if( ( nOptions == SWGRD_NORMAL )
                 && ( r.next() == (BYTE) SWG_DOCINFO ) )
                    InDocInfo();
                bDone = TRUE; break;
            case SWG_DOCINFO:
                InDocInfo(); r.next(); break;
            case SWG_NAMEDFMTS:
                InNamedFmts( nOptions );
                break;
            case SWG_JOBSETUP:
            case SWG_NEWJOBSETUP:
                if( nOptions == SWGRD_NORMAL ) InJobSetup();
                else r.skipnext();
                break;
            case SWG_OUTLINE:
                if( nOptions == SWGRD_NORMAL )
                    InOutlineRule();
                else r.skipnext();
                break;
            case SWG_NUMRULES:
                if( nOptions == SWGRD_NORMAL )
                    InTxtNumRule();
                else r.skipnext();
                break;
            case SWG_MACROTBL:
                if( nOptions & SWGRD_MACROS )
                {
                    InGlobalMacroTbl();
                    if( nOptions == SWGRD_MACROS ) bDone = TRUE;
                } else r.skipnext();
                break;
            case SWG_LAYOUT:
                if( nOptions & SWGRD_PAGEFMTS )
                {
                    InPageDescs();
                    if( nOptions == SWGRD_PAGEFMTS ) bDone = TRUE;
                } else r.skipnext();
                break;
            case SWG_DFLTFMTS:
                if( nOptions & SWGRD_PARAFMTS ) InDfltFmts();
                else r.skipnext();
                break;
            case SWG_FIELDTYPES:
                if( nOptions == SWGRD_NORMAL ) InFieldTypes();
                else r.skipnext();
                break;
            case SWG_COLLECTIONS:
                if( nOptions & SWGRD_PARAFMTS ) InTxtFmtColls();
                else r.skipnext();
                break;
            case SWG_FLYFRAMES:
                if( nOptions == SWGRD_NORMAL ) InFlyFrames( NULL );
                else r.skipnext();
                break;
            case SWG_FOOTINFO:
                // globale Fussnoten-Info
                if( nOptions == SWGRD_NORMAL ) InFtnInfo();
                else r.skipnext();
                break;
            case SWG_CONTENTS:
                if( nOptions == SWGRD_NORMAL )
                    InContents( bNode1, FALSE, nInsFirstPara );
                else
                    r.skipnext();
                nInsFirstPara = 0; bNode1 = FALSE;
                break;
            case SWG_FRAMES:

                if( !( aFile.nFlags & SWGF_NO_FRAMES )
                    && bNew && ( nOptions == SWGRD_NORMAL ) )
                {
                    // Offene Seitenbeschreibungen in Formaten aufloesen
                    // (falls noch nicht geschehen)
                    ResolvePageDescLinks();

                    // Teste ob das Layout auf der 1. Seite einen anderen
                    // PageDesc definiert hat das Doc. Dann nehme den aus
                    // dem Layout! Bug 31264
                    SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 1 );
                    SwCntntNode* pCNd = pDoc->GetNodes().GoNext( &aIdx );
                    if( pCNd && SFX_ITEM_SET != pCNd->GetSwAttrSet().
                        GetItemState(RES_PAGEDESC) && !pCNd->FindTableNode())
                    {
                        // es ist keiner gesetzt, dann muss das Layout auch
                        // der Meinung sein, den Standard zu benutzen!
                        USHORT nVersion;
                        r >> nVersion;
                        if( nVersion >= FRMVER_NEWFLY )
                        {
                            USHORT nU;
                            BYTE nId;
                            r >> nId;
                            if( FRMID_ROOT == nId )
                            {
                                _ReadFrm_LayFrm( r, nVersion );
                                r >> nId
                                  >> nU;    // in nU sollte die Seiten stehen!
                                if( nU )
                                {
                                    r >> nId;
                                    if( FRMID_PAGE == nId )
                                    {
                                        _ReadFrm_LayFrm( r, nVersion );
                                        r >> nId >> nU;
                                        // in nU steht der gesuchte PageDesc!
                                        SwPageDesc* pDesc = &FindPageDesc( nU );
                                        if( pDesc && pDesc != &pDoc->_GetPageDesc( 0 ) )
                                        {
                                            // dann am Node setzen!!
                                            pCNd->SetAttr( SwFmtPageDesc( pDesc ) );
                                            ASSERT( !this, "LayoutPageDesc am Doc setzen" );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                r.skipnext();
                break;
            case SWG_COMMENT:
                InComment(); break;
            default:
                r.skipnext();
        }
    }

    // Offene Seitenbeschreibungen in Formaten aufloesen
    // (falls noch nicht geschehen)
    ResolvePageDescLinks();
    // Bug 9714: Der CharSet an den Fonts muss geaendert werden, wenn
    // es der globale CharSet ist
    USHORT nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_CHRATR_FONT );
    rtl_TextEncoding eSysCharSet = gsl_getSystemTextEncoding();
    SvxFontItem* pItem;
    for( USHORT n = 0; n < nMaxItems; ++n )
    {
        pItem = (SvxFontItem*) pDoc->GetAttrPool().GetItem( RES_CHRATR_FONT, n );
        if( pItem && pItem->GetCharSet() == (rtl_TextEncoding)aHdr.cCodeSet )
            pItem->GetCharSet() = eSysCharSet;
    }
    ClearFmtIds();
    if( !IsError( nErrno ) )
    {
        pDoc->SetGlossDoc( BOOL( ( aFile.nFlags & SWGF_HAS_BLOCKS ) != 0 ) );
        if( aFile.nVersion > SWG_VERSION )
            nErrno = WARN_SWG_FEATURES_LOST | WARN_SW_READ_BASE;
    }

//STRIP001     EndProgress( pDoc->GetDocShell() );

    // Expr-Felder in Shared-Formaten?
 // if( nStatus & SWGSTAT_UPDATEEXPR )
 //     pDoc->SetUpdateExpFldStat();
    if( !IsError( nErrno ))
    {
        if( nOptions == SWGRD_NORMAL )
        {
            // Neues Doc gelesen
            if( bNew )
                pDoc->SetLoaded( TRUE );
        }
        else
        {
            // Nachladen von Vorlagen etc.

            // falls wir die OutlineLevel veraendert haben (Volagen geladen),
            // dann muss am Doc ein Update auf die neuen Nummern erfolgen !!
            if( (SWGRD_FORCE & nOptions) && pDoc->GetOutlineNumRule() )
                pDoc->SetOutlineNumRule( *pDoc->GetOutlineNumRule() );

            pDoc->SetModified();
        }
    }
    if( bNew )
        pDoc->SetInfo( *pInfo );
    pDoc->SetNewDoc( FALSE );
    return nErrno;
 }

// Einlesen einer Textblock-Datei
// Hier wird alles ausser dem Inhalt eingelesen.
// Der Inhalt wird nach SWG_TEXTBLOCK-Records durchsucht.
// Dabei werden in allen Textbloecken die Referenzzaehler fuer Formate
// inkrementiert.

// Ansonsten ist die Funktion analog zu Read().




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
