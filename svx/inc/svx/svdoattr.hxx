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



#ifndef _SVDOATTR_HXX
#define _SVDOATTR_HXX

#include <svx/xfillit0.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnasit.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdattr.hxx>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxPoolItem;
class SfxSetItem;
class SdrOutliner;
class SfxItemSet;
class SfxItemPool;

//************************************************************
//   SdrAttrObj
//************************************************************

class SVX_DLLPUBLIC SdrAttrObj : public SdrObject
{
private:
    friend class                SdrOutliner;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    Rectangle                   maSnapRect;

protected:
    // Strichstaerke ermitteln. Keine Linie -> 0.
    sal_Int32 ImpGetLineWdt() const;

    // Zuhoeren, ob sich ein StyleSheet aendert
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    SdrAttrObj();
    virtual ~SdrAttrObj();

public:
    TYPEINFO();

    // Feststellen, ob bFilledObj && Fuellung!=FillNone
    sal_Bool HasFill() const;

    // Feststellen, ob Linie!=LineNone
    sal_Bool HasLine() const;

    virtual const Rectangle& GetSnapRect() const;

    virtual void SetModel(SdrModel* pNewModel);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOATTR_HXX

