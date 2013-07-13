/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <hintids.hxx>

#include <svx/dialogs.hrc>
#include <i18nlangtag/mslangid.hxx>
#include <sot/storinfo.hxx>
#include <sot/storage.hxx>
#include <svl/zforlist.hxx>
#include <svtools/ctrltool.hxx>
#include <unotools/lingucfg.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/autokernitem.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <rtl/logfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/svxacorr.hxx>
#include <vcl/svapp.hxx>
#include <view.hxx>
#include <prtopt.hxx>
#include <fmtcol.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <docfac.hxx>
#include <docstyle.hxx>
#include <shellio.hxx>
#include <tox.hxx>
#include <swdtflvr.hxx>
#include <dbmgr.hxx>
#include <usrpref.hxx>
#include <fontcfg.hxx>
#include <poolfmt.hxx>
#include <modcfg.hxx>
#include <globdoc.hxx>
#include <ndole.hxx>
#include <mdiexp.hxx>
#include <unotxdoc.hxx>
#include <linkenum.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <swerror.h>
#include <globals.hrc>
#include <unochart.hxx>

// text grid
#include <tgrditem.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

// Load Document
sal_Bool SwDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::InitNew" );

    sal_Bool bRet = SfxObjectShell::InitNew( xStor );
    OSL_ENSURE( GetMapUnit() == MAP_TWIP, "map unit is not twip!" );
    sal_Bool bHTMLTemplSet = sal_False;
    if( bRet )
    {
        AddLink();      // create pDoc / pIo if applicable

        sal_Bool bWeb = ISA( SwWebDocShell );
        if ( bWeb )
            bHTMLTemplSet = SetHTMLTemplate( *GetDoc() );// Styles from HTML.vor
        else if( ISA( SwGlobalDocShell ) )
            GetDoc()->set(IDocumentSettingAccess::GLOBAL_DOCUMENT, true);       // Globaldokument


        if ( GetCreateMode() ==  SFX_CREATE_MODE_EMBEDDED )
            SwTransferable::InitOle( this, *pDoc );

        // set forbidden characters if necessary
        SvxAsianConfig aAsian;
        Sequence<lang::Locale> aLocales =  aAsian.GetStartEndCharLocales();
        if(aLocales.getLength())
        {
            const lang::Locale* pLocales = aLocales.getConstArray();
            for(sal_Int32 i = 0; i < aLocales.getLength(); i++)
            {
                ForbiddenCharacters aForbidden;
                aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine);
                LanguageType  eLang = LanguageTag::convertToLanguageType(pLocales[i]);
                pDoc->setForbiddenCharacters( eLang, aForbidden);
            }
        }
        pDoc->set(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION,
                  !aAsian.IsKerningWesternTextOnly());
        pDoc->setCharacterCompressionType(static_cast<SwCharCompressType>(aAsian.GetCharDistanceCompression()));
        pDoc->setPrintData(*SW_MOD()->GetPrtOptions(bWeb));

        SubInitNew();

        // for all

        SwStdFontConfig* pStdFont = SW_MOD()->GetStdFontConfig();
        SfxPrinter* pPrt = pDoc->getPrinter( false );

        String sEntry;
        sal_uInt16 aFontWhich[] =
        {   RES_CHRATR_FONT,
            RES_CHRATR_CJK_FONT,
            RES_CHRATR_CTL_FONT
        };
        sal_uInt16 aFontHeightWhich[] =
        {
            RES_CHRATR_FONTSIZE,
            RES_CHRATR_CJK_FONTSIZE,
            RES_CHRATR_CTL_FONTSIZE
        };
        sal_uInt16 aFontIds[] =
        {
            FONT_STANDARD,
            FONT_STANDARD_CJK,
            FONT_STANDARD_CTL
        };
        sal_uInt16 nFontTypes[] =
        {
            DEFAULTFONT_LATIN_TEXT,
            DEFAULTFONT_CJK_TEXT,
            DEFAULTFONT_CTL_TEXT
        };
        sal_uInt16 aLangTypes[] =
        {
            RES_CHRATR_LANGUAGE,
            RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CTL_LANGUAGE
        };

        for(sal_uInt8 i = 0; i < 3; i++)
        {
            sal_uInt16 nFontWhich = aFontWhich[i];
            sal_uInt16 nFontId = aFontIds[i];
            SvxFontItem* pFontItem = 0;
            const SvxLanguageItem& rLang = (const SvxLanguageItem&)pDoc->GetDefault( aLangTypes[i] );
            LanguageType eLanguage = rLang.GetLanguage();
            if(!pStdFont->IsFontDefault(nFontId))
            {
                sEntry = pStdFont->GetFontFor(nFontId);

                Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                {
                    aFont = pPrt->GetFontMetric( aFont );
                }

                pFontItem = new SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                            aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich);
            }
            else
            {
                // #107782# OJ use korean language if latin was used
                if ( i == 0 )
                {
                        LanguageType eUiLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
                    if (MsLangId::isKorean(eUiLanguage))
                        eLanguage = eUiLanguage;
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
                pColl->ResetFmtAttr(nFontWhich);
            }
            delete pFontItem;
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( FONT_STANDARD, i, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( nFontId, eLanguage );
            pDoc->SetDefault(SvxFontHeightItem( nFontHeight, 100, aFontHeightWhich[i] ));
            if( !bHTMLTemplSet )
            {
                SwTxtFmtColl *pColl = pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
                pColl->ResetFmtAttr(aFontHeightWhich[i]);
            }

        }
        sal_uInt16 aFontIdPoolId[] =
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

        sal_uInt16 nFontWhich = RES_CHRATR_FONT;
        sal_uInt16 nFontHeightWhich = RES_CHRATR_FONTSIZE;
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

                Font aFont( sEntry, Size( 0, 10 ) );
                if( pPrt )
                    aFont = pPrt->GetFontMetric( aFont );

                pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
                if( !bHTMLTemplSet ||
                    SFX_ITEM_SET != pColl->GetAttrSet().GetItemState(
                                                    nFontWhich, sal_False ) )
                {
                    pColl->SetFmtAttr(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                                  aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
                }
            }
            sal_Int32 nFontHeight = pStdFont->GetFontHeight( static_cast< sal_Int8 >(aFontIdPoolId[nIdx]), 0, eLanguage );
            if(nFontHeight <= 0)
                nFontHeight = pStdFont->GetDefaultHeightFor( aFontIdPoolId[nIdx], eLanguage );
            if(!pColl)
                pColl = pDoc->GetTxtCollFromPool(aFontIdPoolId[nIdx + 1]);
            SvxFontHeightItem aFontHeight( (const SvxFontHeightItem&)pColl->GetFmtAttr( nFontHeightWhich, sal_True ));
            if(aFontHeight.GetHeight() != sal::static_int_cast<sal_uInt32, sal_Int32>(nFontHeight))
            {
                aFontHeight.SetHeight(nFontHeight);
                pColl->SetFmtAttr( aFontHeight );
            }
        }

        // the default for documents created via 'File/New' should be 'on'
        // (old documents, where this property was not yet implemented, will get the
        // value 'false' in the SwDoc c-tor)
        pDoc->set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT,
                SW_MOD()->GetUsrPref( bWeb )->IsAlignMathObjectsToBaseline() );
    }

    /* #106748# If the default frame direction of a document is RTL
        the default adjusment is to the right. */
    if( !bHTMLTemplSet &&
        FRMDIR_HORI_RIGHT_TOP == GetDefaultFrameDirection(GetAppLanguage()) )
        pDoc->SetDefault( SvxAdjustItem(SVX_ADJUST_RIGHT, RES_PARATR_ADJUST ) );

