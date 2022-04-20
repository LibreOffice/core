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

#pragma once

#include <memory>
#include <svx/svdobj.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdpage.hxx>

// Forward declarations
class SfxItemSet;
class SdrObjGroup;

// Helper class to allow administer advanced Diagram related
// data and functionality
class SVXCORE_DLLPUBLIC IDiagramHelper
{
private:
    // These values define behaviour to where take data from at re-creation time.
    // Different definitions will have different consequences for re-creation
    // of Diagram visualization (if needed/triggered).
    // The style attributes per shape e.g. can be re-stored frm either an
    // existing Theme, or the preserved key/value list of properties per XShape.
    // With the current default settings the re-creation uses the preserved
    // key/value pairs, but re-creation from Theme may also be desirable. It
    // is also good to preserve both data packages at initial import to allow
    // alternatively one of these two methods for re-construction

    // If true, the oox::Theme data from ::DiagramData get/set/ThemeDocument()
    // aka mxThemeDocument - if it exists - will be used to create the style
    // attributes for the to-be-created XShapes (theoretically allows re-creation
    // with other Theme)
    bool mbUseDiagramThemeData; // false

    // If true, the UNO API form of attributes per Point as Key/value list
    // that was secured after initial XShape creation is used to create the
    // style attributes for the to-be-created XShapes
    bool mbUseDiagramModelData; // true

    // If true and mxThemeDocument exists it will be re-imported to
    // a newly created oox::drawingml::Theme object
    bool mbForceThemePtrRecreation; // false

protected:
    void anchorToSdrObjGroup(SdrObjGroup& rTarget);

public:
    IDiagramHelper();
    virtual ~IDiagramHelper();

    // re-create XShapes
    virtual void reLayout(SdrObjGroup& rTarget) = 0;

    // get text representation of data tree
    virtual OUString getString() const = 0;

    // get children of provided data node
    // use empty string for top-level nodes
    // returns vector of (id, text)
    virtual std::vector<std::pair<OUString, OUString>>
    getChildren(const OUString& rParentId) const = 0;

    // add/remove new top-level node to data model, returns its id
    virtual OUString addNode(const OUString& rText) = 0;
    virtual bool removeNode(const OUString& rNodeId) = 0;

    bool UseDiagramThemeData() const { return mbUseDiagramThemeData; }
    bool UseDiagramModelData() const { return mbUseDiagramModelData; }
    bool ForceThemePtrRecreation() const { return mbForceThemePtrRecreation; };
};

//   SdrObjGroup
class SVXCORE_DLLPUBLIC SdrObjGroup final : public SdrObject, public SdrObjList
{
private:
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties>
    CreateObjectSpecificProperties() override;

    Point maRefPoint; // Reference point inside the object group

    // Allow *only* DiagramHelper itself to set this internal reference to
    // tightly control usage
    friend class IDiagramHelper;
    std::shared_ptr<IDiagramHelper> mp_DiagramHelper;

public:
    bool isDiagram() const { return bool(mp_DiagramHelper); }
    const std::shared_ptr<IDiagramHelper>& getDiagramHelper() { return mp_DiagramHelper; }

private:
    // protected destructor - due to final, make private
    virtual ~SdrObjGroup() override;

public:
    SdrObjGroup(SdrModel& rSdrModel);
    // Copy constructor
    SdrObjGroup(SdrModel& rSdrModel, SdrObjGroup const& rSource);

    // derived from SdrObjList
    virtual SdrPage* getSdrPageFromSdrObjList() const override;
    virtual SdrObject* getSdrObjectFromSdrObjList() const override;

    // derived from SdrObject
    virtual SdrObjList* getChildrenOfSdrObject() const override;

    virtual void SetBoundRectDirty() override;
    virtual SdrObjKind GetObjIdentifier() const override;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrLayerID GetLayer() const override;
    virtual void NbcSetLayer(SdrLayerID nLayer) override;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    virtual SdrObjList* GetSubList() const override;
    virtual void SetGrabBagItem(const css::uno::Any& rVal) override;

    virtual const tools::Rectangle& GetCurrentBoundRect() const override;
    virtual const tools::Rectangle& GetSnapRect() const override;

    virtual SdrObjGroup* CloneSdrObject(SdrModel& rTargetModel) const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual void RecalcSnapRect() override;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    // special drag methods
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;

    virtual bool BegCreate(SdrDragStat& rStat) override;

    virtual Degree100 GetRotateAngle() const override;
    virtual Degree100 GetShearAngle(bool bVertical = false) const override;

    virtual void Move(const Size& rSiz) override;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact,
                        bool bUnsetRelative = true) override;
    virtual void Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs) override;
    virtual void Mirror(const Point& rRef1, const Point& rRef2) override;
    virtual void Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
    virtual void SetAnchorPos(const Point& rPnt) override;
    virtual void SetRelativePos(const Point& rPnt) override;
    virtual void SetSnapRect(const tools::Rectangle& rRect) override;
    virtual void SetLogicRect(const tools::Rectangle& rRect) override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact,
                           const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
    virtual void NbcSetAnchorPos(const Point& rPnt) override;
    virtual void NbcSetRelativePos(const Point& rPnt) override;
    virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;

    virtual void NbcReformatText() override;

    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
