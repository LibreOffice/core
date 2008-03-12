/*************************************************************************
 *
 *  $RCSfile: docshini.cxx,v $
 *
 *  $Revision: 1.65 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:44:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _SOT_STORINFO_HXX
#include <sot/storinfo.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _OFA_MISCCFG_HXX //autogen
#include <svtools/misccfg.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_ASIANCFG_HXX
#include <svx/asiancfg.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_AKRNTEM_HXX
#include <svx/akrnitem.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen

#include <svx/drawitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
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
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _MySVXACORR_HXX //autogen
#include <svx/svxacorr.hxx>
#endif

#include <vcl/svapp.hxx>

#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#ifndef _PRTOPT_HXX
#include <prtopt.hxx>
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
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
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
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

// #107253#
#ifndef _SWLINGUCONFIG_HXX
#include <swlinguconfig.hxx>
#endif

// #i18732#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif

#include <unochart.hxx>

// text grid
#include <tgrditem.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;

/*-----------------21.09.96 15.29-------------------

--------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


sal_Bool SwDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::InitNew" );

    sal_Bool bRet = SfxObjectShell::InitNew( xStor );
    ASSERT( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
    sal_Bool bHTMLTemplSet = sal_False;
    if( bRet )
    {
        AddLink();      // pDoc / pIo ggf. anlegen

        sal_Bool bWeb = ISA( SwWebDocShell );
        if ( bWeb )
            bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );//Styles aus HTML.vor
        else if( ISA( SwGlobalDocShell ) )
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);       // Globaldokument


/*
        //JP 12.07.95: so einfach waere es fuer die neu Mimik
        pDoc->SetDefault( SvxTabStopItem( 1,
                    GetStar Writer App()->GetUsrPref()->GetDefTabDist(),
                    SVX_TAB_ADJUST_DEFAULT,
                    RES_PARATR_TABSTOP));
*/
        if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
            SwTransferable::InitOle( this, *pDoc );

        // set forbidden characters if necessary
        SvxAsianConfig aAsian;
        Sequence<Locale> aLocales =  aAsian.GetStartEndCharLocales();
        if(aLocales.getLength())
        {
            const Locale* pLocales = aLocales.getConstArray();
            for(sal_Int32 i = 0; i < aLocales.getLength(); i++)
            {
                ForbiddenCharacters aForbidden;
                aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine);
                LanguageType  eLang = SvxLocaleToLanguage(pLocales[i]);
                pDoc->setForbiddenCharacters( eLang, aForbidden);
            }
        }
        pDoc->set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION,
                  !aAsian.IsKerningWesternTextOnly());
        pDoc->setCharacterCompressionType(static_cast<SwCharCompressType>(aAsian.GetCharDistanceCompression()));
        pDoc->setPrintData(*SW_MOD()->GetPrtOptions(bWeb));

        SubInitNew();

        // fuer alle

        SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
        SfxPrinter* pPrt = pDoc->getPrinter( false );

        String sEntry;
        USHORT aFontWhich[] =
        {   RES_CHRATR_FONT,
            RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_FONT
        };
        USHORT aFontHeightWhich[] =
        {
            RES_CHRATR_FONTSIZE,
            RES_CHRATR_CJK_FONTSIZE,
            RES_CHRATR_CTL_FONTSIZE
        };
        USHORT aFontIds[] =
        {
            FONT_STANDARD,
            FONT_STANDARD_CJK,
            FONT_STANDARD_CTL
        };
        USHORT nFontTypes[] =
        {
            DEFAULTFONT_LATIN_TEXT,
            DEFAULTFONT_CJK_TEXT,
            DEFAULTFONT_CTL_TEXT
        };
        USHORT aLangTypes[] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        for(sal_uInt8 i = 0; i < 3; i++)
        {
            USHORT nFontWhich = aFontWhich[i];
            USHORT nFontId = aFontIds[i];
            SvxFontItem* pFontItem = 0;
            const SvxLanguageItem& rLang = (const SvxLanguageItem&)pDoc->GetDefault( aLangTypes[i] );
            LanguageType eLanguage = rLang.GetLanguage();
            if(!pStdFont->IsFontDefault(nFontId))
            {
                sEntry = pStdFont->GetFontFor(nFontId);
                sal_Bool bDelete = sal_False;
                const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sEntry): 0;
                if(!pFnt)
                {
                    pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
                                        ::gsl_getSystemTextEncoding() );
                    bDelete = sal_True;
                }
                pFontItem = new SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                                    aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(), nFontWhich);
                if(bDelete)
                {
                    delete (SfxFont*) pFnt;
                    bDelete = sal_False;
                }
            }
            else
            {
                // #107782# OJ use korean language if latin was used
                if ( i == 0 )
                {
                    LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
                    switch( eUiLanguage )
                    {
                        case LANGUAGE_KOREAN:
                        case LANGUAGE_KOREAN_JOHAB:
                            eLanguage = eUiLanguage;
                        break;
                    }
                }

                Font aLangDefFont = OutputDevice::GetDefaultFont(
                    nFontTypes[i],
                    eLanguage,
                    DEFAULTFONT_FLAGS_ONLYONE );
                pFontItem = new SvxFontItem(aLangDefFont.GetFamily(), aLangDefFont.GetName(),
                                    aEmptyStr, aLangDefFont.GetPitch(), aLangDefFont.GetCharSet(), nFontWhich);
            }
            pDoc->SetDefault(*pFontItem);
            if( !bHTMLTemplSet )
            {
                SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
                pColl->ResetAttr(nFontWhich);
            }
            delete pFontItem;
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( FONT_STANDARD, i, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( nFontId, eLanguage );
            pDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
            if( !bHTMLTemplSet )
            {
                SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
                pColl->ResetAttr(aFontHeightWhich[i]);
            }

        }
        USHORT aFontIdPoolId[] =
        {
            FONT_OUTLINE,       RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST,          RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION,       RES_POOLCOLL_LABEL,
            FONT_INDEX,         RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CJK,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CJK,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CJK,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CJK,     RES_POOLCOLL_REGISTER_BASE,
            FONT_OUTLINE_CTL,   RES_POOLCOLL_HEADLINE_BASE,
            FONT_LIST_CTL,      RES_POOLCOLL_NUMBUL_BASE,
            FONT_CAPTION_CTL,   RES_POOLCOLL_LABEL,
            FONT_INDEX_CTL,     RES_POOLCOLL_REGISTER_BASE
        };

        USHORT nFontWhich = RES_CHRATR_FONT;
        USHORT nFontHeightWhich = RES_CHRATR_FONTSIZE;
        LanguageType eLanguage = static_cast<const SvxLanguageItem&>(pDoc->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();
        for(sal_uInt8 nIdx = 0; nIdx < 24; nIdx += 2)
        {
            if(nIdx == 8)
            {
                nFontWhich = RES_CHRATR_CJK_FONT;
                nFontHeightWhich = RES_CHRATR_CJK_FONTSIZE;
                eLanguage = static_cast<const SvxLanguageItem&>(pDoc->GetDefault( RES_CHRATR_CJK_LANGUAGE )).GetLanguage();
            }
            else if(nIdx == 16)
            {
                nFontWhich = RES_CHRATR_CTL_FONT;
                nFontHeightWhich = RES_CHRATR_CTL_FONTSIZE;
                eLanguage = static_cast<const SvxLanguageItem&>(pDoc->GetDefault( RES_CHRATR_CTL_LANGUAGE )).GetLanguage();
            }
            SwTxtFmtColl *pColl = 0;
            if(!pStdFont->IsFontDefault(aFontIdPoolId[nIdx]))
            {
                sEntry = pStdFont->GetFontFor(aFontIdPoolId[nIdx]);
                sal_Bool bDelete = sal_False;
                const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sEntry): 0;
                if(!pFnt)
                {
                    pFnt = new SfxFont( FAMILY_DONTKNOW, sEntry, PITCH_DONTKNOW,
                                        ::gsl_getSystemTextEncoding() );
                    bDelete = sal_True;
                }
                pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
                if( !bHTMLTemplSet ||
                    SFX_ITEM_SET != pColl->GetAttrSet().GetItemState(
                                                    nFontWhich, sal_False ) )
                {
                    pColl->SetAttr(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                                        aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet(), nFontWhich));
                }
                if(bDelete)
                {
                    delete (SfxFont*) pFnt;
                }
            }
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( static_cast< sal_Int8 >(aFontIdPoolId[nIdx]), 0, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( aFontIdPoolId[nIdx], eLanguage );
            if(!pColl)
                pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
            SvxFontHeightItem aFontHeight( (const SvxFontHeightItem&)pColl->GetAttr( nFontHeightWhich, sal_True ));
            if(aFontHeight.GetHeight() != sal::static_int_cast<sal_uInt32, sal_Int32>(nFontHeight))
            {
                aFontHeight.SetHeight(nFontHeight);
                pColl->SetAttr( aFontHeight );
            }
        }
    }

    /* #106748# If the default frame direction of a document is RTL
        the default adjusment is to the right. */
    if( !bHTMLTemplSet &&
        FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
        pDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST ) );

    // OD 09.10.2003 #i18732# - set dynamic pool default for
    // item RES_FOLLOW_TEXT_FLOW to FALSE for *new document*.
    // Thus, redo this change in method <SwDoc::RemoveAllFmtLanguageDependencies()>,
    // which is called from <SwDocShell::ConvertFrom(..)> in order to restore
    // the static pool default.
    pDoc->SetDefault( SwFmtFollowTextFlow( FALSE ) );