// #i29550#
    pDoc->SetDefault( SfxBoolItem( RES_COLLAPSING_BORDERS, sal_True ) );
// <-- collapsing

    //#i16874# AutoKerning as default for new documents
    pDoc->SetDefault( SvxAutoKernItem( sal_True, RES_CHRATR_AUTOKERN ) );

    // #i42080# - Due to the several calls of method <SetDefault(..)>
    // at the document instance, the document is modified. Thus, reset this
    // status here. Note: In method <SubInitNew()> this is also done.
    pDoc->ResetModified();

    return bRet;
}

// Ctor with SfxCreateMode ?????
SwDocShell::SwDocShell( SfxObjectCreateMode eMode ) :
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
    Init_Impl();
}

// Ctor / Dtor
SwDocShell::SwDocShell( const sal_uInt64 i_nSfxCreationFlags ) :
    SfxObjectShell ( i_nSfxCreationFlags ),
    pDoc(0),
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

// Ctor / Dtor
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

// Dtor
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

    // we, as BroadCaster also become our own Listener
    // (for DocInfo/FileNames/....)
    EndListening( *this );

    delete pOLEChildList;
}

void  SwDocShell::Init_Impl()
{
    SetPool(&SW_MOD()->GetPool());
    SetBaseModel(new SwXTextDocument(this));
    // we, as BroadCaster also become our own Listener
    // (for DocInfo/FileNames/....)
    StartListening( *this );
    //position of the "Automatic" style filter for the stylist (app.src)
    SetAutoStyleFilterIndex(3);

    // set map unit to twip
    SetMapUnit( MAP_TWIP );
}

