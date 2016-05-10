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

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <i18nutil/unicode.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <comphelper/processfactory.hxx>

#include "undo/undomanager.hxx"
#include <vcl/waitobj.hxx>
#include <svl/aeitem.hxx>
#include <editeng/editstat.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urlbmk.hxx>
#include <svx/svdpagv.hxx>
#include <svx/fmshell.hxx>
#include <vcl/scrbar.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdograf.hxx>
#include <svl/eitem.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <editeng/flditem.hxx>
#include <svx/ruler.hxx>
#include <svx/obj3d.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/urlobj.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/ipclient.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <avmedia/mediawindow.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/docfile.hxx>

#include "DrawViewShell.hxx"
#include "slideshow.hxx"
#include "ViewShellImplementation.hxx"
#include "ViewShellHint.hxx"
#include "framework/FrameworkHelper.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "drawdoc.hxx"
#include "fusel.hxx"
#include "futext.hxx"
#include "fuconrec.hxx"
#include "fuconcs.hxx"
#include "fuconuno.hxx"
#include "fuconbez.hxx"
#include "fuediglu.hxx"
#include "fuconarc.hxx"
#include "fucon3d.hxx"
#include "sdresid.hxx"
#include "Outliner.hxx"
#include "PresentationViewShell.hxx"
#include "sdpage.hxx"
#include "FrameView.hxx"
#include "zoomlist.hxx"
#include "drawview.hxx"
#include "DrawDocShell.hxx"
#include "sdattr.hxx"
#include "ViewShellBase.hxx"
#include "ToolBarManager.hxx"
#include "anminfo.hxx"
#include "optsitem.hxx"
#include "Window.hxx"
#include "fuformatpaintbrush.hxx"
#include "fuzoom.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

namespace sd {

// Permanent Functions

void ImpAddPrintableCharactersToTextEdit(SfxRequest& rReq, ::sd::View* pView)
{
    // evtl. feed characters to activated textedit
    const SfxItemSet* pSet = rReq.GetArgs();

    if(pSet)
    {
        OUString aInputString;

        if(SfxItemState::SET == pSet->GetItemState(SID_ATTR_CHAR))
            aInputString = static_cast<const SfxStringItem&>(pSet->Get(SID_ATTR_CHAR)).GetValue();

        if(!aInputString.isEmpty())
        {
            OutlinerView* pOLV = pView->GetTextEditOutlinerView();

            if(pOLV)
            {
                for(sal_Int32 a(0); a < aInputString.getLength(); a++)
                {
                    sal_Char aChar = (sal_Char)aInputString[a];
                    vcl::KeyCode aKeyCode;
                    KeyEvent aKeyEvent(aChar, aKeyCode);

                    // add actual character
                    pOLV->PostKeyEvent(aKeyEvent);
                }
            }
        }
    }
}

void DrawViewShell::FuPermanent(SfxRequest& rReq)
{
    // We do not execute a thing during a native slide show

    if (SlideShow::IsRunning(GetViewShellBase()))
        return;

    sal_uInt16 nSId = rReq.GetSlot();

    if( HasCurrentFunction() &&
        ( nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
          nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL ) )
    {
        rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );

        FuText* pFuText = dynamic_cast< FuText* >( xFunc.get() );

        if( pFuText )
        {
            pFuText->SetPermanent(true);
            xFunc->ReceiveRequest( rReq );

            Invalidate();

            Invalidate();

            // evtl. feed characters to activated textedit
            if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
                ImpAddPrintableCharactersToTextEdit(rReq, GetView());

            rReq.Done();
            return;
        }
    }

    CheckLineTo (rReq);
    sal_uInt16 nOldSId = 0;
    bool bPermanent = false;

    if( !mpDrawView )
        return;

    if(HasCurrentFunction())
    {
        if( (nSId == SID_FORMATPAINTBRUSH) && (GetCurrentFunction()->GetSlotID() == SID_TEXTEDIT) )
        {
            // save text edit mode for format paintbrush!
            SetOldFunction( GetCurrentFunction() );
        }
        else
        {
            if(GetOldFunction() == GetCurrentFunction())
            {
                SetOldFunction(nullptr);
            }
        }

        if ( nSId != SID_TEXTEDIT && nSId != SID_ATTR_CHAR && nSId != SID_TEXT_FITTOSIZE &&
             nSId != SID_ATTR_CHAR_VERTICAL && nSId != SID_TEXT_FITTOSIZE_VERTICAL &&
             nSId != SID_FORMATPAINTBRUSH &&
             mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }

        if( HasCurrentFunction() )
        {
            nOldSId = GetCurrentFunction()->GetSlotID();

            if (nOldSId == nSId ||
                ((nOldSId == SID_TEXTEDIT || nOldSId == SID_ATTR_CHAR || nOldSId == SID_TEXT_FITTOSIZE ||
                nOldSId == SID_ATTR_CHAR_VERTICAL || nOldSId == SID_TEXT_FITTOSIZE_VERTICAL) &&
                (nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
                nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL )))
            {
                bPermanent = true;
            }

            GetCurrentFunction()->Deactivate();
        }

        SetCurrentFunction(nullptr);

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate(nOldSId);
        rBind.Update(nOldSId);
    }

    switch ( nSId )
    {
        case SID_TEXTEDIT:  // BASIC ???
        case SID_ATTR_CHAR:
        case SID_ATTR_CHAR_VERTICAL:
        case SID_TEXT_FITTOSIZE:
        case SID_TEXT_FITTOSIZE_VERTICAL:
        {
            SetCurrentFunction( FuText::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            GetCurrentFunction()->DoExecute(rReq);

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_ATTR_CHAR );
            rBindings.Invalidate( SID_ATTR_CHAR_VERTICAL );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE_VERTICAL );

            // evtl. feed characters to activated textedit
            if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
                ImpAddPrintableCharactersToTextEdit(rReq, GetView());

            rReq.Done();
        }
        break;

