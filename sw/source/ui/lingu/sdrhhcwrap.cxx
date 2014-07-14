/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
#include <svx/svdlegacy.hxx>
#include <svx/fmmodel.hxx>
#include <drawdoc.hxx>

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
        ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
        pSdrView->SdrEndTextEdit( sal_True );
        SetUpdateMode(sal_False);
        pOutlView->SetOutputArea( Rectangle( Point(), Size(1, 1) ) );
//      SetPaperSize( Size(1, 1) );
//      SetText(NULL);
//      pTextObj = NULL;
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
        ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
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
                SetPaperSize( sdr::legacy::GetLogicRect(*pTextObj).GetSize() );
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
                    ASSERT( pSdrView, "SdrHHCWrapper without DrawView?" );
                    nDocIndex = n;
                    bNextDoc = sal_True;
                    pOutlView->SetOutputArea( Rectangle( Point(), Size(1,1)));
                    SetPaperSize( sdr::legacy::GetLogicRect(*pTextObj).GetSize() );
                    SetUpdateMode(sal_True);
                    pView->GetWrtShell().MakeVisible(sdr::legacy::GetLogicRange(*pTextObj));

                    pSdrView->SdrBeginTextEdit(pTextObj, &pView->GetEditWin(), sal_False, this, pOutlView, sal_True, sal_True);
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

