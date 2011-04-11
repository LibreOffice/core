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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <hintids.hxx>
#include <svx/svditer.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <sfx2/printer.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/langitem.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <linguistic/lngprops.hxx>
#include <sfx2/sfxuno.hxx>
#include <svx/svdview.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <sdrhhcwrap.hxx>
#include <frmfmt.hxx>
#include <docsh.hxx>        //     "     "     "
#include <wrtsh.hxx>        //  MakeVisible
#include <view.hxx>
#include <dcontact.hxx>     // Spelling von DrawObj
#include <doc.hxx>        //     "     "     "
#include <docary.hxx>
#include <edtwin.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

//////////////////////////////////////////////////////////////////////

SdrHHCWrapper::SdrHHCWrapper( SwView* pVw,
       LanguageType nSourceLanguage, LanguageType nTargetLanguage,
       const Font* pTargetFnt,
       sal_Int32 nConvOptions,
       sal_Bool bInteractive ) :
    SdrOutliner(pVw->GetDocShell()->GetDoc()->GetDrawModel()->
                             GetDrawOutliner().GetEmptyItemSet().GetPool(),
                OUTLINERMODE_TEXTOBJECT ),
    pView( pVw ),
    pTextObj( NULL ),
    pOutlView( NULL ),
    nOptions( nConvOptions ),
    nDocIndex( 0 ),
    nSourceLang( nSourceLanguage ),
    nTargetLang( nTargetLanguage ),
    pTargetFont( pTargetFnt ),
    bIsInteractive( bInteractive )
{
    SetRefDevice( pView->GetDocShell()->GetDoc()->getPrinter( false ) );

    MapMode aMapMode (MAP_TWIP);
    SetRefMapMode(aMapMode);

     Size aSize( 1, 1 );
    SetPaperSize( aSize );

    pOutlView = new OutlinerView( this, &(pView->GetEditWin()) );
    pOutlView->GetOutliner()->SetRefDevice(pView->GetWrtShell().getIDocumentDeviceAccess()->getPrinter( false ));

    // Hack: Es sollten alle SdrTextObj-Attribute an die EditEngine
    //       uebertragen werden.
    pOutlView->SetBackgroundColor( Color( COL_WHITE ) );


    InsertView( pOutlView );
    Point aPoint( 0, 0 );
     Rectangle aRect( aPoint, aSize );
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
        pSdrView->SdrEndTextEdit( sal_True );
        SetUpdateMode(sal_False);
        pOutlView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
    }
    RemoveView( pOutlView );
    delete pOutlView;
}


void SdrHHCWrapper::StartTextConversion()
{
    pOutlView->StartTextConversion( nSourceLang, nTargetLang, pTargetFont, nOptions, bIsInteractive, sal_True );
}


sal_Bool SdrHHCWrapper::ConvertNextDocument()
{
    sal_Bool bNextDoc = sal_False;

    if ( pTextObj )
    {
        SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
        OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( sal_True );
        SetUpdateMode(sal_False);
        pOutlView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
        SetPaperSize( Size(1, 1) );
        Clear();
        pTextObj = NULL;
    }

    sal_uInt16 n = nDocIndex;

    std::list<SdrTextObj*> aTextObjs;
    SwDrawContact::GetTextObjectsFromFmt( aTextObjs, pView->GetDocShell()->GetDoc() );
    for ( std::list<SdrTextObj*>::iterator aIt = aTextObjs.begin(); aIt != aTextObjs.end(); aIt++ )
    {
        pTextObj = (*aIt);
        if ( pTextObj )
        {
            OutlinerParaObject* pParaObj = pTextObj->GetOutlinerParaObject();
            if ( pParaObj )
            {
                SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                SetText( *pParaObj );

                ClearModifyFlag();

                //!! update mode needs to be set to true otherwise
                //!! the call to 'HasConvertibleTextPortion' will not always
                //!! work correctly because the document may not be properly
                //!! formatted when some information is accessed, and thus
                //!! incorrect results get returned.
                SetUpdateMode(sal_True);
                if (HasConvertibleTextPortion( nSourceLang ))
                {
                    SdrView *pSdrView = pView->GetWrtShell().GetDrawView();
                    OSL_ENSURE( pSdrView, "SdrHHCWrapper without DrawView?" );
                    SdrPageView* pPV = pSdrView->GetSdrPageView();
                    nDocIndex = n;
                    bNextDoc = sal_True;
                    pOutlView->SetOutputArea( Rectangle( Point(), Size(1,1)));
                    SetPaperSize( pTextObj->GetLogicRect().GetSize() );
                    SetUpdateMode(sal_True);
                    pView->GetWrtShell().MakeVisible(pTextObj->GetLogicRect());

                    pSdrView->SdrBeginTextEdit(pTextObj, pPV, &pView->GetEditWin(), sal_False, this, pOutlView, sal_True, sal_True);
                }
                else
                    SetUpdateMode(sal_False);
            }

            if ( !bNextDoc )
                pTextObj = NULL;
            else
                break;
        }
    }

    ClearModifyFlag();

    return bNextDoc;
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
