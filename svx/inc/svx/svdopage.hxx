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



#ifndef _SVDOPAGE_HXX
#define _SVDOPAGE_HXX

#include <svx/svdobj.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SdrPageObj : public SdrObject
{
private:
    // #111111#
    // To make things more safe, remember the page, not a number
    SdrPage*                                mpShownPage;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    ~SdrPageObj();

    // method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    SdrPageObj(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix(), SdrPage* pNewPage = 0);

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // derived from SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    // #111111#
    SdrPage* GetReferencedPage() const;
    void SetReferencedPage(SdrPage* pNewPage);

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOPAGE_HXX

