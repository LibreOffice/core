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

#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdview.hxx>
#include <osl/diagnose.h>
#include <drawdoc.hxx>
#include "sdrhhcwrap.hxx"
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <dcontact.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
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
    m_pView( pVw ),
    m_pTextObj( nullptr ),
    m_nOptions( nConvOptions ),
    m_nDocIndex( 0 ),
    m_nSourceLang( nSourceLanguage ),
    m_nTargetLang( nTargetLanguage ),
    m_pTargetFont( pTargetFnt ),
    m_bIsInteractive( bInteractive )
{
    SetRefDevice( m_pView->GetDocShell()->GetDoc()->getIDocumentDeviceAccess().getPrinter( false ) );

    MapMode aMapMode (MapUnit::MapTwip);
    SetRefMapMode(aMapMode);

    Size aSize( 1, 1 );
    SetPaperSize( aSize );

    m_pOutlView.reset( new OutlinerView( this, &(m_pView->GetEditWin()) ) );
    m_pOutlView->GetOutliner()->SetRefDevice(m_pView->GetWrtShell().getIDocumentDeviceAccess().getPrinter( false ));

    // Hack: all SdrTextObj attributes should be transferred to EditEngine
    m_pOutlView->SetBackgroundColor( COL_WHITE );

    InsertView( m_pOutlView.get() );
    Point aPoint( 0, 0 );
    tools::Rectangle aRect( aPoint, aSize );
    m_pOutlView->SetOutputArea( aRect );
//  SetText( NULL );
    ClearModifyFlag();
}

SdrHHCWrapper::~SdrHHCWrapper()
{
    if (m_pTextObj)
    {
        SdrView *pSdrView = m_pView->GetWrtShell().GetDrawView();
        OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( true );
        SetUpdateLayout(false);
        m_pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1, 1) ) );
    }
    RemoveView( m_pOutlView.get() );
    m_pOutlView.reset();
}

void SdrHHCWrapper::StartTextConversion()
{
    m_pOutlView->StartTextConversion(m_pView->GetFrameWeld(), m_nSourceLang, m_nTargetLang, m_pTargetFont, m_nOptions, m_bIsInteractive, true);
}

bool SdrHHCWrapper::ConvertNextDocument()
{
    bool bNextDoc = false;

    if ( m_pTextObj )
    {
        SdrView *pSdrView = m_pView->GetWrtShell().GetDrawView();
        OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( true );
        SetUpdateLayout(false);
        m_pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1, 1) ) );
        SetPaperSize( Size(1, 1) );
        Clear();
        m_pTextObj = nullptr;
    }

    const auto n = m_nDocIndex;

    std::list<SdrTextObj*> aTextObjs;
    SwDrawContact::GetTextObjectsFromFormat(aTextObjs, *m_pView->GetDocShell()->GetDoc());
    for (auto const& textObj : aTextObjs)
    {
        m_pTextObj = textObj;
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
                SetUpdateLayout(true);
                if (HasConvertibleTextPortion( m_nSourceLang ))
                {
                    SdrView *pSdrView = m_pView->GetWrtShell().GetDrawView();
                    assert(pSdrView && "SdrHHCWrapper without DrawView?");
                    SdrPageView* pPV = pSdrView->GetSdrPageView();
                    m_nDocIndex = n;
                    bNextDoc = true;
                    m_pOutlView->SetOutputArea( tools::Rectangle( Point(), Size(1,1)));
                    SetPaperSize( m_pTextObj->GetLogicRect().GetSize() );
                    SetUpdateLayout(true);
                    m_pView->GetWrtShell().MakeVisible(SwRect(m_pTextObj->GetLogicRect()));

                    pSdrView->SdrBeginTextEdit(m_pTextObj, pPV, &m_pView->GetEditWin(), false, this, m_pOutlView.get(), true, true);
                }
                else
                    SetUpdateLayout(false);
            }

            if ( !bNextDoc )
                m_pTextObj = nullptr;
            else
                break;
        }
    }

    ClearModifyFlag();

    return bNextDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
