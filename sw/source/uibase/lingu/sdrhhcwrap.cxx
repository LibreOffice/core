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
#include <svx/svditer.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/langitem.hxx>
#include <linguistic/lngprops.hxx>
#include <sfx2/sfxuno.hxx>
#include <svx/svdview.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <drawdoc.hxx>
#include "sdrhhcwrap.hxx"
#include <frmfmt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <dcontact.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docary.hxx>
#include <edtwin.hxx>

using namespace ::com::sun::star;

SdrHHCWrapper::SdrHHCWrapper( SwView* pVw,
       LanguageType nSourceLanguage, LanguageType nTargetLanguage,
       const vcl::Font* pTargetFnt,
       sal_Int32 nConvOptions,
       bool bInteractive ) :
    SdrOutliner(pVw->GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->
                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                OutlinerMode::TextObject ),
    pView( pVw ),
    pTextObj( nullptr ),
    pOutlView( nullptr ),
    nOptions( nConvOptions ),
    nDocIndex( 0 ),
    nSourceLang( nSourceLanguage ),
    nTargetLang( nTargetLanguage ),
    pTargetFont( pTargetFnt ),
    bIsInteractive( bInteractive )
{
    SetRefDevice( pView->GetDocShell()->GetDoc()->getIDocumentDeviceAccess().getPrinter( false ) );

    MapMode aMapMode (MapUnit::MapTwip);
    SetRefMapMode(aMapMode);

     Size aSize( 1, 1 );
    SetPaperSize( aSize );

    pOutlView.reset( new OutlinerView( this, &(pView->GetEditWin()) ) );
    pOutlView->GetOutliner()->SetRefDevice(pView->GetWrtShell().getIDocumentDeviceAccess().getPrinter( false ));

    // Hack: all SdrTextObj attributes should be transferred to EditEngine
    pOutlView->SetBackgroundColor( COL_WHITE );

    InsertView( pOutlView.get() );
    Point aPoint( 0, 0 );
     tools::Rectangle aRect( aPoint, aSize );
    pOutlView->SetOutputArea( aRect );
//  SetText( NULL );
    ClearModifyFlag();
}

SdrHHCWrapper::~SdrHHCWrapper()
{
    if (pTextObj)
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( true );
        SetUpdateMode(false);
        pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1, 1) ) );
    }
    RemoveView( pOutlView.get() );
    pOutlView.reset();
}

void SdrHHCWrapper::StartTextConversion()
{
    pOutlView->StartTextConversion( nSourceLang, nTargetLang, pTargetFont, nOptions, bIsInteractive, true );
}

bool SdrHHCWrapper::ConvertNextDocument()
{
    bool bNextDoc = false;

    if ( pTextObj )
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( true );
        SetUpdateMode(false);
        pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1, 1) ) );
        SetPaperSize( Size(1, 1) );
        Clear();
        pTextObj = nullptr;
    }

    const auto n = nDocIndex;

    std::list<SdrTextObj*> aTextObjs;
    SwDrawContact::GetTextObjectsFromFormat( aTextObjs, pView->GetDocShell()->GetDoc() );
    for (auto const& textObj : aTextObjs)
    {
        pTextObj = textObj;
        if (textObj)
        {
            OutlinerParaObject* pParaObj = textObj->GetOutlinerParaObject();
            if ( pParaObj )
            {
                SetPaperSize( textObj->GetLogicRect().GetSize() );
                SetText( *pParaObj );

                ClearModifyFlag();

                //!! update mode needs to be set to true otherwise
                //!! the call to 'HasConvertibleTextPortion' will not always
                //!! work correctly because the document may not be properly
                //!! formatted when some information is accessed, and thus
                //!! incorrect results get returned.
                SetUpdateMode(true);
                if (HasConvertibleTextPortion( nSourceLang ))
                {
                    SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
                    OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
                    SdrPageView* pPV = pSdrView->GetSdrPageView();
                    nDocIndex = n;
                    bNextDoc = true;
                    pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1,1)));
                    SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    SetUpdateMode(true);
                    pView->GetWrtShell().MakeVisible(pTextObj->GetLogicRect());

                    pSdrView->SdrBeginTextEdit(pTextObj, pPV, &pView->GetEditWin(), false, this, pOutlView.get(), true, true);
                }
                else
                    SetUpdateMode(false);
            }

            if ( !bNextDoc )
                pTextObj = nullptr;
            else
                break;
        }
    }

    ClearModifyFlag();

    return bNextDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
