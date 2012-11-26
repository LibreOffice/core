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
#include "precompiled_sd.hxx"

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>

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
#include "slideshow.hxx"
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
#include <svx/svdlegacy.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace sd {

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

/*************************************************************************
|*
|* Permanente Funktionen
|*
\************************************************************************/

void ImpAddPrintableCharactersToTextEdit(SfxRequest& rReq, ::sd::View* pView)
{
    // #98198# evtl. feed characters to activated textedit
    const SfxItemSet* pSet = rReq.GetArgs();

    if(pSet)
    {
        String aInputString;

        if(SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR))
            aInputString = ((SfxStringItem&)pSet->Get(SID_ATTR_CHAR)).GetValue();

        if(aInputString.Len())
        {
            OutlinerView* pOLV = pView->GetTextEditOutlinerView();

            if(pOLV)
            {
                for(sal_uInt16 a(0); a < aInputString.Len(); a++)
                {
                    sal_Char aChar = (sal_Char)aInputString.GetChar(a);
                    KeyCode aKeyCode;
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
    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!

    if (SlideShow::IsRunning(GetViewShellBase()))
        return;

    sal_uInt16 nSId = rReq.GetSlot();

    if( HasCurrentFunction() &&
        ( nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
          nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL ) )
    {
        FunctionReference xFunc( GetCurrentFunction() );

        FuText* pFuText = dynamic_cast< FuText* >( xFunc.get() );

        if( pFuText )
        {
            pFuText->SetPermanent(true);
            xFunc->ReceiveRequest( rReq );

            MapSlot( nSId );

            Invalidate();

            Invalidate();

            // #98198# evtl. feed characters to activated textedit
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
                SetOldFunction(0);
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

        SetCurrentFunction(0);

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate(nOldSId);
        rBind.Update(nOldSId);
    }

    // Slot wird gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

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

            // #98198# evtl. feed characters to activated textedit
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

        // #98721#
        case SID_FM_CREATE_FIELDCONTROL:
        {
            SFX_REQUEST_ARG( rReq, pDescriptorItem, SfxUnoAnyItem, SID_FM_DATACCESS_DESCRIPTOR );
            DBG_ASSERT( pDescriptorItem, "DrawViewShell::FuPermanent(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );

            if(pDescriptorItem)
            {
                // get the form view
                FmFormView* pFormView = dynamic_cast< FmFormView* >(mpDrawView);
                ::svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
                SdrObject* pNewDBField = pFormView->CreateFieldControl(aDescriptor);

                if(pNewDBField)
                {
                    const basegfx::B2DRange aVisArea(GetActiveWindow()->GetLogicRange());
                    const basegfx::B2DVector aObjSize(sdr::legacy::GetLogicRange(*pNewDBField).getRange());
                    const basegfx::B2DPoint aObjPos(aVisArea.getCenter() + (aObjSize * 0.5));
                    const basegfx::B2DRange aNewObjectRange(aObjPos, aObjPos + aObjSize);

                    sdr::legacy::SetLogicRange(*pNewDBField, aNewObjectRange);
                    GetView()->InsertObjectAtView(*pNewDBField);
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
                // togle rotation
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
                if ( mpDrawView->areSdrObjectsSelected() &&
                    !mpDrawView->IsCrookAllowed( mpDrawView->IsCrookNoContortion() ) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        ::sd::Window* pWindow = GetActiveWindow();
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(GetActiveWindow(), WB_YES_NO,
                                      String(SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER) )
                                      ).Execute() == RET_YES )
                    {
                        // Implizite Wandlung in Bezier
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        mpDrawView->ConvertMarkedToPathObj(false);
                    }
                }
            }
            else if (nSlotId == SID_OBJECT_SHEAR)
            {
                const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
                bool b3DObjMarked = false;

                for(sal_uInt32 a(0); !b3DObjMarked && a < aSelection.size(); a++)
                {
                    if (dynamic_cast< E3dObject* >(aSelection[a]))
                    {
                        b3DObjMarked = true;
                    }
                }

                if( aSelection.size() && !b3DObjMarked && (!mpDrawView->IsShearAllowed() || !mpDrawView->IsDistortAllowed()) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        ::sd::Window* pWindow = GetActiveWindow();
                        InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
                    }
                    else if ( QueryBox(GetActiveWindow(), WB_YES_NO,
                                      String(SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER) )
                                      ).Execute() == RET_YES )
                    {
                        // Implizite Wandlung in Bezier
                        WaitObject aWait( (Window*)GetActiveWindow() );
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
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
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

        default:
           break;
    }

    if(HasOldFunction())
    {
        sal_uInt16 nSlotId = GetOldFunction()->GetSlotID();

        GetOldFunction()->Deactivate();
        SetOldFunction(0);

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

    // Shell wird invalidiert, schneller als einzeln (laut MI)
    // Jetzt explizit der letzte Slot incl. Update()
    Invalidate();

    // #97016# III CTRL-SID_OBJECT_SELECT -> select first draw object if none is selected yet
    if(SID_OBJECT_SELECT == nSId && HasCurrentFunction() && (rReq.GetModifier() & KEY_MOD1))
    {
        if(!GetView()->areSdrObjectsSelected())
        {
            // select first object
            GetView()->UnmarkAllObj();
            GetView()->MarkNextObj(true);

            // ...and make it visible
            if(GetView()->areSdrObjectsSelected())
            {
                GetView()->MakeVisibleAtView(GetView()->getMarkedObjectSnapRange(), *GetActiveWindow());
            }
        }
    }

    // #97016# with qualifier construct directly
    if(HasCurrentFunction() && (rReq.GetModifier() & KEY_MOD1))
    {
        // get SdOptions
        SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());
        const sal_uInt32 nDefaultObjectSizeWidth(pOptions->GetDefaultObjectSizeWidth());
        const sal_uInt32 nDefaultObjectSizeHeight(pOptions->GetDefaultObjectSizeHeight());

        // calc position and size
        const basegfx::B2DRange aVisArea(GetActiveWindow()->GetLogicRange());
        const basegfx::B2DVector aObjSize(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight);
        const basegfx::B2DPoint aObjPos(aVisArea.getCenter() - (aObjSize * 0.5));
        const basegfx::B2DRange aNewObjectRange(aObjPos, aObjPos + aObjSize);

            // create the default object
        SdrObject* pObj = GetCurrentFunction()->CreateDefaultObject(nSId, aNewObjectRange);

        if(pObj)
        {
            // insert into page
            GetView()->InsertObjectAtView(*pObj);

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
                        Execute(SID_TEXTEDIT, SFX_CALLMODE_SYNCHRON |
                            SFX_CALLMODE_RECORD, &aItem, 0L);
                    // Put text object into edit mode.
                    GetView()->SdrBeginTextEdit(static_cast<SdrTextObj*>(pObj));
                    break;
                }
            }
        }
    }
}

/*************************************************************************
|*
|* SfxRequests fuer Support-Funktionen
|*
\************************************************************************/

void DrawViewShell::FuSupport(SfxRequest& rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(((SfxUInt16Item&)rReq.GetArgs()->Get( SID_STYLE_FAMILY )).GetValue());

    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
    if(SlideShow::IsRunning(GetViewShellBase()) &&
        (rReq.GetSlot() != SID_PRESENTATION_END &&
         rReq.GetSlot() != SID_SIZE_PAGE))
        return;

    CheckLineTo (rReq);

    if( !mpDrawView )
        return;

    sal_uInt16 nSId = rReq.GetSlot();

    // Slot wird evtl. gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

    switch ( nSId )
    {
        case SID_CLEAR_UNDO_STACK:
        {
            GetDocSh()->ClearUndoBuffer();
            rReq.Ignore ();
        }
        break;

        // Slots der ToolboxController gemapped ausfuehren
        case SID_OBJECT_CHOOSE_MODE:
        case SID_POSITION:
        case SID_OBJECT_ALIGN:
        case SID_ZOOM_TOOLBOX:
        case SID_DRAWTBX_TEXT:
        case SID_DRAWTBX_RECTANGLES:
        case SID_DRAWTBX_ELLIPSES:
        case SID_DRAWTBX_LINES:
        case SID_DRAWTBX_ARROWS:
        case SID_DRAWTBX_3D_OBJECTS:
        case SID_DRAWTBX_CONNECTORS:
        case SID_DRAWTBX_INSERT:
        {
            sal_uInt16 nMappedSlot = GetMappedSlot( nSId );
            if( nMappedSlot > 0 )
            {
                SfxRequest aReq( nMappedSlot, 0, GetDoc()->GetItemPool() );
                ExecuteSlot( aReq );
            }
        }
        break;

        case SID_PRESENTATION:
        case SID_REHEARSE_TIMINGS:
        {
            Reference< XPresentation2 > xPresentation( GetDoc()->getPresentation() );
            if( xPresentation.is() )
            {
                if( ( SID_REHEARSE_TIMINGS != rReq.GetSlot() ) )
                    xPresentation->start();
                else
                    xPresentation->rehearseTimings();
            }
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
            mpDrawView->SetFrameHandles(!mpDrawView->IsFrameHandles());

            /******************************************************************
            * ObjectBar einschalten
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
            SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >(mpDrawView->getSelectedIfSingle());

            if(pPathObj)
            {
                const bool bUndo = mpDrawView->IsUndoEnabled();
                if( bUndo )
                    mpDrawView->BegUndo(String(SdResId(STR_UNDO_BEZCLOSE)));

                mpDrawView->MarkPoints(0, true); // unmarkall

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
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
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
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
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
            WaitObject aWait( (Window*)GetActiveWindow() );

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

        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            WaitObject              aWait( (Window*)GetActiveWindow() );
            TransferableDataHelper  aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            const SfxItemSet*       pReqArgs = rReq.GetArgs();
            sal_uInt32                  nFormat = 0;

            if( pReqArgs )
            {
                SFX_REQUEST_ARG( rReq, pIsActive, SfxUInt32Item, SID_CLIPBOARD_FORMAT_ITEMS );
                nFormat = pIsActive->GetValue();
            }


            if( nFormat && aDataHelper.GetTransferable().is() )
            {
                sal_Int8 nAction = DND_ACTION_COPY;
                const basegfx::B2DPoint aPos(GetActiveWindow()->GetLogicRange().getCenter());

                if( !mpDrawView->InsertData( aDataHelper,
                                          aPos,
                                          nAction, false, nFormat ) )
                {
                    INetBookmark    aINetBookmark( aEmptyStr, aEmptyStr );

                    if( ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField( aINetBookmark.GetURL(), aINetBookmark.GetDescription(), aEmptyStr, 0);
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
                    KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode);
                    pOLV->PostKeyEvent(aKEvt);
                }
            }
            else if ( mpDrawView->IsPresObjSelected(false, true, false, true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                KeyCode aKCode(KEY_DELETE);
                KeyEvent aKEvt( 0, aKCode);

                bool bConsumed = mpDrawView && mpDrawView->getSmartTags().KeyInput( aKEvt );

                if( !bConsumed && HasCurrentFunction() )
                    bConsumed = GetCurrentFunction()->KeyInput(aKEvt);

                if( !bConsumed && mpDrawView )
                    mpDrawView->DeleteMarked();
            }
            rReq.Ignore ();
        }
        break;

        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
            // AutoLayouts have to be ready.
            GetDoc()->StopWorkStartupDelay();
            // Fall through to following case statements.

        case SID_DRAWINGMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
            // Let the sub-shell manager handle the slot handling.
            framework::FrameworkHelper::Instance(GetViewShellBase())->HandleModeChangeSlot(
                nSId,
                rReq);
            rReq.Ignore ();
            break;

        case SID_MASTERPAGE:          // BASIC
        case SID_SLIDE_MASTERPAGE:    // BASIC
        case SID_TITLE_MASTERPAGE:    // BASIC
        case SID_NOTES_MASTERPAGE:    // BASIC
        case SID_HANDOUT_MASTERPAGE:  // BASIC
        {
            // AutoLayouts muessen fertig sein
            GetDoc()->StopWorkStartupDelay();

            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_MASTERPAGE );
                mbIsLayerModeActive = pIsActive->GetValue ();
            }

            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

            if (nSId == SID_MASTERPAGE                                       ||
                (nSId == SID_SLIDE_MASTERPAGE   && mePageKind == PK_STANDARD) ||
                (nSId == SID_TITLE_MASTERPAGE   && mePageKind == PK_STANDARD) ||
                (nSId == SID_NOTES_MASTERPAGE   && mePageKind == PK_NOTES)    ||
                (nSId == SID_HANDOUT_MASTERPAGE && mePageKind == PK_HANDOUT))
            {
                if (nSId == SID_TITLE_MASTERPAGE ||
                    nSId == SID_SLIDE_MASTERPAGE)
                {
                    // Gibt es eine Seite mit dem AutoLayout "Titel"?
                    bool bFound = false;
                    sal_uInt32 i = 0;
                    sal_uInt32 nCount = GetDoc()->GetSdPageCount(PK_STANDARD);

                    while (i < nCount && !bFound)
                    {
                        SdPage* pPage = GetDoc()->GetSdPage(i, PK_STANDARD);

                        if (nSId == SID_TITLE_MASTERPAGE && pPage->GetAutoLayout() == AUTOLAYOUT_TITLE)
                        {
                            bFound = true;
                            SwitchPage((pPage->GetPageNumber() - 1) / 2);
                        }
                        else if (nSId == SID_SLIDE_MASTERPAGE && pPage->GetAutoLayout() != AUTOLAYOUT_TITLE)
                        {
                            bFound = true;
                            SwitchPage((pPage->GetPageNumber() - 1) / 2);
                        }

                        i++;
                    }
                }

                // Default-Layer der MasterPage einschalten
                mpDrawView->SetActiveLayer( String( SdResId(STR_LAYER_BCKGRNDOBJ) ) );

                ChangeEditMode(EM_MASTERPAGE, mbIsLayerModeActive);

                if(HasCurrentFunction(SID_BEZIER_EDIT))
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }
            else
            {
                // Switch to requested ViewShell.
                ::OUString sRequestedView;
                PageKind ePageKind;
                switch (nSId)
                {
                    case SID_SLIDE_MASTERPAGE:
                    case SID_TITLE_MASTERPAGE:
                    default:
                        sRequestedView = framework::FrameworkHelper::msImpressViewURL;
                        ePageKind = PK_STANDARD;
                        break;

                    case SID_NOTES_MASTERPAGE:
                        sRequestedView = framework::FrameworkHelper::msNotesViewURL;
                        ePageKind = PK_NOTES;
                        break;

                    case SID_HANDOUT_MASTERPAGE:
                        sRequestedView = framework::FrameworkHelper::msHandoutViewURL;
                        ePageKind = PK_HANDOUT;
                        break;
                }

                mpFrameView->SetViewShEditMode(EM_MASTERPAGE, ePageKind);
                mpFrameView->SetLayerMode(mbIsLayerModeActive);
                framework::FrameworkHelper::Instance(GetViewShellBase())->RequestView(
                    sRequestedView,
                    framework::FrameworkHelper::msCenterPaneURL);
            }
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
                    SFX_CALLMODE_ASYNCHRON);
            }


            rReq.Done();
        }
        break;


        case SID_RULER:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            // #97516# Remember old ruler state
            bool bOldHasRuler(HasRuler());

            if ( pReqArgs )
            {
                SFX_REQUEST_ARG (rReq, pIsActive, SfxBoolItem, SID_RULER );
                SetRuler (pIsActive->GetValue ());
            }
            else SetRuler (!HasRuler());

            // #97516# Did ruler state change? Tell that to SdOptions, too.
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
                basegfx::B2DPoint aPagePos(0.0, 0.0);
                basegfx::B2DVector aPageSize(pPageView->getSdrPageFromSdrPageView().GetPageScale());

                aPagePos.setX(aPagePos.getX() + (aPageSize.getX() * 0.5));
                aPageSize.setX(aPageSize.getX() * 1.03);

                if( rReq.GetSlot() == SID_SIZE_PAGE )
                {
                    aPagePos.setY(aPagePos.getY() + (aPageSize.getY() * 0.5));
                    aPageSize.setY(aPageSize.getY() * 1.03);
                    aPagePos.setY(aPagePos.getY() - (aPageSize.getY() * 0.5));
                }
                else
                {
                    const basegfx::B2DPoint aPoint(GetActiveWindow()->GetInverseViewTransformation() *
                        basegfx::B2DPoint(0.0, GetActiveWindow()->GetSizePixel().Height() / 2));

                    aPagePos.setY(aPagePos.getY() + aPoint.getY());
                    aPageSize.setY(2.0);
                }

                aPagePos.setX(aPagePos.getX() - (aPageSize.getX() * 0.5));

                SetZoomRange( basegfx::B2DRange( aPagePos, aPagePos + aPageSize ) );

                const basegfx::B2DRange aVisAreaWin(GetActiveWindow()->GetLogicRange());
                mpZoomList->InsertZoomRange(aVisAreaWin);
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_SIZE_REAL:  // BASIC
        {
            mbZoomOnPage = false;
            SetZoom( 100 );
            const basegfx::B2DRange aVisAreaWin(GetActiveWindow()->GetLogicRange());
            mpZoomList->InsertZoomRange(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_IN:  // BASIC
        {
            mbZoomOnPage = false;
            SetZoom( Max( (long) ( GetActiveWindow()->GetZoom() / 2 ), (long) GetActiveWindow()->GetMinZoom() ) );
            const basegfx::B2DRange aVisAreaWin(GetActiveWindow()->GetLogicRange());
            mpZoomList->InsertZoomRange(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        case SID_SIZE_VISAREA:
        {
            const Rectangle aVisArea(mpFrameView->GetVisArea());
            const Size aVisAreaSize(aVisArea.GetSize());

            if (aVisAreaSize.Height()!=0 && aVisAreaSize.Width()!=0)
            {
                mbZoomOnPage = false;
                const basegfx::B2DRange aVisRange(aVisArea.Left(), aVisArea.Top(), aVisArea.Right(), aVisArea.Bottom());
                SetZoomRange(aVisRange);
                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
                Invalidate( SID_ZOOM_TOOLBOX );
            }
            rReq.Done ();
        }
        break;

        // Namensverwirrung: SID_SIZE_OPTIMAL -> Zoom auf selektierte Objekte
        // --> Wird als Objektzoom im Programm angeboten
        case SID_SIZE_OPTIMAL:  // BASIC
        {
            mbZoomOnPage = false;
            if ( mpDrawView->areSdrObjectsSelected() )
            {
                maMarkRange = mpDrawView->getMarkedObjectSnapRange();
                const basegfx::B2DVector aNewScale(maMarkRange.getRange() * 1.03);
                basegfx::B2DPoint aPos(maMarkRange.getCenter());

                if(!aNewScale.equalZero())
                {
                    aPos -= aNewScale * 0.5;

                    SetZoomRange(basegfx::B2DRange(aPos, aPos + aNewScale));

                    const basegfx::B2DRange aVisAreaWin(GetActiveWindow()->GetLogicRange());
                    mpZoomList->InsertZoomRange(aVisAreaWin);
                }
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            Invalidate( SID_ZOOM_TOOLBOX );
            rReq.Done ();
        }
        break;

        // Namensverwirrung: SID_SIZE_ALL -> Zoom auf alle Objekte
        // --> Wird als Optimal im Programm angeboten
        case SID_SIZE_ALL:  // BASIC
        {
            mbZoomOnPage = false;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( pPageView )
            {
                const basegfx::B2DRange aBoundRange(sdr::legacy::GetAllObjBoundRange(pPageView->GetCurrentObjectList()->getSdrObjectVector()));
                const basegfx::B2DVector aScale(aBoundRange.getRange() * 1.03);

                if(!aScale.equalZero())
                {
                    const basegfx::B2DPoint aPos(aBoundRange.getCenter() - (aScale * 0.5));

                    SetZoomRange(basegfx::B2DRange(aPos, aPos + aScale));

                    const basegfx::B2DRange aVisAreaWin(GetActiveWindow()->GetLogicRange());
                    mpZoomList->InsertZoomRange(aVisAreaWin);
                }

                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
                Invalidate( SID_ZOOM_TOOLBOX );
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
                // Vorheriges ZoomRect einstellen
                SetZoomRange(mpZoomList->GetPreviousZoomRange());
            }
            rReq.Done ();
            Invalidate( SID_ZOOM_TOOLBOX );
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
                // Naechstes ZoomRect einstellen
                SetZoomRange(mpZoomList->GetNextZoomRange());
            }
            rReq.Done ();
            Invalidate( SID_ZOOM_TOOLBOX );
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
            FunctionReference xFunc( GetCurrentFunction() );
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
                sal_uLong nCntrl = pOL->GetControlWord();

                if (bOnlineSpell)
                    nCntrl |= EE_CNTRL_ONLINESPELLING;
                else
                    nCntrl &= ~EE_CNTRL_ONLINESPELLING;

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

            mpDrawView->BegUndo(String(SdResId(STR_UNDO_COLORRESOLUTION)));
            const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());

            for(sal_uInt32 a(0); a < aSelection.size(); a++)
            {
                SdrObject* pObj = aSelection[a];

                if (pObj->GetObjInventor() == SdrInventor)
                {
                    if (pObj->GetObjIdentifier() == OBJ_GRAF && !((SdrGrafObj*) pObj)->IsLinkedGraphic())
                    {
                        const Graphic& rGraphic = ((SdrGrafObj*) pObj)->GetGraphic();

                        if( rGraphic.GetType() == GRAPHIC_BITMAP )
                        {
                            SdrGrafObj* pNewObj = (SdrGrafObj*) pObj->CloneSdrObject();

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

                            mpDrawView->ReplaceObjectAtView( *pObj, *pNewObj );
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
            // #96090# moved implementation to BaseClass
            ImpSidUndo(true, rReq);
        }
        break;
        case SID_REDO :
        {
            // #96090# moved implementation to BaseClass
            ImpSidRedo(true, rReq);
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* URL-Feld einfuegen
|*
\************************************************************************/

void DrawViewShell::InsertURLField(const String& rURL, const String& rText, const String& rTarget, const basegfx::B2DPoint* pPos)
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

        pOutl->UpdateFields();
        pOutl->SetUpdateMode( true );
        const Size aOldSize(pOutl->CalcTextSize());
        pOutl->SetUpdateMode( false );

        // originally when pPos it was taken as TopLeft of new object, not
        // as center; I guess this was an error, all other inserters use center
        const basegfx::B2DPoint aPos(pPos ? *pPos : GetActiveWindow()->GetLogicRange().getCenter());
        const basegfx::B2DVector aSize(aOldSize.Width(), aOldSize.Height());
        const basegfx::B2DHomMatrix aObjTrans(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aSize,
                aPos - (aSize * 0.5)));
        SdrRectObj* pRectObj = new SdrRectObj(
            *GetDoc(),
            aObjTrans,
            OBJ_TEXT,
            true);

        pRectObj->SetOutlinerParaObject( pOutlParaObject );
        mpDrawView->InsertObjectAtView(*pRectObj);
        pOutl->Init( nOutlMode );
    }
}

/*************************************************************************
|*
|* URL-Button einfuegen
|*
\************************************************************************/

void DrawViewShell::InsertURLButton(const String& rURL, const String& rText, const String& rTarget, const basegfx::B2DPoint* pPos)
{
    bool bNewObj = true;
    const OUString sTargetURL( ::URIHelper::SmartRel2Abs( INetURLObject( GetDocSh()->GetMedium()->GetBaseURL() ), rURL, URIHelper::GetMaybeFileHdl(), true, false,
                                                                INetURLObject::WAS_ENCODED,
                                                                INetURLObject::DECODE_UNAMBIGUOUS ) );

    SdrObject* pMarkedObj = mpDrawView->getSelectedIfSingle();

    if( pMarkedObj ) try
    {
        // change first marked object
        if( (FmFormInventor == pMarkedObj->GetObjInventor() && pMarkedObj->GetObjIdentifier() == OBJ_FM_BUTTON) )
        {
            bNewObj = false;
            SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pMarkedObj );

            Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), UNO_QUERY_THROW );
            Reference< beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY_THROW );

            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )), Any( OUString( rText ) ) );
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )), Any( sTargetURL ) );

            if( rTarget.Len() )
                xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )), Any( OUString( rTarget ) ) );

            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )), Any( form::FormButtonType_URL ) );
            if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
            {
                // #105638# OJ
                xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), Any( sal_True ) );
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

    if (bNewObj) try
    {
        SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >(SdrObjFactory::MakeNewObject(
            *GetDoc(),
            SdrObjectCreationInfo(OBJ_FM_BUTTON, FmFormInventor)));

        Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), uno::UNO_QUERY_THROW );
        Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )), Any( OUString( rText ) ) );
        xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )), Any( sTargetURL ) );

        if( rTarget.Len() )
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )), Any( OUString( rTarget ) ) );

        xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )), Any(  form::FormButtonType_URL ) );
        // #105638# OJ
        if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), Any( sal_True ) );

        const basegfx::B2DPoint aPos(pPos ? *pPos : GetActiveWindow()->GetLogicRange().getCenter());
        const basegfx::B2DVector aSize(4000.0, 1000.0);
        const basegfx::B2DHomMatrix aObjTrans(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aSize,
                aPos - (aSize * 0.5)));
        sal_uLong nOptions(SDRINSERT_SETDEFLAYER);

        pUnoCtrl->setSdrObjectTransformation(aObjTrans);

        if(GetViewShell())
        {
            SfxInPlaceClient* pIpClient = GetViewShell()->GetIPClient();

            if(pIpClient && pIpClient->IsObjectInPlaceActive())
            {
                nOptions |= SDRINSERT_DONTMARK;
            }
        }
        else
        {
            OSL_ENSURE(false, "No ViewShell ?");
        }

        mpDrawView->InsertObjectAtView(*pUnoCtrl, nOptions);
    }
    catch( Exception& )
    {
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawViewShell::ShowUIControls (bool bVisible)
{
    ViewShell::ShowUIControls (bVisible);

    //AF: The LayerDialogChildWindow is not used anymore (I hope).
    //    GetViewFrame()->SetChildWindow(
    //        LayerDialogChildWindow::GetChildWindowId(),
    //        IsLayerModeActive() && bVisible);
    maTabControl.Show (bVisible);
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

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

} // end of namespace sd
