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



#ifndef _SVDTEXT_HXX
#define _SVDTEXT_HXX

#include <sal/types.h>
#include "svx/svxdllapi.h"
#include <tools/weakbase.hxx>

// --------------------------------------------------------------------

class OutlinerParaObject;
class SdrOutliner;
class SdrTextObj;
class SdrModel;
class SfxItemSet;

namespace sdr { namespace properties {
    class TextProperties;
}}

/** This class stores information about one text inside a shape.
*/

class SVX_DLLPUBLIC SdrText : public tools::WeakBase< SdrText >
{
public:
    SdrText( SdrTextObj& rObject, OutlinerParaObject* pOutlinerParaObject = 0 );
    virtual ~SdrText();

    virtual void ForceOutlinerParaObject( sal_uInt16 nOutlMode );
    virtual void SetOutlinerParaObject( OutlinerParaObject* pTextObject );
    virtual OutlinerParaObject* GetOutlinerParaObject() const;

    virtual void CheckPortionInfo( SdrOutliner& rOutliner );
    virtual void ReformatText();

    // default uses GetObjectItemSet, but may be overloaded to
    // return a text-specific ItemSet
    virtual const SfxItemSet& GetItemSet() const;
    SdrTextObj& getSdrTextObj() const { return mrObject; }

    /** returns the current OutlinerParaObject and removes it from this instance */
    OutlinerParaObject* RemoveOutlinerParaObject();

    // support model change, e.g. when text object is cloned to a new SdrModel
    void ImpModelChange(SdrModel& rSourceModel, SdrModel& rTargetModel);

protected:
    virtual const SfxItemSet& GetObjectItemSet();
    virtual void SetObjectItem(const SfxPoolItem& rItem);
    virtual SfxStyleSheet* GetStyleSheet() const;

private:
    OutlinerParaObject* mpOutlinerParaObject;
    SdrTextObj& mrObject;

    /// bitfield
    bool                    mbPortionInfoChecked : 1;
};

#endif //_SVDTEXT_HXX

