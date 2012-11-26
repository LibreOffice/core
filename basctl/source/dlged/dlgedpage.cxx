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
#include "precompiled_basctl.hxx"


#include "dlgedpage.hxx"
#include "dlged.hxx"
#include "dlgedmod.hxx"
#include "dlgedobj.hxx"


//----------------------------------------------------------------------------

DlgEdPage::DlgEdPage( DlgEdModel& rModel, bool bMasterPage )
    :SdrPage( rModel, bMasterPage )
{
}

//----------------------------------------------------------------------------

//DlgEdPage::DlgEdPage( const DlgEdPage& rPage )
//  :SdrPage( rPage )
//{
//  pDlgEdForm = rPage.pDlgEdForm;
//}

//----------------------------------------------------------------------------

DlgEdPage::~DlgEdPage()
{
    if(GetObjCount())
    {
        getSdrModelFromSdrObjList().SetChanged();
    }
}

void DlgEdPage::copyDataFromSdrPage(const SdrPage& rSource)
{
    if(this != &rSource)
    {
        const DlgEdPage* pSource = dynamic_cast< const DlgEdPage* >(&rSource);

        if(pSource)
{
            // call parent
            SdrPage::copyDataFromSdrPage(rSource);

            // copy local data
            pDlgEdForm = pSource->pDlgEdForm;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrPage* DlgEdPage::CloneSdrPage(SdrModel* pTargetModel) const
{
    DlgEdModel* pDlgEdModel = static_cast< DlgEdModel* >(pTargetModel ? pTargetModel : &getSdrModelFromSdrPage());
    OSL_ENSURE(dynamic_cast< DlgEdModel* >(pDlgEdModel), "Wrong SdrModel type in DlgEdPage cloner (!)");
    DlgEdPage* pClone = new DlgEdPage(
        *pDlgEdModel,
        IsMasterPage());
    OSL_ENSURE(pClone, "CloneSdrPage error (!)");
    pClone->copyDataFromSdrPage(*this);

    return pClone;
}

//----------------------------------------------------------------------------

SdrObject* DlgEdPage::SetNavigationPosition(sal_uInt32 nOldObjNum, sal_uInt32 nNewObjNum)
{
    SdrObject* pObj = SdrPage::SetNavigationPosition( nOldObjNum, nNewObjNum );

    DlgEdHint aHint( DLGED_HINT_OBJORDERCHANGED );
    if ( pDlgEdForm )
    {
        DlgEditor* pDlgEditor = pDlgEdForm->GetDlgEditor();
        if ( pDlgEditor )
            pDlgEditor->Broadcast( aHint );
    }

    return pObj;
}

//----------------------------------------------------------------------------
