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
#include <memory>

class XLineAttrSetItem;
class XFillAttrSetItem;
class SdrEdgeObj;
class SdrObjConnection;

class ImplConnectMarkerOverlay;
class ImpSdrCreateViewExtraData;

class SVX_DLLPUBLIC SdrCreateView : public SdrDragView
{
    friend class                SdrPageView;

protected:
    SdrObject*                  pCurrentCreate;   // The currently being created object
    SdrPageView*                pCreatePV;    // Here, the creation is started
    std::unique_ptr<ImplConnectMarkerOverlay> mpCoMaOverlay;

    // for migrating stuff from XOR, use ImpSdrCreateViewExtraData ATM to not need to
    // compile the apps all the time
    std::unique_ptr<ImpSdrCreateViewExtraData> mpCreateViewExtraData;

    PointerStyle                aCurrentCreatePointer;

    sal_Int32                   nAutoCloseDistPix;
    sal_Int32                   nFreeHandMinDistPix;
    SdrInventor                 nCurrentInvent;     // set the current ones
    sal_uInt16                  nCurrentIdent;      // Obj for re-creating

    bool                        b1stPointAsCenter : 1;
    bool                        bUseIncompatiblePathCreateInterface : 1;

    void ImpClearConnectMarker();

private:
    SVX_DLLPRIVATE void ImpClearVars();

protected:
    bool ImpBegCreateObj(SdrInventor nInvent, sal_uInt16 nIdent, const Point& rPnt, OutputDevice* pOut,
        sal_Int16 nMinMov, const tools::Rectangle& rLogRect, SdrObject* pPreparedFactoryObject);

    void ShowCreateObj(/*OutputDevice* pOut, bool bFull*/);
    void HideCreateObj(/*OutputDevice* pOut, bool bFull*/);
    bool CheckEdgeMode();

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~SdrCreateView() override;

public:
    virtual bool IsAction() const override;
    virtual void MovAction(const Point& rPnt) override;
    virtual void EndAction() override;
    virtual void BckAction() override;
    virtual void BrkAction() override;
    virtual void TakeActionRect(tools::Rectangle& rRect) const override;

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

    void SetCurrentObj(sal_uInt16 nIdent, SdrInventor nInvent=SdrInventor::Default);
    void TakeCurrentObj(sal_uInt16& nIdent, SdrInventor& nInvent) const  { nInvent=nCurrentInvent; nIdent=nCurrentIdent; }
    SdrInventor GetCurrentObjInventor() const { return nCurrentInvent; }
    sal_uInt16  GetCurrentObjIdentifier() const { return nCurrentIdent; }

    // Beginning the regular Create
    bool BegCreateObj(const Point& rPnt, OutputDevice* pOut=nullptr, short nMinMov=-3);
    bool BegCreatePreparedObject(const Point& rPnt, sal_Int16 nMinMov, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const Point& rPnt);
    bool EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // go back one polygon point
    void BrkCreateObj();
    bool IsCreateObj() const { return pCurrentCreate!=nullptr; }
    SdrObject* GetCreateObj() const { return pCurrentCreate; }

    /// Setup layer (eg. foreground / background) of the given object.
    static void SetupObjLayer(const SdrPageView* pPageView, const OUString& aActiveLayer, SdrObject* pObj);

    // BegCreateCaptionObj() creates a SdrCaptionObj (legend item).
    // rObjSiz is the initial size of the legend text frame.
    // Only the length of the tip is dragged
    bool BegCreateCaptionObj(const Point& rPnt, const Size& rObjSiz, OutputDevice* pOut=nullptr, short nMinMov=-3);

    // Create a circle/rectangle/text frame with the first Point being
    // the center of the object instead of the upper-left corner.
    // Persistent flag. Default = FALSE.
    bool IsCreate1stPointAsCenter() const { return b1stPointAsCenter; }
    void SetCreate1stPointAsCenter(bool bOn) { b1stPointAsCenter = bOn; }

    // Default = 5 Pixel
    sal_uInt16 GetAutoCloseDistPix() const { return sal_uInt16(nAutoCloseDistPix); }

    // Setting for the minimum distance in pixels between 2 bezier points when
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
    void SetConnectMarker(const SdrObjConnection& rCon);
    void HideConnectMarker();

    // Attributes of the object that is in the process of being created
    /* new interface src537 */
    void GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const; // SfxStyleSheet* GetStyleSheet(bool& rOk) const;
    void SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);
};

#endif // INCLUDED_SVX_SVDCRTV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