        case SID_FM_CREATE_CONTROL:
        {
            SetCurrentFunction( FuConstructUnoControl::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;

        case SID_FM_CREATE_FIELDCONTROL:
        {
            const SfxUnoAnyItem* pDescriptorItem = rReq.GetArg<SfxUnoAnyItem>(SID_FM_DATACCESS_DESCRIPTOR);
            DBG_ASSERT( pDescriptorItem, "DrawViewShell::FuPermanent(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );

            if(pDescriptorItem)
            {
                // get the form view
                FmFormView* pFormView = dynamic_cast<FmFormView*>( mpDrawView );
                SdrPageView* pPageView = pFormView ? pFormView->GetSdrPageView() : nullptr;

                if(pPageView)
                {
                    svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
                    SdrObject* pNewDBField = pFormView->CreateFieldControl(aDescriptor);

                    if(pNewDBField)
                    {
                        Rectangle aVisArea = GetActiveWindow()->PixelToLogic(Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
                        Point aObjPos(aVisArea.Center());
                        Size aObjSize(pNewDBField->GetLogicRect().GetSize());
                        aObjPos.X() -= aObjSize.Width() / 2;
                        aObjPos.Y() -= aObjSize.Height() / 2;
                        Rectangle aNewObjectRectangle(aObjPos, aObjSize);

                        pNewDBField->SetLogicRect(aNewObjectRectangle);

                        GetView()->InsertObjectAtView(pNewDBField, *pPageView);
                    }
                }
            }
            rReq.Done();
        }
        break;

        case SID_OBJECT_SELECT:
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_CROP:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:
        {
            short nSlotId = rReq.GetSlot();

            if( nSlotId == SID_OBJECT_ROTATE )
            {
                // toggle rotation
                if( nOldSId == nSlotId )
                {
                    nSlotId = SID_OBJECT_SELECT;
                    rReq.SetSlot( nSlotId );
                }
            }

            if (nSlotId == SID_OBJECT_CROOK_ROTATE ||
                nSlotId == SID_OBJECT_CROOK_SLANT ||
                nSlotId == SID_OBJECT_CROOK_STRETCH)
            {
                if ( mpDrawView->GetMarkedObjectList().GetMarkCount() > 0 &&
                    !mpDrawView->IsCrookAllowed( mpDrawView->IsCrookNoContortion() ) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        ::sd::Window* pWindow = GetActiveWindow();
                        ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
                    }
                    else if ( ScopedVclPtr<QueryBox>::Create(GetActiveWindow(), WB_YES_NO,
                                      SD_RESSTR(STR_ASK_FOR_CONVERT_TO_BEZIER)
                                      )->Execute() == RET_YES )
                    {
                        // implicit transformation into bezier
                        WaitObject aWait( GetActiveWindow() );
                        mpDrawView->ConvertMarkedToPathObj(false);
                    }
                }
            }
            else if (nSlotId == SID_OBJECT_SHEAR)
            {
                size_t i = 0;
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const size_t nMarkCnt = rMarkList.GetMarkCount();
                bool b3DObjMarked = false;

                while (i < nMarkCnt && !b3DObjMarked)
                {
                    if (nullptr != dynamic_cast< E3dObject *>( rMarkList.GetMark(i)->GetMarkedSdrObj() ))
                    {
                        b3DObjMarked = true;
                    }
                    else
                    {
                        i++;
                    }
                }

                if ( nMarkCnt > 0 && !b3DObjMarked &&
                     (!mpDrawView->IsShearAllowed() || !mpDrawView->IsDistortAllowed()) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        ::sd::Window* pWindow = GetActiveWindow();
                        ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
                    }
                    else if ( ScopedVclPtr<QueryBox>::Create(GetActiveWindow(), WB_YES_NO,
                                      SD_RESSTR(STR_ASK_FOR_CONVERT_TO_BEZIER)
                                      )->Execute() == RET_YES )
                    {
                        // implicit transformation into bezier
                        WaitObject aWait( GetActiveWindow() );
                        mpDrawView->ConvertMarkedToPathObj(false);
                    }
                }
            }

            SetCurrentFunction( FuSelection::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq) );
            rReq.Done();
            Invalidate( SID_OBJECT_SELECT );
        }
        break;

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:

        case SID_DRAW_RECT:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:
        {
            SetCurrentFunction( FuConstructRectangle::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;
        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_BEZIER_FILL:          // BASIC
        case SID_DRAW_BEZIER_NOFILL:        // BASIC
        {
            SetCurrentFunction( FuConstructBezierPolygon::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent) );
            rReq.Done();
        }
        break;

        case SID_GLUE_EDITMODE:
        {
            if (nOldSId != SID_GLUE_EDITMODE)
            {
                SetCurrentFunction( FuEditGluePoints::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent ) );
            }
            else
            {
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            rReq.Done();
        }
        break;

        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_PIE:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_CIRCLECUT_NOFILL:
        {
            SetCurrentFunction( FuConstructArc::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent) );
            rReq.Done();
        }
        break;

        case SID_3D_CUBE:
        case SID_3D_SHELL:
        case SID_3D_SPHERE:
        case SID_3D_TORUS:
        case SID_3D_HALF_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            SetCurrentFunction( FuConstruct3dObject::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            SetCurrentFunction( FuConstructCustomShape::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq, bPermanent ) );
            rReq.Done();

            if ( nSId != SID_DRAW_CS_ID )
            {
                SfxBindings& rBind = GetViewFrame()->GetBindings();
                rBind.Invalidate( nSId );
                rBind.Update( nSId );
            }
        }
        break;