// --> collapsing borders FME 2005-05-27 #i29550#
    pDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, TRUE ) );
// <-- collapsing

    //#i16874# AutoKerning as default for new documents
    pDoc->SetDefault( SvxAutoKernItem( sal_True, RES_CHRATR_AUTOKERN ) );

    // --> OD 2005-02-10 #i42080# - Due to the several calls of method <SetDefault(..)>
    // at the document instance, the document is modified. Thus, reset this
    // status here. Note: In method <SubInitNew()> this is also done.
    pDoc->ResetModified();
    // <--

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:   Ctor mit SfxCreateMode ?????
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( SfxObjectCreateMode eMode, sal_Bool _bScriptingSupport ) :
    SfxObjectShell ( eMode ),
    pDoc(0),
    pFontList(0),
    pView( 0 ),
    pWrtShell( 0 ),
    pOLEChildList( 0 ),
    nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    bInUpdateFontList(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
    if ( !_bScriptingSupport )
        SetHasNoBasic();
    Init_Impl();
}

/*--------------------------------------------------------------------
    Beschreibung: Ctor / Dtor
 --------------------------------------------------------------------*/


SwDocShell::SwDocShell( SwDoc *pD, SfxObjectCreateMode eMode ):
    SfxObjectShell ( eMode ),
    pDoc(pD),
    pFontList(0),
    pView( 0 ),
    pWrtShell( 0 ),
    pOLEChildList( 0 ),
    nUpdateDocMode(document::UpdateDocMode::ACCORDING_TO_CONFIG),
    bInUpdateFontList(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::SwDocShell" );
    Init_Impl();
}

