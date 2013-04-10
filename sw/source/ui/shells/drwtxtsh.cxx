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
#include <i18npool/lang.h>
#include <svl/slstitm.hxx>
#include <svl/cjkoptions.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/svdview.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <svx/svdotext.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svx/fontwork.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svl/whiter.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdoutl.hxx>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <initui.hxx>               // fuer SpellPointer
#include <drwtxtsh.hxx>
#include <swundo.hxx>
#include <breakit.hxx>

#include <cmdid.h>
#include <helpid.h>
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#define SwDrawTextShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#include <uitool.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <svx/xtable.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/svdoashp.hxx>
#include <cppuhelper/bootstrap.hxx>
#include "swabstdlg.hxx" //CHINA001
#include "misc.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;



SFX_IMPL_INTERFACE(SwDrawTextShell, SfxShell, SW_RES(STR_SHELLNAME_DRAW_TEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_DRWTXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_DRAW_TEXT_TOOLBOX));
    SFX_CHILDWINDOW_REGISTRATION(SvxFontWorkChildWindow::GetChildWindowId());
}

TYPEINIT1(SwDrawTextShell,SfxShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::Init()
{
    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();
    SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
    //#97471# mouse click _and_ key input at the same time
    if( !pOutliner )
        return ;
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    sal_uLong nCtrl = pOutliner->GetControlWord();
    nCtrl |= EE_CNTRL_AUTOCORRECT;

    SetUndoManager(&pOutliner->GetUndoManager());

    // jetzt versuchen wir mal ein AutoSpell

    const SwViewOption* pVOpt = rSh.GetViewOptions();
    if(pVOpt->IsOnlineSpell())
    {
        nCtrl |= EE_CNTRL_ONLINESPELLING|EE_CNTRL_ALLOWBIGOBJS;
    }
    else
        nCtrl &= ~(EE_CNTRL_ONLINESPELLING);

    pOutliner->SetControlWord(nCtrl);
    pOLV->ShowCursor();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwDrawTextShell::SwDrawTextShell(SwView &rV) :
    SfxShell(&rV),
    rView(rV)
{
    SwWrtShell &rSh = GetShell();
    SetPool(rSh.GetAttrPool().GetSecondaryPool());

    Init();

    rSh.NoEdit(sal_True);
    SetName(String::CreateFromAscii("ObjectText"));
    SetHelpId(SW_DRWTXTSHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_DrawText));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



__EXPORT SwDrawTextShell::~SwDrawTextShell()
{
    if ( GetView().GetCurShell() == this )
        rView.ResetSubShell();

    //MA 13. Nov. 96: Das kommt durchaus vor #33141#:
    //(doppel-)Klick von einem Texteditmode in ein anderes Objekt, zwischendurch
    //wird eine andere (Draw-)Shell gepusht, die alte aber noch nicht deletet.
    //Dann wird vor dem Flush wieder ein DrawTextShell gepusht und der Mode ist
    //eingeschaltet. In diesem Moment wird der Dispatcher geflusht und die alte
    //DrawTextShell zerstoert.
//  ASSERT( !pSdrView->IsTextEdit(), "TextEdit in DTor DrwTxtSh?" );
//    if (pSdrView->IsTextEdit())
//      GetShell().EndTextEdit();   // Danebengeklickt, Ende mit Edit

//    GetShell().Edit();
}

SwWrtShell& SwDrawTextShell::GetShell()
{
    return rView.GetWrtShell();
}


/*--------------------------------------------------------------------
    Beschreibung:   Slots mit dieser Statusmethode disablen
 --------------------------------------------------------------------*/

void SwDrawTextShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}


/*************************************************************************
|*
|* Attribute setzen
|*
\************************************************************************/



void SwDrawTextShell::SetAttrToMarked(const SfxItemSet& rAttr)
{
    Rectangle aNullRect;
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    Rectangle aOutRect = pOLV->GetOutputArea();

    if (aNullRect != aOutRect)
    {
        GetShell().GetDrawView()->SetAttributes(rAttr);
//      Init();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



sal_Bool SwDrawTextShell::IsTextEdit()
{
    return pSdrView->IsTextEdit();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecFontWork(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rSh.GetView()));
    SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( rReq.GetArgs() )
    {
        pVFrame->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                ((const SfxBoolItem&) (rReq.GetArgs()->
                                Get(SID_FONTWORK))).GetValue());
    }
    else
        pVFrame->ToggleChildWindow(SvxFontWorkChildWindow::GetChildWindowId());

    pVFrame->GetBindings().Invalidate(SID_FONTWORK);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateFontWork(SfxItemSet& rSet)
{
    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
    rSet.Put(SfxBoolItem(SID_FONTWORK, GetView().GetViewFrame()->HasChildWindow(nId)));
}

/*************************************************************************
|*
|* SfxRequests fuer FontWork bearbeiten
|*
\************************************************************************/



void SwDrawTextShell::ExecFormText(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( rMarkList.GetMarkCount() == 1 && rReq.GetArgs() )
    {
        const SfxItemSet& rSet = *rReq.GetArgs();
        const SfxPoolItem* pItem;

        //ask for the ViewFrame here - "this" may not be valid any longer!
        SfxViewFrame* pVFrame = GetView().GetViewFrame();
        if ( pDrView->IsTextEdit() )
        {
            //#111733# Sometimes SdrEndTextEdit() initiates the change in selection and
            // 'this' is not valid anymore
            SwView& rTempView = GetView();
            pDrView->SdrEndTextEdit(sal_True);
            //this removes the current shell from the dispatcher stack!!
            rTempView.AttrChangedNotify(&rSh);
        }

        pDrView->SetAttributes(rSet);
    }

}

/*************************************************************************
|*
|* Statuswerte fuer FontWork zurueckgeben
|*
\************************************************************************/



void SwDrawTextShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = NULL;
    SvxFontWorkDialog* pDlg = NULL;

    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();

    SfxViewFrame* pVFrame = GetView().GetViewFrame();
    if ( pVFrame->HasChildWindow(nId) )
        pDlg = (SvxFontWorkDialog*)(pVFrame->GetChildWindow(nId)->GetWindow());

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >(pObj);
    const bool bDeactivate(
        !pObj ||
        !pTextObj ||
        !pTextObj->HasText() ||
        dynamic_cast< const SdrObjCustomShape* >(pObj)); // #121538# no FontWork for CustomShapes

    if (bDeactivate)
    {
        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
    }
    else
    {
        if ( pDlg )
            pDlg->SetColorTable(XColorList::GetStdColorList());

        pDrView->GetAttributes( rSet );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecDrawLingu(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    if( rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() )
    {
        switch(rReq.GetSlot())
        {
        case SID_THESAURUS:
            pOLV->StartThesaurus();
            break;

        case SID_HANGUL_HANJA_CONVERSION:
            pOLV->StartTextConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, NULL,
                    i18n::TextConversionOption::CHARACTER_BY_CHARACTER, sal_True, sal_False );
            break;

        case SID_CHINESE_CONVERSION:
            {
                //open ChineseTranslationDialog
                Reference< XComponentContext > xContext(
                    ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
                if(xContext.is())
                {
                    Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                    if(xMCF.is())
                    {
                        Reference< ui::dialogs::XExecutableDialog > xDialog(
                                xMCF->createInstanceWithContext(
                                    rtl::OUString::createFromAscii("com.sun.star.linguistic2.ChineseTranslationDialog")
                                    , xContext), UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(0);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = rtl::OUString::createFromAscii("ParentWindow");
                            aParam.Value <<= makeAny(xDialogParentWindow);
                            pArray[0] <<= makeAny(aParam);
                            xInit->initialize( aSeq );

                            //execute dialog
                            sal_Int16 nDialogRet = xDialog->execute();
                            if( RET_OK == nDialogRet )
                            {
                                //get some parameters from the dialog
                                sal_Bool bToSimplified = sal_True;
                                sal_Bool bUseVariants = sal_True;
                                sal_Bool bCommonTerms = sal_True;
                                Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                                if( xProp.is() )
                                {
                                    try
                                    {
                                        xProp->getPropertyValue( C2U("IsDirectionToSimplified") ) >>= bToSimplified;
                                        xProp->getPropertyValue( C2U("IsUseCharacterVariants") ) >>= bUseVariants;
                                        xProp->getPropertyValue( C2U("IsTranslateCommonTerms") ) >>= bCommonTerms;
                                    }
                                    catch( Exception& )
                                    {
                                    }
                                }

                                //execute translation
                                sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                                sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                                sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                                if( !bCommonTerms )
                                    nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                                Font aTargetFont = pOLV->GetWindow()->GetDefaultFont( DEFAULTFONT_CJK_TEXT,
                                            nTargetLang, DEFAULTFONT_FLAGS_ONLYONE );

                                pOLV->StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, sal_False, sal_False );
                            }
                        }
                        Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
                        if( xComponent.is() )
                            xComponent->dispose();
                    }
                }
            }
            break;

        default:
            ASSERT(!this, "unexpected slot-id");
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
void SwDrawTextShell::ExecDraw(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();

    switch (rReq.GetSlot())
    {
        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        {
            sal_Unicode cIns = 0;
            switch(rReq.GetSlot())
            {
                case FN_INSERT_SOFT_HYPHEN: cIns = CHAR_SOFTHYPHEN; break;
                case FN_INSERT_HARDHYPHEN: cIns = CHAR_HARDHYPHEN; break;
                case FN_INSERT_HARD_SPACE: cIns = CHAR_HARDBLANK; break;
                case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
                case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
                case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
                case SID_INSERT_ZWNBSP: cIns = CHAR_ZWNBSP; break;
            }
            pOLV->InsertText( String(cIns));
            rReq.Done();
        }
        break;
        case SID_CHARMAP:
    {  // Sonderzeichen einfuegen
            InsertSymbol(rReq);
            break;
    }
          case FN_INSERT_STRING:
                {
            const SfxItemSet *pNewAttrs = rReq.GetArgs();
                        sal_uInt16 nSlot = rReq.GetSlot();
            const SfxPoolItem* pItem = 0;
                        if(pNewAttrs)
            {
                                pNewAttrs->GetItemState(nSlot, sal_False, &pItem );
                             pOLV->InsertText(((const SfxStringItem *)pItem)->GetValue());
            }
                        break;
                }

        case SID_SELECTALL:
        {
            SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
            if(pOutliner)
            {
                sal_uLong nParaCount = pOutliner->GetParagraphCount();
                if (nParaCount > 0)
                    pOLV->SelectRange(0L, sal_uInt16(nParaCount) );
            }
        }
        break;

        case FN_FORMAT_RESET:   // delete hard text attributes
        {
            pOLV->RemoveAttribsKeepLanguages( true );
            pOLV->GetEditView().GetEditEngine()->RemoveFields(sal_True);
            rReq.Done();
        }
        break;

        case FN_ESCAPE:
            if (pSdrView->IsTextEdit())
            {
                // Shellwechsel!
                rSh.EndTextEdit();
                SwView& rTempView = rSh.GetView();
                rTempView.ExitDraw();
                rSh.Edit();
                rTempView.AttrChangedNotify(&rSh);
                return;
            }
            break;
        case FN_DRAWTEXT_ATTR_DLG:
            {
                SfxItemSet aNewAttr( pSdrView->GetModel()->GetItemPool() );
                pSdrView->GetAttributes( aNewAttr );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if ( pFact )
                {
                    SfxAbstractTabDialog *pDlg = pFact->CreateTextTabDialog(
                                &(GetView().GetViewFrame()->GetWindow()),
                                &aNewAttr, pSdrView );
                    sal_uInt16 nResult = pDlg->Execute();

                    if (nResult == RET_OK)
                    {
                        if (pSdrView->AreObjectsMarked())
                        {
                            pSdrView->SetAttributes(*pDlg->GetOutputItemSet());
                            rReq.Done(*(pDlg->GetOutputItemSet()));
                        }
                    }

                    delete( pDlg );
                }
            }
            break;
        case SID_TABLE_VERT_NONE:
        case SID_TABLE_VERT_CENTER:
        case SID_TABLE_VERT_BOTTOM:
            {
                sal_uInt16 nSId = rReq.GetSlot();
                if (pSdrView->AreObjectsMarked())
                {
                    SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
                    if (nSId == SID_TABLE_VERT_CENTER)
                        eTVA = SDRTEXTVERTADJUST_CENTER;
                    else if (nSId == SID_TABLE_VERT_BOTTOM)
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;

                    SfxItemSet aNewAttr( pSdrView->GetModel()->GetItemPool() );
                    pSdrView->GetAttributes( aNewAttr );
                    aNewAttr.Put(SdrTextVertAdjustItem(eTVA));
                    pSdrView->SetAttributes(aNewAttr);
                    rReq.Done();
                }

            }
            break;

        default:
            ASSERT(!this, "unexpected slot-id");
            return;
    }

    GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();
}

/*--------------------------------------------------------------------
    Beschreibung:   Undo ausfuehren
 --------------------------------------------------------------------*/



void SwDrawTextShell::ExecUndo(SfxRequest &rReq)
{
    if( IsTextEdit() )
    {
        sal_Bool bCallBase = sal_True;
        const SfxItemSet* pArgs = rReq.GetArgs();
        if( pArgs )
        {
            sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
            const SfxPoolItem* pItem;
            switch( nId )
            {
            case SID_UNDO:
            case SID_REDO:
                if( SFX_ITEM_SET == pArgs->GetItemState( nId, sal_False, &pItem ) &&
                    1 < (nCnt = ((SfxUInt16Item*)pItem)->GetValue()) )
                {
                    // then we make by ourself.
                    ::svl::IUndoManager* pUndoManager = GetUndoManager();
                    if( pUndoManager )
                    {
                        if( SID_UNDO == nId )
                            while( nCnt-- )
                                pUndoManager->Undo();
                        else
                            while( nCnt-- )
                                pUndoManager->Redo();
                    }
                    bCallBase = sal_False;
                    GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);
                }
                break;
            }
        }
        if( bCallBase )
        {
            SfxViewFrame *pSfxViewFrame = GetView().GetViewFrame();
            pSfxViewFrame->ExecuteSlot(rReq, pSfxViewFrame->GetInterface());
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Zustand Undo
 --------------------------------------------------------------------*/



void SwDrawTextShell::StateUndo(SfxItemSet &rSet)
{
    if ( !IsTextEdit() )
        return;

    SfxViewFrame *pSfxViewFrame = GetView().GetViewFrame();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        switch ( nWhich )
        {
        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                ::svl::IUndoManager* pUndoManager = GetUndoManager();
                if( pUndoManager )
                {
                    UniString (::svl::IUndoManager:: *fnGetComment)( size_t, bool const ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWhich )
                    {
                        nCount = pUndoManager->GetUndoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pUndoManager->GetRedoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetRedoActionComment;
                    }
                    if( nCount )
                    {
                        String sList;
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            ( sList += (pUndoManager->*fnGetComment)( n, ::svl::IUndoManager::TopLevel ) )
                                    += '\n';

                        SfxStringListItem aItem( nWhich );
                        aItem.SetString( sList );
                        rSet.Put( aItem );
                    }
                }
                else
                    rSet.DisableItem( nWhich );
            }
            break;

        default:
            pSfxViewFrame->GetSlotState( nWhich,
                                    pSfxViewFrame->GetInterface(), &rSet );
        }

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::ExecTransliteration( SfxRequest & rReq )
{
    using namespace i18n;
    {
        sal_uInt32 nMode = 0;

        switch( rReq.GetSlot() )
        {
        case SID_TRANSLITERATE_SENTENCE_CASE:
            nMode = TransliterationModulesExtra::SENTENCE_CASE;
            break;
        case SID_TRANSLITERATE_TITLE_CASE:
            nMode = TransliterationModulesExtra::TITLE_CASE;
            break;
        case SID_TRANSLITERATE_TOGGLE_CASE:
            nMode = TransliterationModulesExtra::TOGGLE_CASE;
            break;
        case SID_TRANSLITERATE_UPPER:
            nMode = TransliterationModules_LOWERCASE_UPPERCASE;
            break;
        case SID_TRANSLITERATE_LOWER:
            nMode = TransliterationModules_UPPERCASE_LOWERCASE;
            break;

        case SID_TRANSLITERATE_HALFWIDTH:
            nMode = TransliterationModules_FULLWIDTH_HALFWIDTH;
            break;
        case SID_TRANSLITERATE_FULLWIDTH:
            nMode = TransliterationModules_HALFWIDTH_FULLWIDTH;
            break;

        case SID_TRANSLITERATE_HIRAGANA:
            nMode = TransliterationModules_KATAKANA_HIRAGANA;
            break;
        case SID_TRANSLITERATE_KATAGANA:
            nMode = TransliterationModules_HIRAGANA_KATAKANA;
            break;

        default:
            ASSERT(!this, "falscher Dispatcher");
        }

        if( nMode )
        {
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            pOLV->TransliterateText( nMode );
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Sonderzeichen einfuegen (siehe SDraw: FUBULLET.CXX)
 --------------------------------------------------------------------*/

void SwDrawTextShell::InsertSymbol(SfxRequest& rReq)
{
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    if(!pOLV)
        return;
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = 0;
    if( pArgs )
        pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), sal_False, &pItem);

    String sSym;
    String sFontName;
    if ( pItem )
    {
        sSym = ((const SfxStringItem*)pItem)->GetValue();
        const SfxPoolItem* pFtItem = NULL;
        pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), sal_False, &pFtItem);
        const SfxStringItem* pFontItem = PTR_CAST( SfxStringItem, pFtItem );
        if ( pFontItem )
            sFontName = pFontItem->GetValue();
    }

    SfxItemSet aSet(pOLV->GetAttribs());
    sal_uInt16 nScript = pOLV->GetSelectedScriptType();
    SvxFontItem aSetDlgFont( RES_CHRATR_FONT );
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, sal_False );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aSetDlgFont = *(SvxFontItem*)pI;
        else
            aSetDlgFont = (SvxFontItem&)aSet.Get( GetWhichOfScript(
                        SID_ATTR_CHAR_FONT,
                        GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage() ) ));
        if (!sFontName.Len())
            sFontName = aSetDlgFont.GetFamilyName();
    }

    Font aFont(sFontName, Size(1,1));
    if(!sSym.Len())
    {
        SfxAllItemSet aAllSet( GetPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, sal_False ) );

        SwViewOption aOpt(*rView.GetWrtShell().GetViewOptions());
        String sSymbolFont = aOpt.GetSymbolFont();
        if( sSymbolFont.Len() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aSetDlgFont.GetFamilyName() ) );

        // Wenn Zeichen selektiert ist kann es angezeigt werden
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( rView.GetWindow(), aAllSet,
            rView.GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );
        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pCItem, SfxStringItem, SID_CHARMAP, sal_False );
            SFX_ITEMSET_ARG( pDlg->GetOutputItemSet(), pFontItem, SvxFontItem, SID_ATTR_CHAR_FONT, sal_False );
            if ( pFontItem )
            {
                aFont.SetName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }

            if ( pCItem )
            {
                sSym  = pCItem->GetValue();
                aOpt.SetSymbolFont(aFont.GetName());
                SW_MOD()->ApplyUsrPref(aOpt, &rView);
            }
        }

        delete( pDlg );
    }

    if( sSym.Len() )
    {
        // nicht flackern
        pOLV->HideCursor();
        SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
        pOutliner->SetUpdateMode(sal_False);

        SfxItemSet aOldSet( pOLV->GetAttribs() );
        SfxItemSet aFontSet( *aOldSet.GetPool(),
                            EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
                            EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CJK,
                            EE_CHAR_FONTINFO_CTL, EE_CHAR_FONTINFO_CTL,
                            0 );
        aFontSet.Set( aOldSet );

        // String einfuegen
        pOLV->InsertText( sSym );

        // attributieren (Font setzen)
        SfxItemSet aFontAttribSet( *aFontSet.GetPool(), aFontSet.GetRanges() );
        SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                aFont.GetStyleName(), aFont.GetPitch(),
                                aFont.GetCharSet(),
                                EE_CHAR_FONTINFO );
        nScript = pBreakIt->GetAllScriptsOfText( sSym );
        if( SCRIPTTYPE_LATIN & nScript )
            aFontAttribSet.Put( aFontItem, EE_CHAR_FONTINFO );
        if( SCRIPTTYPE_ASIAN & nScript )
            aFontAttribSet.Put( aFontItem, EE_CHAR_FONTINFO_CJK );
        if( SCRIPTTYPE_COMPLEX & nScript )
            aFontAttribSet.Put( aFontItem, EE_CHAR_FONTINFO_CTL );
        pOLV->SetAttribs(aFontAttribSet);

        // Selektion loeschen
        ESelection aSel(pOLV->GetSelection());
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOLV->SetSelection(aSel);

        // Alten Font restaurieren
        pOLV->SetAttribs( aFontSet );

        // ab jetzt wieder anzeigen
        pOutliner->SetUpdateMode(sal_True);
        pOLV->ShowCursor();

        rReq.AppendItem( SfxStringItem( GetPool().GetWhich(SID_CHARMAP), sSym ) );
        if(aFont.GetName().Len())
            rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aFont.GetName() ) );
        rReq.Done();
    }
}
/*-- 22.10.2003 14:26:32---------------------------------------------------

  -----------------------------------------------------------------------*/
::svl::IUndoManager* SwDrawTextShell::GetUndoManager()
{
    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();
    SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
    pOutliner = pSdrView->GetTextEditOutliner();
    return &pOutliner->GetUndoManager();
}

void SwDrawTextShell::GetStatePropPanelAttr(SfxItemSet &rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    SwWrtShell &rSh = GetShell();
    pSdrView = rSh.GetDrawView();

    SfxItemSet aAttrs( pSdrView->GetModel()->GetItemPool() );
    pSdrView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                sal_Bool bContour = sal_False;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SFX_ITEM_DONTCARE )
                {
                    bContour = ( ( const SdrTextContourFrameItem& )aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SFX_ITEM_DONTCARE != eVState && SFX_ITEM_DONTCARE != eHState)
                if(SFX_ITEM_DONTCARE != eVState)
                {
                    SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)aAttrs.Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
                    sal_Bool bSet = nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP||
                            nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER ||
                            nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM;
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, sal_False));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}
