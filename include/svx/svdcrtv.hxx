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

#ifndef INCLUDED_SVX_SVDCRTV_HXX
#define INCLUDED_SVX_SVDCRTV_HXX

#include <svx/svddrgv.hxx>
#include <svx/svxdllapi.h>

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

    sal_Int32                   nAutoCloseDistPix;
    sal_Int32                   nFreeHandMinDistPix;
    sal_uInt32                  nAktInvent;     // set the current ones
    sal_uInt16                  nAktIdent;      // Obj for re-creating

    bool                        bAutoTextEdit : 1; // Textedit after we start the creation of a text frame
    bool                        b1stPointAsCenter : 1;
    bool                        bUseIncompatiblePathCreateInterface : 1;
    bool                        bAutoClosePolys : 1;

    void ImpClearConnectMarker();

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    bool ImpBegCreateObj(sal_uInt32 nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
        sal_Int16 nMinMov, SdrPageView* pPV, const Rectangle& rLogRect, SdrObject* pPreparedFactoryObject);

    void ShowCreateObj(/*OutputDevice* pOut, bool bFull*/);
    void HideCreateObj(/*OutputDevice* pOut, bool bFull*/);
    bool CheckEdgeMode();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(SdrModel* pModel1, OutputDevice* pOut = nullptr);
    virtual ~SdrCreateView();

public:
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BckAction() override;
    virtual void BrkAction() override;
    virtual void TakeActionRect(Rectangle& rRect) const override;

    virtual bool MouseMove(const MouseEvent& rMEvt, vcl::Window* pWin) override;

    void SetMeasureLayer(const OUString& rName) { maMeasureLayer=rName; }

    // If the MeasureLayer is not set (empty string), then use the active layer for measuring.
    void SetEditMode(SdrViewEditMode eMode) { SdrDragView::SetEditMode(eMode); CheckEdgeMode(); }
    void SetEditMode(bool bOn=true) { SdrDragView::SetEditMode(bOn); CheckEdgeMode(); }
    void SetCreateMode(bool bOn=true) { SdrDragView::SetCreateMode(bOn); CheckEdgeMode(); }
    void SetGluePointEditMode(bool bOn=true) { SdrDragView::SetGluePointEditMode(bOn); CheckEdgeMode(); }

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
    bool BegCreateObj(const Point& rPnt, OutputDevice* pOut=nullptr, short nMinMov=-3, SdrPageView* pPV=nullptr);
    bool BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const Point& rPnt);
    bool EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // go back one polygon point
    void BrkCreateObj();
    bool IsCreateObj() const { return pAktCreate!=nullptr; }
    SdrObject* GetCreateObj() const { return pAktCreate; }

    // BegCreateCaptionObj() creates a SdrCaptionObj (legend item).
    // rObjSiz is the initial size of the legend text frame.
    // Only the length of the tip is dragged
    bool BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz, OutputDevice* pOut=nullptr, short nMinMov=-3, SdrPageView* pPV=nullptr);

    // If TextEditAfterCreate is sal_True (the default),
    // then after the creation of a TextFrame object (OBJ_TEXT,
    // OBJ_TEXTEXT, OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatically start a TextEdit (SdrObjEditView: SdrBeginTextEdit)
    bool IsTextEditAfterCreate() const { return bAutoTextEdit; }

    // Create a circle/rectangle/text frame with the first Point being
    // the center of the object instead of the upper-left corner.
    // Persistent flag. Default = FALSE.
    bool IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(bool bOn) { b1stPointAsCenter = bOn; }

    // For polylines (OBJ_PLIN) and freehand lines (OBJ_FREELINE). If this
    // Flag is sal_True, these two types of objects are implicitly closed, and
    // converted to Polygon (OBJ_POLY) or freehand fill (OBJ_FREEFILL) if
    // the distance between the start point and end point of the
    // Object <= nAutoCloseDistPix pixels.
    // Default = TRUE.
    bool IsAutoClosePolys() const { return bAutoClosePolys; }

    // Default = 5 Pixel
    sal_uInt16 GetAutoCloseDistPix() const { return sal_uInt16(nAutoCloseDistPix); }

    // Setting for the minimum distantce in pixels between 2 bezier points when
    // creating a freehand line.
    // Default = 10 Pixel
    sal_uInt16 GetFreeHandMinDistPix() const { return sal_uInt16(nFreeHandMinDistPix); }

    // FIXME: Whoever wants to keep the Create Interface for the PathObj which is
    // incompatible with the rest of the Create functionality of SvDraw, needs
    // to set the following flag. It affects the following object types:
    // OBJ_POLY, OBJ_PLIN, OBJ_PATHLINE, OBJ_PATHFILL

    // This flag should be regarded as temporary. The affected applications should
    // be changed soon.
    // Default = sal_False;
    bool IsUseIncompatiblePathCreateInterface() const { return bUseIncompatiblePathCreateInterface; }
    void SetUseIncompatiblePathCreateInterface(bool bOn) { bUseIncompatiblePathCreateInterface = bOn; }
    void SetConnectMarker(const SdrObjConnection& rCon, const SdrPageView& rPV);
    void HideConnectMarker();

    // Attributes of the object that is in the process of being created
    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);
};

#endif // INCLUDED_SVX_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
