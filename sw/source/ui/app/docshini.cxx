/*************************************************************************
 *
 *  $RCSfile: docshini.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-31 20:32:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif

//Statt uiparam.hxx selbst definieren, das spart keys
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST

#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OFA_MISCCFG_HXX //autogen
#include <sfx2/misccfg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif


#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#define ITEMID_COLOR_TABLE SID_COLOR_TABLE
#include <svx/drawitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif


#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCFAC_HXX
#include <docfac.hxx>
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>        // I/O, Hausformat
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DATAEX_HXX
#include <dataex.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _UNOTXDOC_HXX
#include <unotxdoc.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

extern sal_Bool bNotLoadLayout;


using namespace ::com::sun::star;
using namespace ::rtl;
/*-----------------21.09.96 15.29-------------------

--------------------------------------------------*/


void lcl_SetColl(SwDoc* pDoc, sal_uInt16 nType,
                    SfxPrinter* pPrt, const String& rStyle)
{
    sal_Bool bDelete = sal_False;
    const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(rStyle): 0;
    if(!pFnt)
    {
        pFnt = new SfxFont( FAMILY_DONTKNOW, rStyle, PITCH_DONTKNOW,
                            ::gsl_getSystemTextEncoding() );
        bDelete = sal_True;
    }
    SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(nType);
    pColl->SetAttr(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                        aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet()));
    if(bDelete)
    {
        delete (SfxFont*) pFnt;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::InitNew( SvStorage * pStor )
{
    sal_Bool bRet = SfxInPlaceObject::InitNew( pStor );
    SetMapUnit( MAP_TWIP );
    if( bRet )
    {
        AddLink();      // pDoc / pIo ggf. anlegen

        if ( ISA( SwWebDocShell ) )
            SetHTMLTemplate( *GetDoc() );   //Styles aus HTML.vor
        else if( ISA( SwGlobalDocShell ) )
            GetDoc()->SetGlobalDoc();       // Globaldokument

        // fuer alle

        SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
        SfxPrinter* pPrt = pDoc->GetPrt();

        String sEntry;
        if(!pStdFont->IsFontDefault(FONT_STANDARD))
        {
            sEntry = pStdFont->GetFontStandard();
            sal_Bool bDelete = sal_False;
            const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sEntry): 0;
            if(!pFnt)
            {
                pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
                                    ::gsl_getSystemTextEncoding() );
                bDelete = sal_True;
            }
            pDoc->SetDefault(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                                aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet()));
            SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetAttr(RES_CHRATR_FONT);
            if(bDelete)
            {
                delete (SfxFont*) pFnt;
                bDelete = sal_False;
            }

//              lcl_SetColl(pDoc, RES_POOLCOLL_STANDARD, pPrt, sEntry);
        }

        if(!pStdFont->IsFontDefault(FONT_OUTLINE))
        {
            sEntry = pStdFont->GetFontOutline();
            lcl_SetColl(pDoc, RES_POOLCOLL_HEADLINE_BASE, pPrt, sEntry);
        }

        if(!pStdFont->IsFontDefault(FONT_LIST))
        {
            sEntry = pStdFont->GetFontList();
            lcl_SetColl(pDoc, RES_POOLCOLL_NUMBUL_BASE, pPrt, sEntry);
        }

        if(!pStdFont->IsFontDefault(FONT_CAPTION))
        {
            sEntry = pStdFont->GetFontCaption();
            lcl_SetColl(pDoc, RES_POOLCOLL_LABEL, pPrt, sEntry);
        }

        if(!pStdFont->IsFontDefault(FONT_INDEX))
        {
            sEntry = pStdFont->GetFontIndex();
            lcl_SetColl(pDoc, RES_POOLCOLL_REGISTER_BASE, pPrt, sEntry);
        }


