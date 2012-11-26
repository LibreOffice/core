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
#include "precompiled_svx.hxx"

#include <svx/svdoattr.hxx>
#include <svx/xpool.hxx>
#include <editeng/editdata.hxx>
#include <svx/svditext.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdomeas.hxx>
#include <svl/smplhint.hxx>
#include <svl/itemiter.hxx>
#include <svx/xenum.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xtextit0.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xftshit.hxx>
#include <editeng/colritem.hxx>
#include "editeng/fontitem.hxx"
#include <editeng/fhgtitem.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnwtit.hxx>
#include <svl/style.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlnedcit.hxx>
#include <editeng/adjitem.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xtable.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/sdr/properties/attributeproperties.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include "svx/xlinjoit.hxx"

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SdrAttrObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::AttributeProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrAttrObj::SdrAttrObj(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform)
:   SdrObject(rSdrModel, rTransform)
{
}

SdrAttrObj::~SdrAttrObj()
{
}

void __EXPORT SdrAttrObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    const SfxSimpleHint *pSimple = dynamic_cast< const SfxSimpleHint* >(&rHint);
    bool bDataChg(pSimple && SFX_HINT_DATACHANGED == pSimple->GetId());

    if(bDataChg)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this, HINT_OBJCHG_ATTR);
        SetChanged();
    }
}

sal_Int32 SdrAttrObj::ImpGetLineWdt() const
{
    sal_Int32 nRetval(0);

    if(XLINE_NONE != ((XLineStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE))).GetValue())
    {
        nRetval = ((XLineWidthItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINEWIDTH))).GetValue();
    }

    return nRetval;
}

bool SdrAttrObj::HasFill() const
{
    return IsClosedObj() && XFILL_NONE != ((XFillStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_FILLSTYLE))).GetValue();
}

bool SdrAttrObj::HasLine() const
{
    return XLINE_NONE != ((XLineStyleItem&)(GetProperties().GetObjectItemSet().Get(XATTR_LINESTYLE))).GetValue();
}

//////////////////////////////////////////////////////////////////////////////
// eof
