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

#include "scitems.hxx"
#include <svx/drawitem.hxx>
#include <svl/asiancfg.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <editeng/unolingu.hxx>
#include <rtl/logfile.hxx>

#include "drwlayer.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "docshimp.hxx"
#include "docfunc.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------

sal_Bool ScDocShell::InitNew( const uno::Reference < embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::InitNew" );

    sal_Bool bRet = SfxObjectShell::InitNew( xStor );

    aDocument.MakeTable(0);
    //  zusaetzliche Tabellen werden von der ersten View angelegt,
    //  wenn bIsEmpty dann noch sal_True ist

    if( bRet )
    {
        Size aSize( (long) ( STD_COL_WIDTH           * HMM_PER_TWIPS * OLE_STD_CELLS_X ),
                    (long) ( ScGlobal::nStdRowHeight * HMM_PER_TWIPS * OLE_STD_CELLS_Y ) );
        // hier muss auch der Start angepasst werden
        SetVisAreaOrSize( Rectangle( Point(), aSize ), sal_True );
    }

    // InitOptions sets the document languages, must be called before CreateStandardStyles
    InitOptions(false);

    aDocument.GetStyleSheetPool()->CreateStandardStyles();
    aDocument.UpdStlShtPtrsFrmNms();

    //  SetDocumentModified ist in Load/InitNew nicht mehr erlaubt!

    InitItems();
    CalcOutputFactor();

    return bRet;
}

//------------------------------------------------------------------

bool ScDocShell::IsEmpty() const
{
    return bIsEmpty;
}


void ScDocShell::SetEmpty(bool bSet)
{
    bIsEmpty = bSet;
}

//------------------------------------------------------------------

void ScDocShell::InitItems()
{
    // AllItemSet fuer Controller mit benoetigten Items fuellen:

    //  Druck-Optionen werden beim Drucken und evtl. in GetPrinter gesetzt

    UpdateFontList();

    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (pDrawLayer)
    {
        PutItem( SvxColorListItem  ( pDrawLayer->GetColorList(), SID_COLOR_TABLE ) );
        PutItem( SvxGradientListItem( pDrawLayer->GetGradientList(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem   ( pDrawLayer->GetHatchList(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem  ( pDrawLayer->GetBitmapList(), SID_BITMAP_LIST ) );
        PutItem( SvxDashListItem    ( pDrawLayer->GetDashList(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem ( pDrawLayer->GetLineEndList(), SID_LINEEND_LIST ) );

            //  andere Anpassungen nach dem Anlegen des DrawLayers

        pDrawLayer->SetNotifyUndoActionHdl( LINK( pDocFunc, ScDocFunc, NotifyDrawUndo ) );
    }
    else
    {
        //  always use global color table instead of local copy
        PutItem( SvxColorListItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ) );
    }

    if ( !aDocument.GetForbiddenCharacters().is() ||
            !aDocument.IsValidAsianCompression() || !aDocument.IsValidAsianKerning() )
    {
        //  get settings from SvxAsianConfig
        SvxAsianConfig aAsian;

        if ( !aDocument.GetForbiddenCharacters().is() )
        {
            // set forbidden characters if necessary
            uno::Sequence<lang::Locale> aLocales = aAsian.GetStartEndCharLocales();
            if (aLocales.getLength())
            {
                rtl::Reference<SvxForbiddenCharactersTable> xForbiddenTable =
                        new SvxForbiddenCharactersTable( comphelper::getProcessComponentContext() );

                const lang::Locale* pLocales = aLocales.getConstArray();
                for (sal_Int32 i = 0; i < aLocales.getLength(); i++)
                {
                    i18n::ForbiddenCharacters aForbidden;
                    aAsian.GetStartEndChars( pLocales[i], aForbidden.beginLine, aForbidden.endLine );
                    LanguageType eLang = LanguageTag::convertToLanguageType(pLocales[i]);

                    xForbiddenTable->SetForbiddenCharacters( eLang, aForbidden );
                }

                aDocument.SetForbiddenCharacters( xForbiddenTable );
            }
        }

        if ( !aDocument.IsValidAsianCompression() )
        {
            // set compression mode from configuration if not already set (e.g. XML import)
            aDocument.SetAsianCompression( sal::static_int_cast<sal_uInt8>( aAsian.GetCharDistanceCompression() ) );
        }

        if ( !aDocument.IsValidAsianKerning() )
        {
            // set asian punctuation kerning from configuration if not already set (e.g. XML import)
            aDocument.SetAsianKerning( !aAsian.IsKerningWesternTextOnly() );    // reversed
        }
    }
}

//------------------------------------------------------------------

void ScDocShell::ResetDrawObjectShell()
{
    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (pDrawLayer)
        pDrawLayer->SetObjectShell( NULL );
}

//------------------------------------------------------------------

void ScDocShell::Activate()
{
}


void ScDocShell::Deactivate()
{
}

//------------------------------------------------------------------


ScDrawLayer* ScDocShell::MakeDrawLayer()
{
    ScDrawLayer* pDrawLayer = aDocument.GetDrawLayer();
    if (!pDrawLayer)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "nn93723", "ScDocShell::MakeDrawLayer" );

        aDocument.InitDrawLayer(this);
        pDrawLayer = aDocument.GetDrawLayer();
        InitItems();                                            // incl. Undo und Basic
        Broadcast( SfxSimpleHint( SC_HINT_DRWLAYER_NEW ) );
        if (nDocumentLock)
            pDrawLayer->setLock(true);
    }
    return pDrawLayer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