// AddLink
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
    pDoc->SetDocShell( this );      // set the DocShell-Pointer for Doc
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);

    SetPool(&pDoc->GetAttrPool());

    // most suitably not until a sdbcx::View is created!!!
    pDoc->SetOle2Link(LINK(this, SwDocShell, Ole2ModifiedHdl));
}

// create new FontList Change Printer
void SwDocShell::UpdateFontList()
{
    if(!bInUpdateFontList)
    {
        bInUpdateFontList = true;
        OSL_ENSURE(pDoc, "No Doc no FontList");
        if( pDoc )
        {
            delete pFontList;
            pFontList = new FontList( pDoc->getReferenceDevice( true ) );
            PutItem( SvxFontListItem( pFontList, SID_ATTR_CHAR_FONTLIST ) );
        }
        bInUpdateFontList = false;
    }
}

// RemoveLink
void SwDocShell::RemoveLink()
{
    // disconnect Uno-Object
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
        pDoc = 0;       // we don't have the Doc anymore!!
    }
}
void SwDocShell::InvalidateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Invalidate();
}
void SwDocShell::ReactivateModel()
{
    // disconnect Uno-Object
    uno::Reference< text::XTextDocument >  xDoc(GetBaseModel(), uno::UNO_QUERY);
    ((SwXTextDocument*)xDoc.get())->Reactivate(this);
}

// Load, Default-Format
sal_Bool  SwDocShell::Load( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::Load" );
    sal_Bool bRet = sal_False;
    if( SfxObjectShell::Load( rMedium ))
    {
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "after SfxInPlaceObject::Load" );
        if( pDoc )              // for last version!!
            RemoveLink();       // release the existing

        AddLink();      // set Link and update Data!!

        // Loading
        // for MD
        OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
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
            case SFX_CREATE_MODE_ORGANIZER:
                {
                    if( ReadXML )
                    {
                        ReadXML->SetOrganizerMode( sal_True );
                        SwReader aRdr( rMedium, aEmptyStr, pDoc );
                        nErr = aRdr.Read( *ReadXML );
                        ReadXML->SetOrganizerMode( sal_False );
                    }
                }
                break;

            case SFX_CREATE_MODE_INTERNAL:
            case SFX_CREATE_MODE_EMBEDDED:
                {
                    // for MWERKS (Mac-Compiler): can't cast autonomously
                    SwTransferable::InitOle( this, *pDoc );
                }
                // suppress SfxProgress, when we are Embedded
                SW_MOD()->SetEmbeddedLoadSave( sal_True );
                // no break;

            case SFX_CREATE_MODE_STANDARD:
            case SFX_CREATE_MODE_PREVIEW:
                {
                    Reader *pReader = ReadXML;
                    if( pReader )
                    {
                        // set Doc's DocInfo at DocShell-Medium
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
                }
                break;

            default:
                OSL_ENSURE( !this, "Load: new CreateMode?" );
        }

        UpdateFontList();
        InitDraw();

        SetError( nErr, OUString( OSL_LOG_PREFIX ) );
        bRet = !IsError( nErr );

        if ( bRet && !pDoc->IsInLoadAsynchron() &&
             GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        {
            LoadingFinished();
        }

        // suppress SfxProgress, when we are Embedded
        SW_MOD()->SetEmbeddedLoadSave( sal_False );
    }

    return bRet;
}

