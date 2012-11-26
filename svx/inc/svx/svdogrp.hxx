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



#ifndef _SVDOGRP_HXX
#define _SVDOGRP_HXX

#include <tools/datetime.hxx>
#include <svx/svdobj.hxx>
#include "svx/svxdllapi.h"
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SdrObjListIter;
class SfxItemSet;

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjGroup
:   public SdrObject,
    public SdrObjList
{
private:
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    virtual ~SdrObjGroup();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    SdrObjGroup(SdrModel& rSdrModel);

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // derived from SdrObjList
    virtual SdrPage* getSdrPageFromSdrObjList() const;
    virtual SdrObject* getSdrObjectFromSdrObjList() const;
    virtual SdrModel& getSdrModelFromSdrObjList() const;
    virtual void handleContentChange(const SfxHint& rHint);

    // derived from SdrObject
    virtual SdrObjList* getChildrenOfSdrObject() const;

    // react on model change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual SdrLayerID GetLayer() const;
    virtual void SetLayer(SdrLayerID nLayer);
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
    virtual void SetAnchorPos(const basegfx::B2DPoint& rPnt);
    virtual void ReformatText();
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;
    virtual void getMergedHierarchyLayerSet(SetOfByte& rSet) const;
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDOGRP_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