        case SID_FORMATPAINTBRUSH:
        {
            SetCurrentFunction( FuFormatPaintBrush::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            rReq.Done();
            SfxBindings& rBind = GetViewFrame()->GetBindings();
            rBind.Invalidate( nSId );
            rBind.Update( nSId );
            break;
        }

        case SID_ZOOM_MODE:
        case SID_ZOOM_PANNING:
        {
            mbZoomOnPage = false;
            SetCurrentFunction( FuZoom::Create(this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
            rReq.Done();
        }
        break;

        default:
           break;
    }

    if(HasOldFunction())
    {
        sal_uInt16 nSlotId = GetOldFunction()->GetSlotID();

        GetOldFunction()->Deactivate();
        SetOldFunction(nullptr);

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate( nSlotId );
        rBind.Update( nSlotId );
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
        SetOldFunction( GetCurrentFunction() );

        SetHelpId( GetCurrentFunction()->GetSlotID() );
    }

    // invalidate shell, is faster than every individually (says MI)
    // now explicit the last slot incl. Update()
    Invalidate();

    // CTRL-SID_OBJECT_SELECT -> select first draw object if none is selected yet
    if(SID_OBJECT_SELECT == nSId && HasCurrentFunction() && (rReq.GetModifier() & KEY_MOD1))
    {
        if(!GetView()->AreObjectsMarked())
        {
            // select first object
            GetView()->UnmarkAllObj();
            GetView()->MarkNextObj(true);

            // ...and make it visible
            if(GetView()->AreObjectsMarked())
                GetView()->MakeVisible(GetView()->GetAllMarkedRect(), *GetActiveWindow());
        }
    }

    // with qualifier construct directly
    if(HasCurrentFunction() && (rReq.GetModifier() & KEY_MOD1))
    {
        // get SdOptions
        SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());
        sal_uInt32 nDefaultObjectSizeWidth(pOptions->GetDefaultObjectSizeWidth());
        sal_uInt32 nDefaultObjectSizeHeight(pOptions->GetDefaultObjectSizeHeight());

        // calc position and size
        Rectangle aVisArea = GetActiveWindow()->PixelToLogic(Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
        Point aPagePos = aVisArea.Center();
        aPagePos.X() -= nDefaultObjectSizeWidth / 2;
        aPagePos.Y() -= nDefaultObjectSizeHeight / 2;
        Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));
        SdrPageView* pPageView = mpDrawView->GetSdrPageView();

        if(pPageView)
        {
            // create the default object
            SdrObject* pObj = GetCurrentFunction()->CreateDefaultObject(nSId, aNewObjectRectangle);

            if(pObj)
            {
                // insert into page
                GetView()->InsertObjectAtView(pObj, *pPageView);

                // Now that pFuActual has done what it was created for we
                // can switch on the edit mode for callout objects.
                switch (nSId)
                {
                    case SID_DRAW_CAPTION:
                    case SID_DRAW_CAPTION_VERTICAL:
                    {
                        // Make FuText the current function.
                        SfxUInt16Item aItem (SID_TEXTEDIT, 1);
                        GetViewFrame()->GetDispatcher()->
                            Execute(SID_TEXTEDIT, SfxCallMode::SYNCHRON |
                                SfxCallMode::RECORD, &aItem, 0L);
                        // Put text object into edit mode.
                        GetView()->SdrBeginTextEdit(static_cast<SdrTextObj*>(pObj), pPageView);
                        break;
                    }
                }
            }
        }
    }
}

void DrawViewShell::FuDeleteSelectedObjects()
{
    if( !mpDrawView )
        return;

    bool bConsumed = false;

    //if any placeholders are selected
    if (mpDrawView->IsPresObjSelected(false))
    {
        //If there are placeholders in the list which can be toggled
        //off in edit->master->master elements then do that here,
        std::vector<SdrObject*> aPresMarksToRemove;
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
        for (size_t i=0; i < rMarkList.GetMarkCount(); ++i)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            SdPage* pPage = static_cast<SdPage*>(pObj->GetPage());
            PresObjKind eKind = pPage->GetPresObjKind(pObj);
            if (eKind == PRESOBJ_FOOTER || eKind == PRESOBJ_HEADER ||
                eKind == PRESOBJ_DATETIME || eKind == PRESOBJ_SLIDENUMBER)
            {
                aPresMarksToRemove.push_back(pObj);
            }
        }

        for (SdrObject* pObj : aPresMarksToRemove)
        {
            //Unmark object
            mpDrawView->MarkObj(pObj, mpDrawView->GetSdrPageView(), true);
            SdPage* pPage = static_cast<SdPage*>(pObj->GetPage());
            //remove placeholder from master page
            pPage->DestroyDefaultPresObj(pPage->GetPresObjKind(pObj));
        }

        bConsumed = true;
    }

    // placeholders which cannot be deleted selected
    if (mpDrawView->IsPresObjSelected(false, true, false, true))
    {
        ::sd::Window* pWindow = GetActiveWindow();
        ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
        bConsumed = true;
    }

    if (!bConsumed)
    {
        vcl::KeyCode aKCode(KEY_DELETE);
        KeyEvent aKEvt( 0, aKCode);

        bConsumed = mpDrawView->getSmartTags().KeyInput( aKEvt );

        if (!bConsumed && HasCurrentFunction())
            bConsumed = GetCurrentFunction()->KeyInput(aKEvt);

        if (!bConsumed)
            mpDrawView->DeleteMarked();
    }
}

