#include "ChartController.hxx"

#include "DrawViewWrapper.hxx"
#include "ChartWindow.hxx"
#include "TitleHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"

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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::drafts::com::sun::star::chart2;

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

    SdrOutliner* pOutliner = m_pDrawViewWrapper->getOutliner();
    //pOutliner->SetRefDevice(m_pChartWindow);
    //pOutliner->SetStyleSheetPool((SfxStyleSheetPool*)pStyleSheetPool);
    //pOutliner->SetDefaultLanguage( eLang );
    //pOutliner->SetHyphenator( xHyphenator );
    BOOL bEdit = m_pDrawViewWrapper->BegTextEdit( pTextObj
                    , m_pDrawViewWrapper->GetPageView()
                    , m_pChartWindow
                    , FALSE //bIsNewObj
                    , pOutliner
                    , NULL //pOutlinerView
                    , TRUE //bDontDeleteOutliner
                    , TRUE //bOnlyOneView
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
    m_pDrawViewWrapper->EndTextEdit();
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
            ObjectIdentifier::getObjectPropertySet( m_aSelectedObjectCID, getModel() );

        //Paragraph* pPara =
        TitleHelper::setCompleteString( aString, uno::Reference<
            ::drafts::com::sun::star::chart2::XTitle >::query( xPropSet ), m_xCC );
    }
    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void SAL_CALL ChartController::executeDispatch_InsertSpecialCharacter()
{
    SvxCharacterMap aDlg( NULL, FALSE );
    //set fixed current font
    aDlg.SetFont( m_pDrawViewWrapper->getOutliner()->GetRefDevice()->GetFont() );
    aDlg.DisableFontSelection(); //maybe not necessary in future

    if( aDlg.Execute() == RET_OK )
    {
        String aString( aDlg.GetCharacters() );

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
}

//.............................................................................
} //namespace chart
//.............................................................................
