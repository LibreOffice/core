/*************************************************************************
 *
 *  $RCSfile: writerhelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:41:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif

#include <algorithm>        //std::swap
#include <functional>       //std::binary_function

#ifndef _DOC_HXX
#include <doc.hxx>          //SwDoc
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      //SwTable
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>       //SwFrmFmt
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>   //SdrObject
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx> //SdrOle2Obj
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>   //FmFormInventor
#endif
#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>  //SvxFmtBreakItem
#endif

namespace
{
    //Utility to sort SwTxtFmtColl's by their outline numbering level
    class outlinecmp : public
        std::binary_function<const SwTxtFmtColl*, const SwTxtFmtColl*, bool>
    {
    public:
        bool operator()(const SwTxtFmtColl *pA, const SwTxtFmtColl *pB) const
        {
            return pB->GetOutlineLevel() < pB->GetOutlineLevel();
        }
    };
}

namespace sw
{
    namespace hack
    {
        void SetLayer::SendObjectToHell(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHell);
        }

        void SetLayer::SendObjectToHeaven(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHeaven);
        }

        void SetLayer::SetObjectLayer(SdrObject &rObject, Layer eLayer) const
        {
            if (FmFormInventor == rObject.GetObjInventor())
                rObject.SetLayer(mnFormLayer);
            else
            {
                switch (eLayer)
                {
                    case eHeaven:
                        rObject.SetLayer(mnHeavenLayer);
                        break;
                    case eHell:
                        rObject.SetLayer(mnHellLayer);
                        break;
                }
            }
        }

        //SetLayer boilerplate begin
        void SetLayer::Swap(SetLayer& rOther) throw()
        {
            std::swap(mnHeavenLayer, rOther.mnHeavenLayer);
            std::swap(mnHellLayer, rOther.mnHellLayer);
            std::swap(mnFormLayer, rOther.mnFormLayer);
        }

        SetLayer::SetLayer(const SwDoc &rDoc)
            : mnHeavenLayer(rDoc.GetHeavenId()),
            mnHellLayer(rDoc.GetHellId()),
            mnFormLayer(rDoc.GetControlsId())
        {
        }

        SetLayer::SetLayer(const SetLayer& rOther) throw()
            : mnHeavenLayer(rOther.mnHeavenLayer),
            mnHellLayer(rOther.mnHellLayer),
            mnFormLayer(rOther.mnFormLayer)
        {
        }

        SetLayer& SetLayer::operator=(const SetLayer& rOther) throw()
        {
            SetLayer aTemp(rOther);
            Swap(aTemp);
            return *this;
        }
        //SetLayer boilerplate end


        DrawingOLEAdaptor::DrawingOLEAdaptor(SdrOle2Obj &rObj,
            SvPersist &rPers)
            : mxIPRef(rObj.GetObjRef()),
            msOrigPersistName(rObj.GetPersistName()), mrPers(rPers)
        {
            rObj.SetPersistName(String());
            rObj.SetObjRef(SvInPlaceObjectRef());
        }

        bool DrawingOLEAdaptor::TransferToDoc(const String &rName)
        {
            ASSERT(mxIPRef.Is(), "Transferring invalid object to doc");
            if (!mxIPRef.Is())
                return false;

            SvInfoObjectRef refObj = new SvEmbeddedInfoObject(mxIPRef, rName);
            bool bSuccess = mrPers.Move(refObj, rName);
            if (bSuccess)
            {
                SvPersist *pO = mxIPRef;
                ASSERT(!pO->IsModified(), "Not expected to be modified here");
                if (pO->IsModified())
                {
                    pO->DoSave();
                    pO->DoSaveCompleted();
                }
                mxIPRef.Clear();
                bSuccess = mrPers.Unload(pO);
            }

            return bSuccess;
        }

        DrawingOLEAdaptor::~DrawingOLEAdaptor()
        {
            if (mxIPRef.Is())
            {
                if (SvInfoObject* pInfo = mrPers.Find(msOrigPersistName))
                {
                    pInfo->SetDeleted(TRUE);
                    pInfo->SetObj(0);
                }
                mxIPRef->DoClose();
                mrPers.Remove(mxIPRef);
                mxIPRef.Clear();
            }
        }
    }

    namespace util
    {
        ParaStyles GetParaStyles(const SwDoc &rDoc)
        {
            ParaStyles aStyles;
            typedef ParaStyles::size_type mysizet;

            const SwTxtFmtColls *pColls = rDoc.GetTxtFmtColls();
            mysizet nCount = pColls ? pColls->Count() : 0;
            aStyles.reserve(nCount);
            for (mysizet nI = 0; nI < nCount; ++nI)
                aStyles.push_back((*pColls)[nI]);
            return aStyles;
        }

        void SortByOutline(ParaStyles &rStyles)
        {
            std::sort(rStyles.begin(), rStyles.end(), outlinecmp());
        }

        bool HasPageBreak(const SwNode &rNd)
        {
            const SvxFmtBreakItem *pBreak = 0;
            if (rNd.IsTableNode() && rNd.GetTableNode())
            {
                const SwTable& rTable = rNd.GetTableNode()->GetTable();
                const SwFrmFmt* pApply = rTable.GetFrmFmt();
                ASSERT(pApply, "impossible");
                if (pApply)
                    pBreak = &(ItemGet<SvxFmtBreakItem>(*pApply, RES_BREAK));
            }
            else if (const SwCntntNode *pNd = rNd.GetCntntNode())
                pBreak = &(ItemGet<SvxFmtBreakItem>(*pNd, RES_BREAK));

            if (pBreak && pBreak->GetBreak() == SVX_BREAK_PAGE_BEFORE)
                return true;
            return false;
        }

    }
}
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
