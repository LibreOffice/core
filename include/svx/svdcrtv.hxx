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

#ifndef _SVDCRTV_HXX
#define _SVDCRTV_HXX

#include <svx/svddrgv.hxx>
#include "svx/svxdllapi.h"

class XLineAttrSetItem;
class XFillAttrSetItem;
class SdrEdgeObj;
class SdrObjConnection;

class ImplConnectMarkerOverlay;
class ImpSdrCreateViewExtraData;

class SVX_DLLPUBLIC SdrCreateView: public SdrDragView
{
    friend class                SdrPageView;

protected:
    SdrObject*                  pAktCreate;   // Currently in creation of the located object
    SdrPageView*                pCreatePV;    // Here, the creation is started
    ImplConnectMarkerOverlay*   mpCoMaOverlay;

    // for migrating stuff from XOR, use ImpSdrCreateViewExtraData ATM to not need to
    // compile the apps all the time
    ImpSdrCreateViewExtraData*  mpCreateViewExtraData;

    Pointer                     aAktCreatePointer;

    sal_Int32                       nAutoCloseDistPix;
    sal_Int32                       nFreeHandMinDistPix;
    sal_uInt32                      nAktInvent;     // set the current ones
    sal_uInt16                      nAktIdent;      // Obj for re-creating

    unsigned                    bAutoTextEdit : 1; // Textedit after we start the creation of a text frame
    unsigned                    b1stPointAsCenter : 1;
    unsigned                    bUseIncompatiblePathCreateInterface : 1;
    unsigned                    bAutoClosePolys : 1;

    void ImpClearConnectMarker();

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    sal_Bool ImpBegCreateObj(sal_uInt32 nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
        sal_Int16 nMinMov, SdrPageView* pPV, const Rectangle& rLogRect, SdrObject* pPreparedFactoryObject);

    void ShowCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/);
    void HideCreateObj(/*OutputDevice* pOut, sal_Bool bFull*/);
    sal_Bool CheckEdgeMode();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrCreateView();

public:
    virtual sal_Bool IsAction() const;
    virtual void MovAction(const Point& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual void TakeActionRect(Rectangle& rRect) const;

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt, Window* pWin);

    void SetMeasureLayer(const OUString& rName) { aMeasureLayer=rName; }
    const OUString& GetMeasureLayer() const { return aMeasureLayer; }

    // If the MeasureLayer is not set (empty string), then use the active layer for measuring.
    void SetEditMode(SdrViewEditMode eMode) { SdrDragView::SetEditMode(eMode); CheckEdgeMode(); }
    void SetEditMode(sal_Bool bOn=sal_True) { SdrDragView::SetEditMode(bOn); CheckEdgeMode(); }
    void SetCreateMode(sal_Bool bOn=sal_True) { SdrDragView::SetCreateMode(bOn); CheckEdgeMode(); }
    void SetGluePointEditMode(sal_Bool bOn=sal_True) { SdrDragView::SetGluePointEditMode(bOn); CheckEdgeMode(); }

    // Determine whether a text tool is activated
    bool IsTextTool() const;

    // Determine whether an object connector tool activated
    bool IsEdgeTool() const;

    // Determine whether a measurement tool activated
    bool IsMeasureTool() const;

    void SetCurrentObj(sal_uInt16 nIdent, sal_uInt32 nInvent=SdrInventor);
    void TakeCurrentObj(sal_uInt16& nIdent, sal_uInt32& nInvent) const  { nInvent=nAktInvent; nIdent=nAktIdent; }
    sal_uInt32 GetCurrentObjInventor() const { return nAktInvent; }
    sal_uInt16 GetCurrentObjIdentifier() const { return nAktIdent; }

    // Beginning the regular Create
    sal_Bool BegCreateObj(const Point& rPnt, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);
    sal_Bool BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const Point& rPnt);
    sal_Bool EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // go back one polygon point
    void BrkCreateObj();
    sal_Bool IsCreateObj() const { return pAktCreate!=NULL; }
    SdrObject* GetCreateObj() const { return pAktCreate; }

    // BegCreateCaptionObj() creates a SdrCaptionObj (legend item).
    // rObjSiz is the initial size of the legend text frame.
    // Only the length of the tip is dragged
    sal_Bool BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz, OutputDevice* pOut=NULL, short nMinMov=-3, SdrPageView* pPV=NULL);

    // If TextEditAfterCreate is sal_True (the default),
    // then after the creation of a TextFrame object (OBJ_TEXT,
    // OBJ_TEXTEXT, OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatically start a TextEdit (SdrObjEditView: SdrBeginTextEdit)
    sal_Bool IsTextEditAfterCreate() const { return bAutoTextEdit; }
    void SetTextEditAfterCreate(sal_Bool bOn) { bAutoTextEdit = bOn; }

    // Create a circle/rectangle/text frame with the first Point being
    // the center of the object instead of the upper-left corner.
    // Persistent flag. Default = FALSE.
    sal_Bool IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(sal_Bool bOn) { b1stPointAsCenter = bOn; }

    // For polylines (OBJ_PLIN) and freehand lines (OBJ_FREELINE). If this
    // Flag is sal_True, these two types of objects are implicitly closed, and
    // converted to Polygon (OBJ_POLY) or freehand fill (OBJ_FREEFILL) if
    // the distance between the start point and end point of the
    // Object <= nAutoCloseDistPix pixels.
    // Default = TRUE.
    sal_Bool IsAutoClosePolys() const { return bAutoClosePolys; }
    void SetAutoClosePolys(sal_Bool bOn) { bAutoClosePolys=bOn; }

    // Default = 5 Pixel
    sal_uInt16 GetAutoCloseDistPix() const { return sal_uInt16(nAutoCloseDistPix); }
    void SetAutoCloseDistPix(sal_uInt16 nVal) { nAutoCloseDistPix=nVal; }

    // Setting for the minimum distantce in pixels between 2 bezier points when
    // creating a freehand line.
    // Default = 10 Pixel
    sal_uInt16 GetFreeHandMinDistPix() const { return sal_uInt16(nFreeHandMinDistPix); }
    void SetFreeHandMinDistPix(sal_uInt16 nVal) { nFreeHandMinDistPix=nVal; }

    // FIXME: Whoever wants to keep the Create Interface for the PathObj which is
    // incompatible with the rest of the Create functionality of SvDraw, needs
    // to set the following flag. It affects the following object types:
    // OBJ_POLY, OBJ_PLIN, OBJ_PATHLINE, OBJ_PATHFILL
    //
    // This flag should be regarded as temporary. The affected applications should
    // be changed soon.
    // Default = sal_False;
    sal_Bool IsUseIncompatiblePathCreateInterface() const { return bUseIncompatiblePathCreateInterface; }
    void SetUseIncompatiblePathCreateInterface(sal_Bool bOn) { bUseIncompatiblePathCreateInterface = bOn; }
    void SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& rPV);
    void HideConnectMarker();

    // Attributes of the object that is in the process of being created
    /* new interface src537 */
    sal_Bool GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr=sal_False) const;

    sal_Bool SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(sal_Bool& rOk) const;
    sal_Bool SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr);
};

#endif //_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