sal_Bool  SwDocShell::LoadFrom( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDocShell::LoadFrom" );
    sal_Bool bRet = sal_False;
    if( pDoc )
        RemoveLink();

    AddLink();      // set Link and update Data!!

    do {        // middle check loop
        sal_uInt32 nErr = ERR_SWG_READ_ERROR;
        String aStreamName;
        aStreamName = OUString("styles.xml");
        uno::Reference < container::XNameAccess > xAccess( rMedium.GetStorage(), uno::UNO_QUERY );
        if ( xAccess->hasByName( aStreamName ) && rMedium.GetStorage()->isStreamElement( aStreamName ) )
        {
            // Loading
            SwWait aWait( *this, sal_True );
            {
                OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
                mxBasePool = new SwDocStyleSheetPool( *pDoc, SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
                if( ReadXML )
                {
                    ReadXML->SetOrganizerMode( sal_True );
                    SwReader aRdr( rMedium, aEmptyStr, pDoc );
                    nErr = aRdr.Read( *ReadXML );
                    ReadXML->SetOrganizerMode( sal_False );
                }
            }
        }
        else
        {
            OSL_FAIL("Code removed!");
        }

        SetError( nErr, OUString( OSL_LOG_PREFIX ) );
        bRet = !IsError( nErr );

    } while( false );

    SfxObjectShell::LoadFrom( rMedium );
    pDoc->ResetModified();
    return bRet;
}


void SwDocShell::SubInitNew()
{
    OSL_ENSURE( !mxBasePool.is(), "who hasn't destroyed their Pool?" );
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

    SvtLinguConfig().GetOptions( aLinguOpt );

    sal_Int16   nVal = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage, ::com::sun::star::i18n::ScriptType::LATIN),
                eCJK = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CJK, ::com::sun::star::i18n::ScriptType::ASIAN),
                eCTL = MsLangId::resolveSystemLanguageByScriptType(aLinguOpt.nDefaultLanguage_CTL, ::com::sun::star::i18n::ScriptType::COMPLEX);
    aDfltSet.Put( SvxLanguageItem( nVal, RES_CHRATR_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCJK, RES_CHRATR_CJK_LANGUAGE ) );
    aDfltSet.Put( SvxLanguageItem( eCTL, RES_CHRATR_CTL_LANGUAGE ) );

    if(!bWeb)
    {
        SvxHyphenZoneItem aHyp( (SvxHyphenZoneItem&) pDoc->GetDefault(
                                                        RES_PARATR_HYPHENZONE) );
        aHyp.GetMinLead()   = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinLeading);
        aHyp.GetMinTrail()  = static_cast< sal_uInt8 >(aLinguOpt.nHyphMinTrailing);

        aDfltSet.Put( aHyp );

        sal_uInt16 nNewPos = static_cast< sal_uInt16 >(SW_MOD()->GetUsrPref(sal_False)->GetDefTab());
        if( nNewPos )
            aDfltSet.Put( SvxTabStopItem( 1, nNewPos,
                                          SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP ) );
    }
    aDfltSet.Put( SvxColorItem( Color( COL_AUTO ), RES_CHRATR_COLOR ) );

    pDoc->SetDefault( aDfltSet );

    //default page mode for text grid
    if(!bWeb)
    {
        sal_Bool bSquaredPageMode = SW_MOD()->GetUsrPref(sal_False)->IsSquaredPageMode();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
