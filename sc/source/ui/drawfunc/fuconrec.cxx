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

#include <fuconrec.hxx>
#include <tabvwsh.hxx>
#include <sc.hrc>
#include <drawview.hxx>

#include <editeng/outlobj.hxx>
// Create default drawing objects via keyboard
#include <svx/svdopath.hxx>
#include <svx/svdocapt.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <svx/xlnwtit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdomeas.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include <scresid.hxx>

FuConstRectangle::FuConstRectangle(ScTabViewShell* pViewSh, vcl::Window* pWin, ScDrawView* pViewP,
                   SdrModel* pDoc, const SfxRequest& rReq)
    : FuConstruct(pViewSh, pWin, pViewP, pDoc, rReq)
{
}

FuConstRectangle::~FuConstRectangle()
{
}

/**
 * set line starts and ends for the object to be created
 */

namespace {

::basegfx::B2DPolyPolygon getPolygon(const char* pResId, const SdrModel* pDoc)
{
    ::basegfx::B2DPolyPolygon aRetval;
    XLineEndListRef pLineEndList = pDoc->GetLineEndList();

    if( pLineEndList.is() )
    {
        OUString aArrowName( SvxResId(pResId) );
        long nCount = pLineEndList->Count();
        long nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nIndex);
            if( pEntry->GetName() == aArrowName )
            {
                aRetval = pEntry->GetLineEnd();
                break;
            }
        }
    }

    return aRetval;
}

}

bool FuConstRectangle::MouseButtonDown(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPos( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        // Hack  to align object to nearest grid position where object
        // would be anchored ( if it were cell anchored )
        // Get grid offset for current position ( note: aPnt is
        // also adjusted )
        Point aGridOff = CurrentGridSyncOffsetAndPos( aPos );
        pWindow->CaptureMouse();

        if ( pView->GetCurrentObjIdentifier() == OBJ_CAPTION )
        {
            Size aCaptionSize ( 2268, 1134 ); // 4x2cm

            bReturn = pView->BegCreateCaptionObj( aPos, aCaptionSize );

            // How do you set the font for writing
        }
        else
            bReturn = pView->BegCreateObj(aPos);

        SdrObject* pObj = pView->GetCreateObj();

        if (pObj)
        {
            SfxItemSet aAttr(pObj->GetModel()->GetItemPool());
            SetLineEnds(aAttr, pObj, aSfxRequest.GetSlot());
            pObj->SetMergedItemSet(aAttr);
        }

        if ( bReturn )
            pView->GetCreateObj()->SetGridOffset( aGridOff );
    }
    return bReturn;
}

bool FuConstRectangle::MouseButtonUp(const MouseEvent& rMEvt)
{
    // remember button state for creation of own MouseEvents
    SetMouseButtonCode(rMEvt.GetButtons());

    bool bReturn = false;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        pView->EndCreateObj(SdrCreateCmd::ForceEnd);

        if (aSfxRequest.GetSlot() == SID_DRAW_CAPTION_VERTICAL)
        {
            //  set vertical flag for caption object

            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
            if (rMarkList.GetMark(0))
            {
                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                //  create OutlinerParaObject now so it can be set to vertical
                if ( dynamic_cast<const SdrTextObj*>( pObj) !=  nullptr )
                    static_cast<SdrTextObj*>(pObj)->ForceOutlinerParaObject();
                OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
                if( pOPO && !pOPO->IsVertical() )
                    pOPO->SetVertical( true );
            }
        }

        bReturn = true;
    }
    return (FuConstruct::MouseButtonUp(rMEvt) || bReturn);
}