/*--------------------------------------------------------------------
    Beschreibung:   Dtor
 --------------------------------------------------------------------*/


 SwDocShell::~SwDocShell()
{
    // disable chart related objects now because in ~SwDoc it may be to late for this
    if( pDoc )
    {
        pDoc->GetChartControllerHelper().Disconnect();
        SwChartDataProvider *pPCD = pDoc->GetChartDataProvider();
        if (pPCD)
            pPCD->dispose();
    }

    RemoveLink();
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
        if((void*)pTable  != (void*)(XColorTable::GetStdColorTable()) )
            delete pTable;
    }

    delete pOLEChildList;
}
/* -----------------------------10.09.2001 15:59------------------------------

 ---------------------------------------------------------------------------*/
void  SwDocShell::Init_Impl()
{
    SetPool(&SW_MOD()->GetPool());
    SetBaseModel(new SwXTextDocument(this));
    // wir als BroadCaster werden auch unser eigener Listener
    // (fuer DocInfo/FileNamen/....)
    StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
    SetAutoStyleFilterIndex(3);

    // set map unit to twip
    SetMapUnit( MAP_TWIP );
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
        pDoc->acquire();
        pDoc->set(IDocumentSettingAccess::HTML_MODE, ISA(SwWebDocShell) );
    }
    else
        pDoc->acquire();
    pDoc->SetDocShell( this );      // am Doc den DocShell-Pointer setzen
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);

    SetPool(&pDoc->GetAttrPool());

    // am besten erst wenn eine sdbcx::View erzeugt wird !!!
    pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:   neue FontList erzeugen Aenderung Drucker
 --------------------------------------------------------------------*/


