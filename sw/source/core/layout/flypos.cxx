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
#include "precompiled_sw.hxx"

#include "doc.hxx"
#include "node.hxx"
#include <docary.hxx>
#include <fmtanchr.hxx>
#include "flypos.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include "dflyobj.hxx"
#include "ndindex.hxx"
#include "switerator.hxx"

bool SwPosFlyFrmCmp::operator()(const SwPosFlyFrmPtr& rA, const SwPosFlyFrmPtr& rB) const
{
    if(rA->GetNdIndex() == rB->GetNdIndex())
    {
        return rA->GetOrdNum() < rB->GetOrdNum();
    }

    return rA->GetNdIndex() < rB->GetNdIndex();
}

SwPosFlyFrm::SwPosFlyFrm( const SwNodeIndex& rIdx, const SwFrmFmt* pFmt,
                            sal_uInt16 nArrPos )
    : pFrmFmt( pFmt ), pNdIdx( (SwNodeIndex*) &rIdx )
{
    bool bFnd = false;
    const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
    if (FLY_AT_PAGE == rAnchor.GetAnchorId())
    {
        pNdIdx = new SwNodeIndex( rIdx );
    }
    else if( pFmt->GetDoc()->GetCurrentViewShell() )    //swmod 071108//swmod 071225
    {
        if( RES_FLYFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            SwFlyFrm* pFly = SwIterator<SwFlyFrm,SwFmt>::FirstElement(*pFmt);
            if( pFly )
            {
                nOrdNum = pFly->GetVirtDrawObj()->GetNavigationPosition();
                bFnd = true;
            }
        }
        else if( RES_DRAWFRMFMT == pFmt->Which() )
        {
            // Schauen, ob es ein SdrObject dafuer gibt
            SwDrawContact* pContact = SwIterator<SwDrawContact,SwFmt>::FirstElement(*pFmt);
            if( pContact )
            {
                nOrdNum = pContact->GetMaster()->GetNavigationPosition();
                bFnd = true;
            }
        }
    }

    if( !bFnd )
    {
        nOrdNum = pFmt->GetDoc()->GetSpzFrmFmts()->Count();
        nOrdNum += nArrPos;
    }
}

SwPosFlyFrm::~SwPosFlyFrm()
{
    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    if (FLY_AT_PAGE == rAnchor.GetAnchorId())
    {
        delete pNdIdx;
    }
}

// eof