void FuConstRectangle::Activate()
{
    SdrObjKind aObjKind;

    switch (aSfxRequest.GetSlot() )
    {
        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_ARROW_START:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        case SID_LINE_ARROWS:
            aNewPointer = Pointer( PointerStyle::DrawLine );
            aObjKind = OBJ_LINE;
            break;

        case SID_DRAW_MEASURELINE:
            aNewPointer = Pointer( PointerStyle::DrawLine );
            aObjKind = OBJ_MEASURE;
            break;

        case SID_DRAW_RECT:
            aNewPointer = Pointer( PointerStyle::DrawRect );
            aObjKind = OBJ_RECT;
            break;

        case SID_DRAW_ELLIPSE:
            aNewPointer = Pointer( PointerStyle::DrawEllipse );
            aObjKind = OBJ_CIRC;
            break;

        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
            aNewPointer = Pointer( PointerStyle::DrawCaption );
            aObjKind = OBJ_CAPTION;
            break;

        default:
            aNewPointer = Pointer( PointerStyle::Cross );
            aObjKind = OBJ_RECT;
            break;
    }

    pView->SetCurrentObj(sal::static_int_cast<sal_uInt16>(aObjKind));

    aOldPointer = pWindow->GetPointer();
    pViewShell->SetActivePointer( aNewPointer );

    FuConstruct::Activate();
}

void FuConstRectangle::SetLineEnds(SfxItemSet& rAttr, const SdrObject* pObj, sal_uInt16 nSlotId)
{
    SdrModel *pDoc = pObj->GetModel();

    if ( nSlotId == SID_LINE_ARROW_START      ||
         nSlotId == SID_LINE_ARROW_END        ||
         nSlotId == SID_LINE_ARROWS           ||
         nSlotId == SID_LINE_ARROW_CIRCLE     ||
         nSlotId == SID_LINE_CIRCLE_ARROW     ||
         nSlotId == SID_LINE_ARROW_SQUARE     ||
         nSlotId == SID_LINE_SQUARE_ARROW )
    {

        // set attributes of line start and ends

        // arrowhead
        ::basegfx::B2DPolyPolygon aArrow( getPolygon( RID_SVXSTR_ARROW, pDoc ) );
        if( !aArrow.count() )
        {
            ::basegfx::B2DPolygon aNewArrow;
            aNewArrow.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewArrow.append(::basegfx::B2DPoint(0.0, 30.0));
            aNewArrow.append(::basegfx::B2DPoint(20.0, 30.0));
            aNewArrow.setClosed(true);
            aArrow.append(aNewArrow);
        }

        // Circles
        ::basegfx::B2DPolyPolygon aCircle( getPolygon( RID_SVXSTR_CIRCLE, pDoc ) );
        if( !aCircle.count() )
        {
            ::basegfx::B2DPolygon aNewCircle;
            aNewCircle = ::basegfx::utils::createPolygonFromEllipse(::basegfx::B2DPoint(0.0, 0.0), 250.0, 250.0);
            aNewCircle.setClosed(true);
            aCircle.append(aNewCircle);
        }

        // Square
        ::basegfx::B2DPolyPolygon aSquare( getPolygon( RID_SVXSTR_SQUARE, pDoc ) );
        if( !aSquare.count() )
        {
            ::basegfx::B2DPolygon aNewSquare;
            aNewSquare.append(::basegfx::B2DPoint(0.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 0.0));
            aNewSquare.append(::basegfx::B2DPoint(10.0, 10.0));
            aNewSquare.append(::basegfx::B2DPoint(0.0, 10.0));
            aNewSquare.setClosed(true);
            aSquare.append(aNewSquare);
        }

        SfxItemSet aSet( pDoc->GetItemPool() );
        long nWidth = 200; // (1/100th mm)

        // determine line width and calculate with it the line end width
        if( aSet.GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
        {
            long nValue = aSet.Get( XATTR_LINEWIDTH ).GetValue();
            if( nValue > 0 )
                nWidth = nValue * 3;
        }

        switch (nSlotId)
        {
            case SID_LINE_ARROWS:
            {
                // connector with arrow ends
                rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
                rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_START:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_ARROW_SQUARE:
            {
                // connector with arrow start
                rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_END:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_SQUARE_ARROW:
            {
                // connector with arrow end
                rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW), aArrow));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;
        }

        // and again, for the still missing ends
        switch (nSlotId)
        {
            case SID_LINE_ARROW_CIRCLE:
            {
                // circle end
                rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_CIRCLE_ARROW:
            {
                // circle start
                rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_CIRCLE), aCircle));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;

            case SID_LINE_ARROW_SQUARE:
            {
                // square end
                rAttr.Put(XLineEndItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineEndWidthItem(nWidth));
            }
            break;

            case SID_LINE_SQUARE_ARROW:
            {
                // square start
                rAttr.Put(XLineStartItem(SvxResId(RID_SVXSTR_SQUARE), aSquare));
                rAttr.Put(XLineStartWidthItem(nWidth));
            }
            break;
        }
    }
}