void SwDocShell::UpdateFontList()
{
    if(!bInUpdateFontList)
    {
        bInUpdateFontList = true;
        ASSERT(pDoc, "Kein Doc keine FontList");
        if( pDoc )
        {
            delete pFontList;
            pFontList = new FontList( pDoc->getReferenceDevice( true ) );
            PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        bInUpdateFontList = false;
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
        if( mxBasePool.is() )
        {
            static_cast<SwDocStyleSheetPool*>(mxBasePool.get())->dispose();
            mxBasePool.clear();
        }
        sal_Int8 nRefCt = static_cast< sal_Int8 >(pDoc->release());
        pDoc->SetOle2Link(Link());
        pDoc->SetDocShell( 0 );
        if( !nRefCt )
            delete pDoc;
        pDoc = 0;       // wir haben das Doc nicht mehr !!
    }
}
void SwDocShell::InvalidateModel()
{
    // Uno-Object abklemmen
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Invalidate();
}
void SwDocShell::ReactivateModel()
{
    // Uno-Object abklemmen
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);
}

/*--------------------------------------------------------------------
    Beschreibung: Laden, Default-Format
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::Load( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Load" );
    sal_Bool bRet = sal_False;
    if( SfxObjectShell::Load( rMedium ))
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "after SfxInPlaceObject::Load" );
        if( pDoc )              // fuer Letzte Version !!
            RemoveLink();       // das existierende Loslassen

        AddLink();      // Link setzen und Daten updaten !!

        // Das Laden
        // fuer MD
            ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
            mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
            if(GetCreateMode() != SFX_CREATE_MODE_ORGANIZER)
            {
                SFX_ITEMSET_ARG( rMedium.GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
                nUpdateDocMode = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;
            }

        SwWait aWait( *this, sal_True );
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        switch( GetCreateMode() )
        {
//      case SFX_CREATE_MODE_INTERNAL:
//          nErr = 0;
//          break;

        case SFX_CREATE_MODE_ORGANIZER:
            {
                if( ReadXML )
                {
                    ReadXML->SetOrganizerMode( TRUE );
                    SwReader aRdr( rMedium, aEmptyStr, pDoc );
                    nErr = aRdr.Read( *ReadXML );
                    ReadXML->SetOrganizerMode( FALSE );
                }
            }
            break;

        case SFX_CREATE_MODE_INTERNAL:
        case SFX_CREATE_MODE_EMBEDDED:
            {
                // fuer MWERKS (Mac-Compiler): kann nicht selbststaendig casten
                SwTransferable::InitOle( this, *pDoc );
            }
            // SfxProgress unterdruecken, wenn man Embedded ist
            SW_MOD()->SetEmbeddedLoadSave( sal_True );
            // kein break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
            {
                Reader *pReader = ReadXML;
                if( pReader )
                {
                    // die DocInfo vom Doc am DocShell-Medium setzen
                    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before ReadDocInfo" );
                    SwReader aRdr( rMedium, aEmptyStr, pDoc );
                    RTL_LOGFILE_CONTEXT_TRACE( aLog, "before Read" );
                    nErr = aRdr.Read( *pReader );
                    RTL_LOGFILE_CONTEXT_TRACE( aLog, "after Read" );

                    // If a XML document is loaded, the global doc/web doc
                    // flags have to be set, because they aren't loaded
                    // by this formats.
                    if( ISA( SwWebDocShell ) )
                    {
                        if( !pDoc->get(IDocumentSettingAccess::HTML_MODE) )
                            pDoc->set(IDocumentSettingAccess::HTML_MODE, true);
                    }
                    if( ISA( SwGlobalDocShell ) )
                    {
                        if( !pDoc->get(IDocumentSettingAccess::GLOBAL_DOCUMENT) )
                            pDoc->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);
                    }
                }
#ifndef PRODUCT
                else
                    ASSERT( !this, "ohne Sw3Reader geht nichts" );
#endif
            }
            break;

        default:
            ASSERT( !this, "Load: new CreateMode?" );

        }

        UpdateFontList();
        InitDraw();

        SetError( nErr );
        bRet = !IsError( nErr );

        // --> OD 2006-11-07 #i59688#
//        // StartFinishedLoading rufen.
//        if( bRet && !pDoc->IsInLoadAsynchron() &&
//            GetCreateMode() == SFX_CREATE_MODE_STANDARD )
//            StartLoadFinishedTimer();
        if ( bRet && !pDoc->IsInLoadAsynchron() &&
             GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        {
            LoadingFinished();
        }
        // <--

        // SfxProgress unterdruecken, wenn man Embedded ist
        SW_MOD()->SetEmbeddedLoadSave( sal_False );
    }

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


sal_Bool  SwDocShell::LoadFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::LoadFrom" );
    sal_Bool bRet = sal_False;
    if( pDoc )
        RemoveLink();

    AddLink();      // Link setzen und Daten updaten !!

    do {        // middle check loop
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        //const String& rNm = pStor->GetName();
        String aStreamName;
        aStreamName = String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("styles.xml"));
        uno::Reference < container::XNameAccess > xAccess( rMedium.GetStorage(), uno::UNO_QUERY );
        if ( xAccess->hasByName( aStreamName ) && rMedium.GetStorage()->isStreamElement( aStreamName ) )
        {
            // Das Laden
            SwWait aWait( *this, sal_True );
            {
                ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
                mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
                if( ReadXML )
                {
                    ReadXML->SetOrganizerMode( TRUE );
                    SwReader aRdr( rMedium, aEmptyStr, pDoc );
                    nErr = aRdr.Read( *ReadXML );
                    ReadXML->SetOrganizerMode( FALSE );
                }
            }
        }
        else
        {
            DBG_ERROR("Code removed!");
        /*
        //TODO/LATER: looks like some binary stuff?!
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
         */
        }

        SetError( nErr );
        bRet = !IsError( nErr );

    } while( sal_False );

    SfxObjectShell::LoadFrom( rMedium );
    pDoc->ResetModified();
    return bRet;
}