/*
        //JP 12.07.95: so einfach waere es fuer die neu Mimik
        pDoc->SetDefault( SvxTabStopItem( 1,
                    GetStar Writer App()->GetUsrPref()->GetDefTabDist(),
                    SVX_TAB_ADJUST_DEFAULT,
                    RES_PARATR_TABSTOP));
*/
        if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
        {
            // fuer MWERKS (Mac-Compiler): kann nicht selbststaendig casten
            SvEmbeddedObject* pObj = this;
            SwDataExchange::InitOle( pObj, pDoc );
        }
        SubInitNew();
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Ctor mit SfxCreateMode ?????
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell(SfxObjectCreateMode eMode) :
    pDoc(0),
    pIo(0),
    pBasePool(0),
    pFontList(0),
    SfxObjectShell ( eMode ),
    pView( 0 ),
    pWrtShell( 0 )
{
    SetShell(this);
    SetBaseModel(new SwXTextDocument(this));
    // wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
    StartListening( *this );
}

/*--------------------------------------------------------------------
    Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode ):
    pDoc(pD),
    pIo(0),
    pBasePool(0),
    pFontList(0),
    SfxObjectShell ( eMode ),
    pView( 0 ),
    pWrtShell( 0 )
{
    SetShell(this);
    SetBaseModel(new SwXTextDocument(this));
    // wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
    StartListening( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:   Dtor
 --------------------------------------------------------------------*/


 SwDocShell::~SwDocShell()
{
    RemoveLink();
    delete pIo;
    delete pFontList;

    // wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
    EndListening( *this );
    SvxColorTableItem* pColItem = (SvxColorTableItem*)GetItem(SID_COLOR_TABLE);
    // wird nur die DocInfo fuer den Explorer gelesen, ist das Item nicht da
    if(pColItem)
    {
        XColorTable* pTable = pColItem->GetColorTable();
        // wurde eine neue Table angelegt, muss sie auch geloescht werden.
        if((void*)pTable  != (void*)(OFF_APP())->GetStdColorTable())
            delete pTable;
    }
}

/*--------------------------------------------------------------------
    Beschreibung: AddLink
 --------------------------------------------------------------------*/


void SwDocShell::AddLink()
{
    if( !pDoc )
    {
        SwDocFac aFactory;
        pDoc = aFactory.GetDoc();
        pDoc->AddLink();
        pDoc->SetHTMLMode( ISA(SwWebDocShell) );
    }
    else
        pDoc->AddLink();
    pDoc->SetDocShell( this );      // am Doc den DocShell-Pointer setzen
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);

    if( !pIo )
        pIo = new Sw3Io( *pDoc );
    else
        pIo->SetDoc( *pDoc );

    SetPool(&pDoc->GetAttrPool());

    // am besten erst wenn eine sdbcx::View erzeugt wird !!!
    pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:   neue FontList erzeugen Aenderung Drucker
 --------------------------------------------------------------------*/


void SwDocShell::UpdateFontList()
{
    ASSERT(pDoc, "Kein Doc keine FontList");
    if( pDoc )
    {
        SfxPrinter* pPrt = pDoc->GetPrt();
        delete pFontList;

        if( pPrt && pPrt->GetDevFontCount() && !pDoc->IsBrowseMode() )
            pFontList = new FontList( pPrt );
        else
            pFontList = new FontList( Application::GetDefaultDevice() );

        PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
    }
}

/*--------------------------------------------------------------------
    Beschreibung: RemoveLink
 --------------------------------------------------------------------*/


void SwDocShell::RemoveLink()
{
    // Uno-Object abklemmen
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Invalidate();
    aFinishedTimer.Stop();
    if(pDoc)
    {
        DELETEZ(pBasePool);
        sal_Int8 nRefCt = pDoc->RemoveLink();
        pDoc->SetOle2Link(Link());
        pDoc->SetDocShell( 0 );
        if( !nRefCt )
            delete pDoc;
        pDoc = 0;       // wir haben das Doc nicht mehr !!
    }
}