void FuConstRectangle::Deactivate()
{
    FuConstruct::Deactivate();
    pViewShell->SetActivePointer( aOldPointer );
}

// Create default drawing objects via keyboard
SdrObject* FuConstRectangle::CreateDefaultObject(const sal_uInt16 nID, const tools::Rectangle& rRectangle)
{
    SdrObject* pObj = SdrObjFactory::MakeNewObject(
        pView->GetCurrentObjInventor(), pView->GetCurrentObjIdentifier(),
        nullptr, pDrDoc);

    if(pObj)
    {
        tools::Rectangle aRect(rRectangle);
        Point aStart = aRect.TopLeft();
        Point aEnd = aRect.BottomRight();

        switch(nID)
        {
            case SID_DRAW_LINE:
            case SID_DRAW_XLINE:
            case SID_LINE_ARROW_END:
            case SID_LINE_ARROW_CIRCLE:
            case SID_LINE_ARROW_SQUARE:
            case SID_LINE_ARROW_START:
            case SID_LINE_CIRCLE_ARROW:
            case SID_LINE_SQUARE_ARROW:
            case SID_LINE_ARROWS:
            {
                if(dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr)
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    basegfx::B2DPolygon aPoly;
                    aPoly.append(basegfx::B2DPoint(aStart.X(), nYMiddle));
                    aPoly.append(basegfx::B2DPoint(aEnd.X(), nYMiddle));
                    static_cast<SdrPathObj*>(pObj)->SetPathPoly(basegfx::B2DPolyPolygon(aPoly));
                }
                else
                {
                    OSL_FAIL("Object is NO line object");
                }

                break;
            }

            case SID_DRAW_MEASURELINE:
            {
                if(dynamic_cast<const SdrMeasureObj*>( pObj) != nullptr)
                {
                    sal_Int32 nYMiddle((aRect.Top() + aRect.Bottom()) / 2);
                    static_cast<SdrMeasureObj*>(pObj)->SetPoint(Point(aStart.X(), nYMiddle), 0);
                    static_cast<SdrMeasureObj*>(pObj)->SetPoint(Point(aEnd.X(), nYMiddle), 1);
                }

                break;
            }

            case SID_DRAW_CAPTION:
            case SID_DRAW_CAPTION_VERTICAL:
            {
                if(dynamic_cast<const SdrCaptionObj*>( pObj) !=  nullptr)
                {
                    bool bIsVertical(SID_DRAW_CAPTION_VERTICAL == nID);

                    static_cast<SdrTextObj*>(pObj)->SetVerticalWriting(bIsVertical);

                    if(bIsVertical)
                    {
                        SfxItemSet aSet(pObj->GetMergedItemSet());
                        aSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
                        aSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                        pObj->SetMergedItemSet(aSet);
                    }

                    //  don't set default text, start edit mode instead
                    //  (Edit mode is started in ScTabViewShell::ExecDraw, because
                    //  it must be handled by FuText)

                    static_cast<SdrCaptionObj*>(pObj)->SetLogicRect(aRect);
                    static_cast<SdrCaptionObj*>(pObj)->SetTailPos(
                        aRect.TopLeft() - Point(aRect.GetWidth() / 2, aRect.GetHeight() / 2));
                }
                else
                {
                    OSL_FAIL("Object is NO caption object");
                }

                break;
            }

            default:
            {
                pObj->SetLogicRect(aRect);

                break;
            }
        }

        SfxItemSet aAttr(pDrDoc->GetItemPool());
        SetLineEnds(aAttr, pObj, nID);
        pObj->SetMergedItemSet(aAttr);
    }

    return pObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