void DrawViewShell::FuSupport(SfxRequest& rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(static_cast<const SfxUInt16Item&>(rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    // We do not execute a thing during a native slide show
    if(SlideShow::IsRunning(GetViewShellBase()) &&
        (rReq.GetSlot() != SID_PRESENTATION_END &&
         rReq.GetSlot() != SID_SIZE_PAGE))
        return;

    CheckLineTo (rReq);

    if( !mpDrawView )
        return;

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
        case SID_CLEAR_UNDO_STACK:
        {
            GetDocSh()->ClearUndoBuffer();
            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION:
        case SID_PRESENTATION_CURRENT_SLIDE:
        case SID_REHEARSE_TIMINGS:
        {
            ShowSlideShow(rReq);
            rReq.Ignore ();
        }
        break;

        case SID_PRESENTATION_END:
        {
            StopSlideShow(true);

            rReq.Ignore ();
        }
        break;

        case SID_BEZIER_EDIT:
        {
            mpDrawView->SetFrameDragSingles(!mpDrawView->IsFrameDragSingles());

            /******************************************************************
            * turn ObjectBar on
            ******************************************************************/
            if( dynamic_cast< FuSelection* >( GetCurrentFunction().get() ) || dynamic_cast< FuConstructBezierPolygon* >( GetCurrentFunction().get() ) )
            {
                // Tell the tool bar manager about the context change.
                GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*this,*mpDrawView);
            }

            Invalidate(SID_BEZIER_EDIT);
            rReq.Ignore();
        }
        break;

        case SID_OBJECT_CLOSE:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if ( rMarkList.GetMark(0) && !mpDrawView->IsAction() )
            {
                SdrPathObj* pPathObj = static_cast<SdrPathObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj());
                const bool bUndo = mpDrawView->IsUndoEnabled();
                if( bUndo )
                    mpDrawView->BegUndo(SD_RESSTR(STR_UNDO_BEZCLOSE));

                mpDrawView->UnmarkAllPoints();

                if( bUndo )
                    mpDrawView->AddUndo(new SdrUndoGeoObj(*pPathObj));

                pPathObj->ToggleClosed();

                if( bUndo )
                    mpDrawView->EndUndo();
            }
            rReq.Done();
        }
        break;

        case SID_CUT:
        {
            if ( mpDrawView->IsPresObjSelected(false, true, false, true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
            }
            else
            {
                if(HasCurrentFunction())
                {
                    GetCurrentFunction()->DoCut();
                }
                else if(mpDrawView)
                {
                    mpDrawView->DoCut();
                }
            }
            rReq.Ignore ();
        }
        break;

        case SID_COPY:
        {
            if ( mpDrawView->IsPresObjSelected(false, true, false, true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                ScopedVclPtr<InfoBox>::Create(pWindow, SD_RESSTR(STR_ACTION_NOTPOSSIBLE) )->Execute();
            }
            else
            {
                if(HasCurrentFunction())
                {
                    GetCurrentFunction()->DoCopy();
                }
                else if( mpDrawView )
                {
                    mpDrawView->DoCopy();
                }
            }
            rReq.Ignore ();
        }
        break;

        case SID_PASTE:
        {
            WaitObject aWait( GetActiveWindow() );

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPaste();
            }
            else if(mpDrawView)
            {
                mpDrawView->DoPaste();
            }

            rReq.Ignore ();
        }
        break;

        case SID_UNICODE_NOTATION_TOGGLE:
        {
            if( mpDrawView->IsTextEdit() )
            {
                OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
                if( pOLV )
                {
                    OUString sInput = pOLV->GetSurroundingText();
                    ESelection aSel( pOLV->GetSelection() );
                    if( aSel.nStartPos > aSel.nEndPos )
                        aSel.nEndPos = aSel.nStartPos;

                    //calculate a valid end-position by reading logical characters
                    sal_Int32 nUtf16Pos=0;
                    while( (nUtf16Pos < sInput.getLength()) && (nUtf16Pos < aSel.nEndPos) )
                    {
                        sInput.iterateCodePoints(&nUtf16Pos);
                        //The mouse can set the cursor in the middle of a multi-unit character,
                        //    so reset to the proper end of the logical characters
                        if( nUtf16Pos > aSel.nEndPos )
                            aSel.nEndPos = nUtf16Pos;
                    }

                    ToggleUnicodeCodepoint aToggle;
                    while( nUtf16Pos && aToggle.AllowMoreInput( sInput[nUtf16Pos-1]) )
                        --nUtf16Pos;
                    OUString sReplacement = aToggle.ReplacementString();
                    if( !sReplacement.isEmpty() )
                    {
                        OUString sStringToReplace = aToggle.StringToReplace();
                        mpDrawView->BegUndo(sStringToReplace +"->"+ sReplacement);
                        aSel.nStartPos = aSel.nEndPos - sStringToReplace.getLength();
                        pOLV->SetSelection( aSel );
                        pOLV->InsertText(sReplacement, true);
                        mpDrawView->EndUndo();
                    }
                }
            }
        }
        break;

        case SID_PASTE_UNFORMATTED:
        {
            WaitObject aWait( GetActiveWindow() );

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPasteUnformatted();
            }
            else if(mpDrawView)
            {
                sal_Int8 nAction = DND_ACTION_COPY;
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                if (aDataHelper.GetTransferable().is())
                {
                    mpDrawView->InsertData( aDataHelper,
                                            GetActiveWindow()->PixelToLogic( Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                            nAction, false, SotClipboardFormatId::STRING);
                }
            }

            rReq.Ignore ();
        }
        break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            WaitObject              aWait( GetActiveWindow() );
            TransferableDataHelper  aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            const SfxItemSet*       pReqArgs = rReq.GetArgs();
            SotClipboardFormatId    nFormat = SotClipboardFormatId::NONE;

            if( pReqArgs )
            {
                const SfxUInt32Item* pIsActive = rReq.GetArg<SfxUInt32Item>(SID_CLIPBOARD_FORMAT_ITEMS);
                nFormat = static_cast<SotClipboardFormatId>(pIsActive->GetValue());
            }

            if( nFormat != SotClipboardFormatId::NONE && aDataHelper.GetTransferable().is() )
            {
                sal_Int8 nAction = DND_ACTION_COPY;

                if( !mpDrawView->InsertData( aDataHelper,
                                          GetActiveWindow()->PixelToLogic( Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                          nAction, false, nFormat ) )
                {
                    INetBookmark    aINetBookmark( "", "" );

                    if( ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), "", nullptr );
                    }
                }
            }
        }
        break;

        case SID_DELETE:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

                if (pOLV)
                {
                    vcl::KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode);
                    //pOLV->PostKeyEvent(aKEvt);
                    // We use SdrObjEditView to handle DEL for underflow handling
                    mpDrawView->KeyInput(aKEvt, nullptr);

                }
            }
            else
            {
                FuDeleteSelectedObjects();
            }
            rReq.Ignore ();
        }
        break;

        case SID_NOTES_MODE:
        case SID_SLIDE_MASTER_MODE:
        case SID_NOTES_MASTER_MODE:
        case SID_HANDOUT_MASTER_MODE:

            // AutoLayouts have to be ready.
            GetDoc()->StopWorkStartupDelay();
            SAL_FALLTHROUGH;

        case SID_DRAWINGMODE:
        case SID_SLIDE_SORTER_MODE:
        case SID_OUTLINE_MODE:
            // Let the sub-shell manager handle the slot handling.
            framework::FrameworkHelper::Instance(GetViewShellBase())->HandleModeChangeSlot(
                nSId,
                rReq);
            rReq.Ignore ();
            break;

        case SID_MASTERPAGE:          // BASIC
        {
            // AutoLayouts needs to be finished
            GetDoc()->StopWorkStartupDelay();

            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                const SfxBoolItem* pIsActive = rReq.GetArg<SfxBoolItem>(SID_MASTERPAGE);
                mbIsLayerModeActive = pIsActive->GetValue ();
            }

            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

            // Is there a page with the AutoLayout "Title"?
            bool bFound = false;
            sal_uInt16 i = 0;
            sal_uInt16 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

            while (i < nCount && !bFound)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                if (nSId == SID_SLIDE_MASTER_MODE && pPage->GetAutoLayout() != AUTOLAYOUT_TITLE)
                {
                    bFound = true;
                    SwitchPage((pPage->GetPageNum() - 1) / 2);
                }

                i++;
            }

            // turn on default layer of MasterPage
            mpDrawView->SetActiveLayer( SD_RESSTR(STR_LAYER_BCKGRNDOBJ) );

            ChangeEditMode(EM_MASTERPAGE, mbIsLayerModeActive);

            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));

            InvalidateWindows();
            Invalidate();

            rReq.Done();
        }
        break;

        case SID_CLOSE_MASTER_VIEW:
        {
            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

            // Switch page back to the first one.  Not doing so leads to a
            // crash.  This seems to be some bug in the edit mode switching
            // and page switching methods.
            SwitchPage (0);
            ChangeEditMode(EM_PAGE, IsLayerModeActive());
            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));

            if(HasCurrentFunction(SID_BEZIER_EDIT))
            {
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SfxCallMode::ASYNCHRON);
            }

            rReq.Done();
        }
        break;

        case SID_RULER:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            // Remember old ruler state
            bool bOldHasRuler(HasRuler());

            if ( pReqArgs )
            {
                const SfxBoolItem* pIsActive = rReq.GetArg<SfxBoolItem>(SID_RULER);
                SetRuler (pIsActive->GetValue ());
            }
            else SetRuler (!HasRuler());

            // Did ruler state change? Tell that to SdOptions, too.
            bool bHasRuler(HasRuler());

            if(bOldHasRuler != bHasRuler)
            {
                SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());

                if(pOptions && pOptions->IsRulerVisible() != bHasRuler)
                {
                    pOptions->SetRulerVisible(bHasRuler);
                }
            }

            Invalidate (SID_RULER);
            Resize();
            rReq.Done ();
        }
        break;

        case SID_SIZE_PAGE:
        case SID_SIZE_PAGE_WIDTH:  // BASIC
        {
            mbZoomOnPage = ( rReq.GetSlot() == SID_SIZE_PAGE );

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if ( pPageView )
            {
                Point aPagePos(0, 0); // = pPageView->GetOffset();
                Size aPageSize = pPageView->GetPage()->GetSize();

                aPagePos.X() += aPageSize.Width()  / 2;
                aPageSize.Width() = (long) (aPageSize.Width() * 1.03);

                if( rReq.GetSlot() == SID_SIZE_PAGE )
                {
                    aPagePos.Y() += aPageSize.Height() / 2;
                    aPageSize.Height() = (long) (aPageSize.Height() * 1.03);
                    aPagePos.Y() -= aPageSize.Height() / 2;
                }
                else
                {
                    Point aPt = GetActiveWindow()->PixelToLogic( Point( 0, GetActiveWindow()->GetSizePixel().Height() / 2 ) );
                    aPagePos.Y() += aPt.Y();
                    aPageSize.Height() = 2;
                }

                aPagePos.X() -= aPageSize.Width()  / 2;

                SetZoomRect( Rectangle( aPagePos, aPageSize ) );

                Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                mpZoomList->InsertZoomRect(aVisAreaWin);
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_SIZE_REAL:  // BASIC
        {
            mbZoomOnPage = false;
            SetZoom( 100 );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_IN:  // BASIC
        {
            mbZoomOnPage = false;
            SetZoom( std::max( (long) ( GetActiveWindow()->GetZoom() / 2 ), (long) GetActiveWindow()->GetMinZoom() ) );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_OUT:
        {
            mbZoomOnPage = false;
            SetZoom( std::min( (long) ( GetActiveWindow()->GetZoom() * 2 ), (long) GetActiveWindow()->GetMaxZoom() ) );
            Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_SIZE_VISAREA:
        {
            Rectangle aVisArea = mpFrameView->GetVisArea();
            Size aVisAreaSize = aVisArea.GetSize();

            if (aVisAreaSize.Height()!=0 && aVisAreaSize.Width()!=0)
            {
                mbZoomOnPage = false;
                SetZoomRect(aVisArea);
                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
            }
            rReq.Done ();
        }
        break;

        // name confusion: SID_SIZE_OPTIMAL -> Zoom onto selected objects
        // --> Is offered as object zoom in program
        case SID_SIZE_OPTIMAL:  // BASIC
        {
            mbZoomOnPage = false;
            if ( mpDrawView->AreObjectsMarked() )
            {
                maMarkRect = mpDrawView->GetAllMarkedRect();
                long nW = (long) (maMarkRect.GetWidth()  * 1.03);
                long nH = (long) (maMarkRect.GetHeight() * 1.03);
                Point aPos = maMarkRect.Center();
                aPos.X() -= nW / 2;
                aPos.Y() -= nH / 2;
                if ( nW && nH )
                {
                    SetZoomRect(Rectangle(aPos, Size(nW, nH)));

                    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                    mpZoomList->InsertZoomRect(aVisAreaWin);
                }
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        // name confusion: SID_SIZE_ALL -> Zoom onto all objects
        // --> Is offered as optimal in program
        case SID_SIZE_ALL:  // BASIC
        {
            mbZoomOnPage = false;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( pPageView )
            {
                Rectangle aBoundRect( pPageView->GetObjList()->GetAllObjBoundRect() );

                long nW = (long) (aBoundRect.GetWidth() * 1.03);
                long nH = (long) (aBoundRect.GetHeight() * 1.03);
                Point aPos = aBoundRect.Center();
                aPos.X() -= nW / 2;
                aPos.Y() -= nH / 2;
                if ( nW && nH )
                {
                    SetZoomRect( Rectangle( aPos, Size( nW, nH ) ) );

                    Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                    mpZoomList->InsertZoomRect(aVisAreaWin);
                }

                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_PREV:
        {
            if (mpDrawView->IsTextEdit())
            {
                mpDrawView->SdrEndTextEdit();
            }

            if (mpZoomList->IsPreviousPossible())
            {
                // set previous ZoomRect
                SetZoomRect(mpZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (mpDrawView->IsTextEdit())
            {
                mpDrawView->SdrEndTextEdit();
            }

            if (mpZoomList->IsNextPossible())
            {
                // set next ZoomRect
                SetZoomRect(mpZoomList->GetNextZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_GLUE_INSERT_POINT:
        case SID_GLUE_PERCENT:
        case SID_GLUE_ESCDIR:
        case SID_GLUE_ESCDIR_LEFT:
        case SID_GLUE_ESCDIR_RIGHT:
        case SID_GLUE_ESCDIR_TOP:
        case SID_GLUE_ESCDIR_BOTTOM:
        case SID_GLUE_HORZALIGN_CENTER:
        case SID_GLUE_HORZALIGN_LEFT:
        case SID_GLUE_HORZALIGN_RIGHT:
        case SID_GLUE_VERTALIGN_CENTER:
        case SID_GLUE_VERTALIGN_TOP:
        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );
            FuEditGluePoints* pFunc = dynamic_cast< FuEditGluePoints* >( xFunc.get() );

            if(pFunc)
                pFunc->ReceiveRequest(rReq);

            rReq.Done();
        }
        break;

#ifdef DBG_UTIL
        case SID_SHOW_ITEMBROWSER:
        {
            mpDrawView->ShowItemBrowser( !mpDrawView->IsItemBrowserVisible() );
            rReq.Done ();
        }
        break;
#endif

        case SID_AUTOSPELL_CHECK:
        {
            bool bOnlineSpell = !GetDoc()->GetOnlineSpell();
            GetDoc()->SetOnlineSpell(bOnlineSpell);

            ::Outliner* pOL = mpDrawView->GetTextEditOutliner();

            if (pOL)
            {
                EEControlBits nCntrl = pOL->GetControlWord();

                if (bOnlineSpell)
                    nCntrl |= EEControlBits::ONLINESPELLING;
                else
                    nCntrl &= ~EEControlBits::ONLINESPELLING;

                pOL->SetControlWord(nCntrl);
            }

            GetActiveWindow()->Invalidate();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_1BIT_THRESHOLD:
        case SID_CONVERT_TO_1BIT_MATRIX:
        case SID_CONVERT_TO_4BIT_GRAYS:
        case SID_CONVERT_TO_4BIT_COLORS:
        case SID_CONVERT_TO_8BIT_GRAYS:
        case SID_CONVERT_TO_8BIT_COLORS:
        case SID_CONVERT_TO_24BIT:
        {
            BmpConversion eBmpConvert = BMP_CONVERSION_NONE;

            switch( nSId )
            {
                case SID_CONVERT_TO_1BIT_THRESHOLD:
                    eBmpConvert = BMP_CONVERSION_1BIT_THRESHOLD;
                    break;

                case SID_CONVERT_TO_1BIT_MATRIX:
                    eBmpConvert = BMP_CONVERSION_1BIT_MATRIX;
                    break;

                case SID_CONVERT_TO_4BIT_GRAYS:
                    eBmpConvert = BMP_CONVERSION_4BIT_GREYS;
                    break;

                case SID_CONVERT_TO_4BIT_COLORS:
                    eBmpConvert = BMP_CONVERSION_4BIT_COLORS;
                    break;

                case SID_CONVERT_TO_8BIT_GRAYS:
                    eBmpConvert = BMP_CONVERSION_8BIT_GREYS;
                    break;

                case SID_CONVERT_TO_8BIT_COLORS:
                    eBmpConvert = BMP_CONVERSION_8BIT_COLORS;
                    break;

                case SID_CONVERT_TO_24BIT:
                    eBmpConvert = BMP_CONVERSION_24BIT;
                    break;
            }

            mpDrawView->BegUndo(SD_RESSTR(STR_UNDO_COLORRESOLUTION));
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

            for (size_t i=0; i<rMarkList.GetMarkCount(); ++i)
            {
                SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                if (pObj->GetObjInventor() == SdrInventor)
                {
                    if (pObj->GetObjIdentifier() == OBJ_GRAF && !static_cast<SdrGrafObj*>(pObj)->IsLinkedGraphic())
                    {
                        const Graphic& rGraphic = static_cast<SdrGrafObj*>(pObj)->GetGraphic();

                        if( rGraphic.GetType() == GRAPHIC_BITMAP )
                        {
                            SdrGrafObj* pNewObj = static_cast<SdrGrafObj*>( pObj->Clone() );

                            if( rGraphic.IsAnimated() )
                            {
                                Animation aAnim( rGraphic.GetAnimation() );
                                aAnim.Convert( eBmpConvert );
                                pNewObj->SetGraphic( aAnim );
                            }
                            else
                            {
                                BitmapEx aBmpEx( rGraphic.GetBitmapEx() );
                                aBmpEx.Convert( eBmpConvert );
                                pNewObj->SetGraphic( aBmpEx );
                            }

                            mpDrawView->ReplaceObjectAtView( pObj, *mpDrawView->GetSdrPageView(), pNewObj );
                        }
                    }
                }
            }

            mpDrawView->EndUndo();
            rReq.Done ();
        }
        break;

        case SID_TRANSLITERATE_SENTENCE_CASE:
        case SID_TRANSLITERATE_TITLE_CASE:
        case SID_TRANSLITERATE_TOGGLE_CASE:
        case SID_TRANSLITERATE_UPPER:
        case SID_TRANSLITERATE_LOWER:
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            OutlinerView* pOLV = GetView()->GetTextEditOutlinerView();
            if( pOLV )
            {
                using namespace ::com::sun::star::i18n;
                sal_Int32 nType = 0;

                switch( nSId )
                {
                    case SID_TRANSLITERATE_SENTENCE_CASE:
                        nType = TransliterationModulesExtra::SENTENCE_CASE;
                        break;
                    case SID_TRANSLITERATE_TITLE_CASE:
                        nType = TransliterationModulesExtra::TITLE_CASE;
                        break;
                    case SID_TRANSLITERATE_TOGGLE_CASE:
                        nType = TransliterationModulesExtra::TOGGLE_CASE;
                        break;
                    case SID_TRANSLITERATE_UPPER:
                        nType = TransliterationModules_LOWERCASE_UPPERCASE;
                        break;
                    case SID_TRANSLITERATE_LOWER:
                        nType = TransliterationModules_UPPERCASE_LOWERCASE;
                        break;
                    case SID_TRANSLITERATE_HALFWIDTH:
                        nType = TransliterationModules_FULLWIDTH_HALFWIDTH;
                        break;
                    case SID_TRANSLITERATE_FULLWIDTH:
                        nType = TransliterationModules_HALFWIDTH_FULLWIDTH;
                        break;
                    case SID_TRANSLITERATE_HIRAGANA:
                        nType = TransliterationModules_KATAKANA_HIRAGANA;
                        break;
                    case SID_TRANSLITERATE_KATAGANA:
                        nType = TransliterationModules_HIRAGANA_KATAKANA;
                        break;
                }

                pOLV->TransliterateText( nType );
            }

            rReq.Done();
        }
        break;

        // #UndoRedo#
        case SID_UNDO :
        {
            // moved implementation to BaseClass
            ImpSidUndo(true, rReq);
        }
        break;
        case SID_REDO :
        {
            // moved implementation to BaseClass
            ImpSidRedo(true, rReq);
        }
        break;

        default:
        break;
    }
}

void DrawViewShell::FuSupportRotate(SfxRequest &rReq)
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
    {
        ::sd::View* pView = GetView();

        if (!pView)
            return;

        OutlinerView* pOLV = pView->GetTextEditOutlinerView();

        if (!pOLV)
            return;

        pOLV->TransliterateText( m_aRotateCase.getNextMode() );
    }
}

void DrawViewShell::InsertURLField(const OUString& rURL, const OUString& rText,
                                   const OUString& rTarget, const Point* pPos)
{
    OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

    if (pOLV)
    {
        ESelection aSel( pOLV->GetSelection() );
        SvxFieldItem aURLItem( SvxURLField( rURL, rText, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD );
        pOLV->InsertField( aURLItem );
        if ( aSel.nStartPos <= aSel.nEndPos )
            aSel.nEndPos = aSel.nStartPos + 1;
        else
            aSel.nStartPos = aSel.nEndPos + 1;
        pOLV->SetSelection( aSel );
    }
    else
    {
        Outliner* pOutl = GetDoc()->GetInternalOutliner();
        pOutl->Init( OUTLINERMODE_TEXTOBJECT );
        sal_uInt16 nOutlMode = pOutl->GetMode();

        SvxURLField aURLField(rURL, rText, SVXURLFORMAT_REPR);
        aURLField.SetTargetFrame(rTarget);
        SvxFieldItem aURLItem(aURLField, EE_FEATURE_FIELD);
        pOutl->QuickInsertField( aURLItem, ESelection() );
        OutlinerParaObject* pOutlParaObject = pOutl->CreateParaObject();

        SdrRectObj* pRectObj = new SdrRectObj(OBJ_TEXT);

        pOutl->UpdateFields();
        pOutl->SetUpdateMode( true );
        Size aSize(pOutl->CalcTextSize());
        pOutl->SetUpdateMode( false );

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            Rectangle aRect(aPos, GetActiveWindow()->GetOutputSizePixel() );
            aPos = aRect.Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
            aPos.X() -= aSize.Width() / 2;
            aPos.Y() -= aSize.Height() / 2;
        }

        Rectangle aLogicRect(aPos, aSize);
        pRectObj->SetLogicRect(aLogicRect);
        pRectObj->SetOutlinerParaObject( pOutlParaObject );
        mpDrawView->InsertObjectAtView(pRectObj, *mpDrawView->GetSdrPageView());
        pOutl->Init( nOutlMode );
    }
}

void DrawViewShell::InsertURLButton(const OUString& rURL, const OUString& rText,
                                    const OUString& rTarget, const Point* pPos)
{
    bool bNewObj = true;

    const OUString sTargetURL( ::URIHelper::SmartRel2Abs( INetURLObject( GetDocSh()->GetMedium()->GetBaseURL() ), rURL, URIHelper::GetMaybeFileHdl(), true, false,
                                                                INetURLObject::WAS_ENCODED,
                                                                INetURLObject::DECODE_UNAMBIGUOUS ) );
    if (mpDrawView->GetMarkedObjectList().GetMarkCount() > 0)
    {
        SdrObject* pMarkedObj = mpDrawView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
        if( pMarkedObj ) try
        {
            // change first marked object
            if( (FmFormInventor == pMarkedObj->GetObjInventor() && pMarkedObj->GetObjIdentifier() == OBJ_FM_BUTTON) )
            {
                bNewObj = false;

                SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pMarkedObj );

                Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), UNO_QUERY_THROW );
                Reference< beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY_THROW );

                xPropSet->setPropertyValue("Label" , Any( rText ) );
                xPropSet->setPropertyValue("TargetURL" , Any( sTargetURL ) );

                if( !rTarget.isEmpty() )
                    xPropSet->setPropertyValue("TargetFrame" , Any( rTarget ) );

                xPropSet->setPropertyValue( "ButtonType" , Any( form::FormButtonType_URL ) );
                if ( ::avmedia::MediaWindow::isMediaURL( rURL, ""/*TODO?*/ ) )
                {
                    xPropSet->setPropertyValue( "DispatchURLInternal" , Any( sal_True ) );
                }
            }
            else
            {
                // add url as interaction for first selected shape
                bNewObj = false;

                SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pMarkedObj, true);
                pInfo->meClickAction = presentation::ClickAction_DOCUMENT;
                pInfo->SetBookmark( sTargetURL );
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    if (bNewObj) try
    {
        SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                                mpDrawView->GetSdrPageView()->GetPage(), GetDoc()) );

        Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( "Label" , Any( OUString( rText ) ) );
        xPropSet->setPropertyValue( "TargetURL" , Any( sTargetURL ) );

        if( !rTarget.isEmpty() )
            xPropSet->setPropertyValue( "TargetFrame" , Any( rTarget ) );

        xPropSet->setPropertyValue( "ButtonType" , Any(  form::FormButtonType_URL ) );
        if ( ::avmedia::MediaWindow::isMediaURL( rURL, ""/*TODO?*/ ) )
            xPropSet->setPropertyValue( "DispatchURLInternal" , Any( sal_True ) );

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            aPos = Rectangle(aPos, GetActiveWindow()->GetOutputSizePixel()).Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
        }

        Size aSize(4000, 1000);
        aPos.X() -= aSize.Width() / 2;
        aPos.Y() -= aSize.Height() / 2;
        pUnoCtrl->SetLogicRect(Rectangle(aPos, aSize));

        SdrInsertFlags nOptions = SdrInsertFlags::SETDEFLAYER;

        OSL_ASSERT (GetViewShell()!=nullptr);
        SfxInPlaceClient* pIpClient = GetViewShell()->GetIPClient();
        if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
        {
            nOptions |= SdrInsertFlags::DONTMARK;
        }

        mpDrawView->InsertObjectAtView(pUnoCtrl, *mpDrawView->GetSdrPageView(), nOptions);
    }
    catch( Exception& )
    {
    }
}

void DrawViewShell::ShowUIControls (bool bVisible)
{
    ViewShell::ShowUIControls (bVisible);
    maTabControl->Show (bVisible);
}

namespace slideshowhelp
{
    void ShowSlideShow(SfxRequest& rReq, SdDrawDocument &rDoc)
    {
        Reference< XPresentation2 > xPresentation( rDoc.getPresentation() );
        if( xPresentation.is() )
        {
            if (SID_REHEARSE_TIMINGS == rReq.GetSlot())
                xPresentation->rehearseTimings();
            else if (rDoc.getPresentationSettings().mbCustomShow)
            {
                //fdo#69975 if a custom show has been set, then
                //use it whether or not we've been asked to
                //start from the current or first slide
                xPresentation->start();
            }
            else if (SID_PRESENTATION_CURRENT_SLIDE == rReq.GetSlot())
            {
                //If there is no custom show set, start will automatically
                //start at the current page
                xPresentation->start();
            }
            else
            {
                //Start at page 0, this would blow away any custom
                //show settings if any were set
                Sequence< PropertyValue > aArguments(1);
                PropertyValue aPage;

                aPage.Name = "FirstPage";
                aPage.Value <<= OUString("0");

                aArguments[0] = aPage;

                xPresentation->startWithArguments( aArguments );
            }
        }
    }
}

void DrawViewShell::ShowSlideShow(SfxRequest& rReq)
{
    slideshowhelp::ShowSlideShow(rReq, *GetDoc());
}

void DrawViewShell::StopSlideShow (bool /*bCloseFrame*/)
{
    Reference< XPresentation2 > xPresentation( GetDoc()->getPresentation() );
    if(xPresentation.is() && xPresentation->isRunning())
    {
        if( mpDrawView->IsTextEdit() )
            mpDrawView->SdrEndTextEdit();

        xPresentation->end();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