void SwDocShell::SubInitNew()
{
    ASSERT( !mxBasePool.is(), "wer hat seinen Pool nicht zerstoert?" );
    mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    UpdateFontList();
    InitDraw();

    pDoc->setLinkUpdateMode( GLOBALSETTING );
    pDoc->setFieldUpdateFlags( AUTOUPD_GLOBALSETTING );

    sal_Bool bWeb = ISA(SwWebDocShell);

    sal_uInt16 nRange[] =   {
        RES_PARATR_ADJUST, RES_PARATR_ADJUST,
        RES_CHRATR_COLOR, RES_CHRATR_COLOR,
        RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
        0, 0, 0  };
    if(!bWeb)
    {
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 3 ] = RES_PARATR_TABSTOP;
        nRange[ (sizeof(nRange)/sizeof(nRange[0])) - 2 ] = RES_PARATR_HYPHENZONE;
    }
    SfxItemSet aDfltSet( pDoc->GetAttrPool(), nRange );

    //! get lingu options without loading lingu DLL
    SvtLinguOptions aLinguOpt;

    // #107253# Replaced SvtLinguConfig with SwLinguConfig wrapper with UsageCount
    SwLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   nVal = aLinguOpt.nDefaultLanguage,
                eCJK = aLinguOpt.nDefaultLanguage_CJK,
                eCTL = aLinguOpt.nDefaultLanguage_CTL;
    aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );

    if(!bWeb)
    {
        SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) pDoc->GetDefault(
                                                        RES_PARATR_HYPHENZONE) );
        aHyp.GetMinLead()   = static_cast< BYTE >(aLinguOpt.nHyphMinLeading);
        aHyp.GetMinTrail()  = static_cast< BYTE >(aLinguOpt.nHyphMinTrailing);

        aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = static_cast< sal_uInt16 >(SW_MOD()->GetUsrPref(FALSE)->GetDefTab());
        if( nNewPos )
            aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                          SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ) );
    }
    aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );

    pDoc->SetDefault( aDfltSet );

    //default page mode for text grid
    if(!bWeb)
    {
        sal_Bool bSquaredPageMode = SW_MOD()->GetUsrPref(FALSE)->IsSquaredPageMode();
        pDoc->SetDefaultPageMode( bSquaredPageMode );
    }

    pDoc->ResetModified();
}

/*
 * Document Interface Access
 */
IDocumentDeviceAccess* SwDocShell::getIDocumentDeviceAccess() { return pDoc; }
const IDocumentSettingAccess* SwDocShell::getIDocumentSettingAccess() const { return pDoc; }
IDocumentChartDataProviderAccess* SwDocShell::getIDocumentChartDataProviderAccess() { return pDoc; }

