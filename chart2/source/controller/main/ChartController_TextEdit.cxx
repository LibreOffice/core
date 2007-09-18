/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_TextEdit.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:56:36 $
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
#include "precompiled_chart2.hxx"
#include "ChartController.hxx"

#include "DrawViewWrapper.hxx"
#include "ChartWindow.hxx"
#include "TitleHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "ControllerLockGuard.hxx"
#include "AccessibleTextHelper.hxx"

#include <svx/svdotext.hxx>

// header for class SvxCharacterMap
#ifndef _SVX_CHARMAP_HXX
#include <svx/charmap.hxx>
#endif
// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
// header for class SdrOutliner
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef _SVX_DIALOG_HXX
#include <svx/svxdlg.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_EditText()
{
    this->StartTextEdit();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void ChartController::StartTextEdit()
{
    //the first marked object will be edited

    SdrObject* pTextObj = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObj)
        return;

    m_xUndoManager->preAction( m_aModel->getModel());
    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();
    //pOutliner->SetRefDevice(m_pChartWindow);
    //pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pStyleSheetPool);
    //pOutliner->SetDefaultLanguage( eLang );
    //pOutliner->SetHyphenator( xHyphenator );

    //#i77362 change notification for changes on additional shapes are missing
    uno::Reference< beans::XPropertySet > xChartViewProps( m_xChartView, uno::UNO_QUERY );
    if( xChartViewProps.is() )
        xChartViewProps->setPropertyValue( C2U("SdrViewIsInEditMode"), uno::makeAny(sal_True) );

    sal_Bool bEdit = m_pDrawViewWrapper->SdrBeginTextEdit( pTextObj
                    , m_pDrawViewWrapper->GetPageView()
                    , m_pChartWindow
                    , sal_False //bIsNewObj
                    , pOutliner
                    , 0L //pOutlinerView
                    , sal_True //bDontDeleteOutliner
                    , sal_True //bOnlyOneView
                    );
    if(bEdit)
    {
        // set undo manager at topmost shell ( SdDrawTextObjectBar )
        /*
        if( pViewSh )
            pViewSh->GetViewFrame()->GetDispatcher()->GetShell( 0 )->
                SetUndoManager(&pOutliner->GetUndoManager());
        */
        m_pDrawViewWrapper->SetEditMode();

        //we invalidate the outliner region because the outliner has some
        //paint problems (some characters are painted twice a little bit shifted)
        m_pChartWindow->Invalidate( m_pDrawViewWrapper->GetMarkedObjBoundRect() );
    }
}

bool ChartController::EndTextEdit()
{
    m_pDrawViewWrapper->SdrEndTextEdit();

    //#i77362 change notification for changes on additional shapes are missing
    uno::Reference< beans::XPropertySet > xChartViewProps( m_xChartView, uno::UNO_QUERY );
    if( xChartViewProps.is() )
        xChartViewProps->setPropertyValue( C2U("SdrViewIsInEditMode"), uno::makeAny(sal_False) );

    SdrObject* pTextObject = m_pDrawViewWrapper->getTextEditObject();
    if(!pTextObject)
        return false;

    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();
    OutlinerParaObject* pParaObj = pTextObject->GetOutlinerParaObject();
    if( pParaObj && pOutliner )
    {
        pOutliner->SetText( *pParaObj );

        String aString = pOutliner->GetText(
                            pOutliner->GetParagraph( 0 ),
                            pOutliner->GetParagraphCount() );
        uno::Reference< beans::XPropertySet > xPropSet =
            ObjectIdentifier::getObjectPropertySet( m_aSelection.getSelectedCID(), getModel() );

        // lock controllers till end of block
        ControllerLockGuard aCLGuard( m_aModel->getModel());

        //Paragraph* pPara =
        TitleHelper::setCompleteString( aString, uno::Reference<
            ::com::sun::star::chart2::XTitle >::query( xPropSet ), m_xCC );
        try
        {
            m_xUndoManager->postAction( C2U("Edit Text") );
        }
        catch( uno::RuntimeException& e)
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_InsertSpecialCharacter()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex());

    if( m_pDrawViewWrapper && !m_pDrawViewWrapper->IsTextEdit() )
        this->StartTextEdit();

    SvxAbstractDialogFactory * pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT( pFact, "No dialog factory" );
    AbstractSvxCharacterMap * pDlg = pFact->CreateSvxCharacterMap( m_pChartWindow,  RID_SVXDLG_CHARMAP, FALSE );
    DBG_ASSERT( pDlg, "Couldn't create SvxCharacterMap dialog" );

    //set fixed current font
    pDlg->SetFont( m_pDrawViewWrapper->getOutliner()->GetRefDevice()->GetFont() );
    pDlg->DisableFontSelection(); //maybe not necessary in future

    if( pDlg->Execute() == RET_OK )
    {
        String aString( pDlg->GetCharacters() );

        OutlinerView* pOutlinerView = m_pDrawViewWrapper->GetTextEditOutlinerView();
        SdrOutliner*  pOutliner = m_pDrawViewWrapper->getOutliner();

        if(!pOutliner || !pOutlinerView)
            return;

        // insert string to outliner

        // prevent flicker
        pOutlinerView->HideCursor();
        pOutliner->SetUpdateMode(FALSE);

        // delete current selection by inserting empty String, so current
        // attributes become unique (sel. has to be erased anyway)
        pOutlinerView->InsertText(String());

        //SfxUndoManager& rUndoMgr =  pOutliner->GetUndoManager();
        //rUndoMgr.EnterListAction( String( SchResId( STR_UNDO_INSERT_SPECCHAR )), String( SchResId( STR_UNDO_INSERT_SPECCHAR )));
        pOutlinerView->InsertText(aString, TRUE);

        ESelection aSel = pOutlinerView->GetSelection();
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOutlinerView->SetSelection(aSel);

        //rUndoMgr.LeaveListAction();

        // show changes
        pOutliner->SetUpdateMode(TRUE);
        pOutlinerView->ShowCursor();
    }

    delete pDlg;
}

uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
    ChartController::impl_createAccessibleTextContext()
{
    uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > xResult(
        new AccessibleTextHelper( m_pDrawViewWrapper ));

    return xResult;
}


//.............................................................................
} //namespace chart
//.............................................................................