/*--------------------------------------------------------------------
    Beschreibung: Laden, Default-Format
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::Load(SvStorage* pStor)
{
    sal_Bool bRet = sal_False;
    if(SfxInPlaceObject::Load( pStor ))
    {
        if( pDoc )              // fuer Letzte Version !!
            RemoveLink();       // das existierende Loslassen

        AddLink();      // Link setzen und Daten updaten !!

        // Das Laden
        // fuer MD
        sal_Bool bSave = bNotLoadLayout;
        bNotLoadLayout = sal_Bool( !LoadLayout() );
#ifndef PRODUCT
        sal_Bool bWeb = 0 != PTR_CAST(SwWebDocShell, this);
        bNotLoadLayout |= SW_MOD()->GetUsrPref(bWeb)->IsTest1();
#endif
        SwWait aWait( *this, sal_True );
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        switch( GetCreateMode() )
        {
//      case SFX_CREATE_MODE_INTERNAL:
//          nErr = 0;
//          break;

        case SFX_CREATE_MODE_ORGANIZER:
            nErr = pIo->LoadStyles( pStor );
            break;

        case SFX_CREATE_MODE_INTERNAL:
            bNotLoadLayout = sal_True;
            // kein break;

        case SFX_CREATE_MODE_EMBEDDED:
            // SfxProgress unterdruecken, wenn man Embedded ist
            SW_MOD()->SetEmbeddedLoadSave( sal_True );
            // kein break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
            if( ReadSw3 )
            {
                // die DocInfo vom Doc am DocShell-Medium setzen
                {
                    SfxDocumentInfo aInfo;
                    aInfo.Load( pStor );
                    pDoc->DocInfoChgd( aInfo );
                }
                SwReader aRdr( *pStor, aEmptyStr, pDoc );
                nErr = aRdr.Read( *ReadSw3 );
            }
#ifndef PRODUCT
            else
                ASSERT( !this, "ohne Sw3Reader geht nichts" );
#endif
            break;

#ifndef PRODUCT
        default:
            ASSERT( !this, "Load: new CreateMode?" );
#endif

        }
        bNotLoadLayout = bSave;

        ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
        pBasePool = new SwDocStyleSheetPool( *pDoc,
                        SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
        UpdateFontList();
        InitDraw();

        SetError( nErr );
        bRet = !IsError( nErr );

        // StartFinishedLoading rufen.
        if( bRet && !pDoc->IsInLoadAsynchron() &&
            GetCreateMode() == SFX_CREATE_MODE_STANDARD )
            StartLoadFinishedTimer();

        // SfxProgress unterdruecken, wenn man Embedded ist
        SW_MOD()->SetEmbeddedLoadSave( sal_False );

        if( pDoc->IsGlobalDoc() && !pDoc->IsGlblDocSaveLinks() )
        {
            // dann entferne alle nicht referenzierte OLE-Objecte
            SvStorageInfoList aInfoList;
            pStor->FillInfoList( &aInfoList );

            // erstmal alle nicht "OLE-Objecte" aus der Liste entfernen
            for( sal_uInt32 n = aInfoList.Count(); n; )
            {
                const String& rName = aInfoList.GetObject( --n ).GetName();
                // in ndole.cxx wird dieser PreFix benutzt
                if( 3 != rName.Match( String::CreateFromAscii("Obj") ))
                    aInfoList.Remove( n );
            }

            // dann alle referenzierten Object aus der Liste entfernen
            SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
            for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
                    pNd; pNd = (SwCntntNode*)aIter.Next() )
            {
                SwOLENode* pOLENd = pNd->GetOLENode();
                if( pOLENd )
                {
                    const String& rOLEName = pOLENd->GetOLEObj().GetName();
                    for( n = aInfoList.Count(); n; )
                    {
                        const String& rName = aInfoList.GetObject( --n ).GetName();
                        if( rOLEName == rName )
                        {
                            aInfoList.Remove( n );
                            break;
                        }
                    }
                }
            }
            // und jetzt haben wir alle Objecte, die nicht mehr
            // referenziert werden
            SvPersist* p = this;
            for( n = aInfoList.Count(); n; )
            {
                const String& rName = aInfoList.GetObject( --n ).GetName();
                SvInfoObjectRef aRef( p->Find( rName ) );
                if( aRef.Is() )
                    p->Remove( &aRef );
            }
        }
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::LoadFrom(SvStorage* pStor)
{
    sal_Bool bRet = sal_False;
    if( pDoc )
        RemoveLink();

    AddLink();      // Link setzen und Daten updaten !!

    do {        // middle check loop
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        const String& rNm = pStor->GetName();
        if( pStor->IsStream( SfxStyleSheetBasePool::GetStreamName() ))
        {
            // Das Laden
            SwWait aWait( *this, sal_True );
            nErr = pIo->LoadStyles( pStor );
        }
        else
        {
            // sollte es sich um eine 2. Vrolage handeln ??
            if( SvStorage::IsStorageFile( rNm ) )
                break;

            const SfxFilter* pFltr = SwIoSystem::GetFileFilter( rNm, aEmptyStr );
            if( !pFltr || !pFltr->GetUserData().EqualsAscii( FILTER_SWG ))
                break;

            SfxMedium aMed( rNm, STREAM_STD_READ, FALSE );
            if( 0 == ( nErr = aMed.GetInStream()->GetError() ) )
            {
                SwWait aWait( *this, sal_True );
                SwReader aRead( aMed, rNm, pDoc );
                nErr = aRead.Read( *ReadSwg );
            }
        }

        ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
        pBasePool = new SwDocStyleSheetPool( *pDoc,
                            SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );

        SetError( nErr );
        bRet = !IsError( nErr );

    } while( sal_False );

    SfxObjectShell::LoadFrom( pStor );
    pDoc->ResetModified();
    return bRet;
}


SvDataMemberObjectRef  SwDocShell::CreateSnapshot()
{
    SvDataMemberObjectRef aRef;
    if( pDoc )
        aRef = new SwDataExchange( *pDoc );
    return aRef;
}


void SwDocShell::SubInitNew()
{
    ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
    pBasePool = new SwDocStyleSheetPool( *pDoc,
                    SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    UpdateFontList();
    InitDraw();

//  const SwModuleOptions& rModCfg = *SW_MOD()->GetModuleConfig();
//  pDoc->SetLinkUpdMode( rModCfg.GetLinkMode() );
//  pDoc->SetFldUpdateFlags( rModCfg.GetFldUpdateFlags() );
    pDoc->SetLinkUpdMode( GLOBALSETTING );
    pDoc->SetFldUpdateFlags( AUTOUPD_GLOBALSETTING );

    sal_Bool bWeb = ISA(SwWebDocShell);

    sal_uInt16 nRange[] =   {
                            RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
                            0, 0, 0  };
    if(!bWeb)
    {
        nRange[2] = RES_PARATR_TABSTOP;
        nRange[3] = RES_PARATR_HYPHENZONE;
    }
    SfxItemSet aDfltSet( pDoc->GetAttrPool(), nRange );
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );

    sal_Int16 nVal;
    if (xProp.is())
    {
        xProp->getPropertyValue( OUString::createFromAscii(UPN_DEFAULT_LANGUAGE)) >>= nVal;
    }
    else
    {   // guess DefaultLanguage to be used from other sources
        nVal = Application::GetAppInternational().GetLanguage();
        if( nVal == LANGUAGE_SYSTEM )
            nVal = ::GetSystemLanguage();
    }

    aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    if(!bWeb)
    {
        SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) pDoc->GetDefault(
                                                        RES_PARATR_HYPHENZONE) );
        if( xProp.is() )
            xProp->getPropertyValue(
                OUString::createFromAscii(UPN_HYPH_MIN_LEADING) ) >>= nVal;
        else
            nVal = 2;
        aHyp.GetMinLead()   = sal_Int8(nVal);

        if(xProp.is())
            xProp->getPropertyValue(
                OUString::createFromAscii(UPN_HYPH_MIN_TRAILING) ) >>= nVal;
        else
            nVal = 2;
        aHyp.GetMinTrail()  = sal_Int8(nVal);

        aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = SW_MOD()->GetUsrPref(FALSE)->GetDefTab();
        if( nNewPos )
            aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                            SVX_TAB_ADJUST_DEFAULT ) );
    }
    pDoc->SetDefault( aDfltSet );
    pDoc->ResetModified();
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.5  2000/10/31 15:44:59  hr
    #65293#: includes

    Revision 1.4  2000/10/30 20:30:08  jp
    Bug #79779#: BrushGraphicCache removed

    Revision 1.3  2000/10/09 18:15:05  jp
    Bug #78395#: set fontcharset to systemcharset

    Revision 1.2  2000/09/28 15:22:17  os
    use of configuration service in view options

    Revision 1.1.1.1  2000/09/18 17:14:31  hr
    initial import

    Revision 1.209  2000/09/18 16:05:11  willem.vandorp
    OpenOffice header added.

    Revision 1.208  2000/09/08 15:11:56  os
    use configuration service

    Revision 1.207  2000/07/13 12:36:19  os
    new: SwXTextDocument::reactivate()

    Revision 1.206  2000/07/05 10:28:14  os
    invalidate model in ::RemoveLink()

    Revision 1.205  2000/06/26 10:38:34  jp
    must change: GetAppWindow->GetDefaultDevice

    Revision 1.204  2000/04/11 08:01:30  os
    UNICODE

    Revision 1.203  2000/03/21 15:47:50  os
    UNOIII

    Revision 1.202  2000/03/03 15:16:58  os
    StarView remainders removed

    Revision 1.201  2000/02/02 17:00:37  jp
    Task #72579#: interface of SwReader is changed

    Revision 1.200  2000/01/19 18:26:37  jp
    Bug #72117#: create numberformatter only if he is needed

    Revision 1.199  1999/12/10 15:49:58  tl
    #70667# shadowing of variable fixed

    Revision 1.198  1999/12/10 13:05:52  tl
    #70383# SvxGetLinguPropertySet => ::GetLinguPropertySet

    Revision 1.197  1999/11/24 18:24:13  tl
    check for Service availability

    Revision 1.196  1999/11/19 16:40:22  os
    modules renamed

    Revision 1.195  1999/11/15 10:30:05  tl
    some property name changes for ONE_LINGU

    Revision 1.194  1999/10/25 19:10:08  tl
    ongoing ONE_LINGU implementation

    Revision 1.193  1999/08/31 08:34:00  TL
    #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)


      Rev 1.192   31 Aug 1999 10:34:00   TL
   #if[n]def ONE_LINGU inserted (for transition of lingu to StarOne)

      Rev 1.191   08 Jul 1999 15:59:34   MA
   Use internal object to toggle wait cursor

      Rev 1.190   07 Jul 1999 08:25:26   OS
   extended indexes: create index types in SwDoc::Ctor

      Rev 1.189   15 Mar 1999 15:07:32   JP
   Task #61405#: AutoCompleteList wurde statisch

      Rev 1.188   11 Mar 1999 23:58:40   JP
   Task #63171#: Optionen fuer Feld-/LinkUpdate Doc oder Modul lokal, Task #61405: Optionen setzen

      Rev 1.187   09 Mar 1999 15:36:10   OS
   #62742# Invalidate am Model rufen, nicht InitNewDoc

      Rev 1.186   04 Feb 1999 11:22:14   ER
   #61415# OfaMiscCfg nach SFX verschoben

      Rev 1.185   02 Feb 1999 08:42:48   OS
   #61027# zweistellige Jahreszahlen

      Rev 1.184   01 Feb 1999 08:23:24   OS
   #56371# unnoetigen include wieder raus

      Rev 1.183   27 Jan 1999 09:51:54   OS
   #56371# TF_ONE51

      Rev 1.182   10 Dec 1998 15:55:18   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.181   25 Nov 1998 11:26:54   OS
   #59240##59637# SubInitNew vor dem Import, kein style::TabStop fuer HTML

      Rev 1.180   23 Nov 1998 16:12:16   OM
   #58216# Verknuepfungsoptionen pro Dokument

      Rev 1.179   11 Nov 1998 09:04:20   OS
   #59240# keine Defaults fuer style::TabStop und Hyphenation in der WebDocShell

      Rev 1.178   19 Aug 1998 15:18:28   OM
   #55274# Default-Datenbanknamen am Dokument nur OnDemand setzen

      Rev 1.177   24 Apr 1998 17:17:30   JP
   Bug #49791#: nur auf OLE casten wenn auch OLE drin ist (und nicht Grafik)

      Rev 1.176   21 Apr 1998 12:37:56   JP
   Bug #47798#: GlobalDocument ohne Inhalt laden - nicht referenzierte Objecte loeschen

      Rev 1.175   17 Feb 1998 09:19:04   TJ
   IFDEF TF _STARONE

      Rev 1.174   12 Feb 1998 15:00:10   OS
   Model im Dtor invalidieren

      Rev 1.173   11 Feb 1998 16:31:30   OS
   SwXTextDocument wieder aktiv

      Rev 1.172   04 Feb 1998 17:22:50   OS
   SetModel erstmal raus

------------------------------------------------------------------------*/


