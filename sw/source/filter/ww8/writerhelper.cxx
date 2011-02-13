/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/util/XCloseable.hpp>

#include <doc.hxx>
#include "writerhelper.hxx"
#include <msfilter.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <algorithm>                //std::swap
#include <functional>               //std::binary_function
#include <svl/itemiter.hxx>  //SfxItemIter
#include <svx/svdobj.hxx>        //SdrObject
#include <svx/svdoole2.hxx>      //SdrOle2Obj
#include <svx/fmglob.hxx>        //FmFormInventor
#include <editeng/brkitem.hxx>       //SvxFmtBreakItem
#include <editeng/tstpitem.hxx>      //SvxTabStopItem
#include <ndtxt.hxx>             //SwTxtNode
#include <ndnotxt.hxx>          //SwNoTxtNode
#include <fmtcntnt.hxx>         //SwFmtCntnt
#include <swtable.hxx>          //SwTable
#include <frmfmt.hxx>           //SwFrmFmt
#include <flypos.hxx>           //SwPosFlyFrms
#include <fmtanchr.hxx>         //SwFmtAnchor
#include <ndgrf.hxx>            //SwGrfNode
#include <fmtfsize.hxx>         //SwFmtFrmSize
#include <SwStyleNameMapper.hxx> //SwStyleNameMapper
#include <docary.hxx>            //SwCharFmts
#include <charfmt.hxx>           //SwCharFmt
#include <fchrfmt.hxx>           //SwFmtCharFmt
#ifndef _UNOTOOLS_STREAMWRAP_HXX
#include <unotools/streamwrap.hxx>
#endif
#include <numrule.hxx>

using namespace com::sun::star;
using namespace nsSwGetPoolIdFromName;


namespace
{
    /*
     Stroustroup forgets copy_if, See C++ Programming language Chp 18, pg 530
    */
    template <typename In , typename Out , typename Pred>
        Out my_copy_if(In first, In last, Out res, Pred p)
    {
        while (first != last)
        {
            if (p(*first))
                *res = *first;
            ++first;
        }
        return res;
    }

    // #i98791# - adjust sorting
    // Utility to sort SwTxtFmtColl's by their assigned outline style list level
    class outlinecmp : public
        std::binary_function<const SwTxtFmtColl*, const SwTxtFmtColl*, bool>
    {
    public:
        bool operator()(const SwTxtFmtColl *pA, const SwTxtFmtColl *pB) const
        {
            // #i98791#
            bool bResult( false );
            const bool bIsAAssignedToOutlineStyle( pA->IsAssignedToListLevelOfOutlineStyle() );
            const bool bIsBAssignedToOutlineStyle( pB->IsAssignedToListLevelOfOutlineStyle() );
            if ( bIsAAssignedToOutlineStyle != bIsBAssignedToOutlineStyle )
            {
                bResult = bIsBAssignedToOutlineStyle;
            }
            else if ( !bIsAAssignedToOutlineStyle )
            {
                // pA and pB are equal regarding the sorting criteria.
                // Thus return value does not matter.
                bResult = false;
            }
            else
            {
                bResult = pA->GetAssignedOutlineStyleLevel() < pB->GetAssignedOutlineStyleLevel();
            }

            return bResult;
            // <--
       }
    };
    // <--

    bool IsValidSlotWhich(USHORT nSlotId, USHORT nWhichId)
    {
        return (nSlotId != 0 && nWhichId != 0 && nSlotId != nWhichId);
    }

    /*
     Utility to convert a SwPosFlyFrms into a simple vector of sw::Frames

     The crucial thing is that a sw::Frame always has an anchor which
     points to some content in the document. This is a requirement of exporting
     to Word
    */
    sw::Frames SwPosFlyFrmsToFrames(const SwPosFlyFrms &rFlys)
    {
        sw::Frames aRet;
        USHORT nEnd = rFlys.Count();
        for (USHORT nI = 0; nI < nEnd; ++nI)
        {
            const SwFrmFmt &rEntry = rFlys[nI]->GetFmt();
            if (const SwPosition* pAnchor = rEntry.GetAnchor().GetCntntAnchor())
                aRet.push_back(sw::Frame(rEntry, *pAnchor));
            else
            {
                SwPosition aPos(rFlys[nI]->GetNdIndex());
                if (SwTxtNode* pTxtNd = aPos.nNode.GetNode().GetTxtNode())
                    aPos.nContent.Assign(pTxtNd, 0);
                aRet.push_back(sw::Frame(rEntry, aPos));
            }
        }
        return aRet;
    }

    //Utility to test if a frame is anchored at a given node index
    class anchoredto: public std::unary_function<const sw::Frame&, bool>
    {
    private:
        ULONG mnNode;
    public:
        anchoredto(ULONG nNode) : mnNode(nNode) {}
        bool operator()(const sw::Frame &rFrame) const
        {
            return (mnNode == rFrame.GetPosition().nNode.GetNode().GetIndex());
        }
    };
}

namespace sw
{
    Frame::Frame(const SwFrmFmt &rFmt, const SwPosition &rPos)
        : mpFlyFrm(&rFmt),
          maPos(rPos),
          maSize(),
          maLayoutSize(), // #i43447#
          meWriterType(eTxtBox),
          mpStartFrameContent(0),
          // #i43447# - move to initialization list
          mbIsInline( (rFmt.GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
    {
        switch (rFmt.Which())
        {
            case RES_FLYFRMFMT:
                if (const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx())
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    const SwNode &rNd = aIdx.GetNode();
                    using sw::util::GetSwappedInSize;
                    // #i43447# - determine layout size
                    {
                        SwRect aLayRect( rFmt.FindLayoutRect() );
                        Rectangle aRect( aLayRect.SVRect() );
                        // The Object is not rendered (e.g. something in unused
                        // header/footer) - thus, get the values from the format.
                        if ( aLayRect.IsEmpty() )
                        {
                            aRect.SetSize( rFmt.GetFrmSize().GetSize() );
                        }
                        maLayoutSize = aRect.GetSize();
                    }
                    // <--
                    switch (rNd.GetNodeType())
                    {
                        case ND_GRFNODE:
                            meWriterType = eGraphic;
                            maSize = GetSwappedInSize(*rNd.GetNoTxtNode());
                            break;
                        case ND_OLENODE:
                            meWriterType = eOle;
                            maSize = GetSwappedInSize(*rNd.GetNoTxtNode());
                            break;
                        default:
                            meWriterType = eTxtBox;
                            // #i43447# - Size equals layout size for text boxes
                            maSize = maLayoutSize;
                            // <--
                            break;
                    }
                    mpStartFrameContent = &rNd;
                }
                else
                {
                    OSL_ENSURE(!this, "Impossible");
                    meWriterType = eTxtBox;
                }
                break;
            default:
                if (const SdrObject* pObj = rFmt.FindRealSdrObject())
                {
                    if (pObj->GetObjInventor() == FmFormInventor)
                        meWriterType = eFormControl;
                    else
                        meWriterType = eDrawing;
                    maSize = pObj->GetSnapRect().GetSize();
                }
                else
                {
                    OSL_ENSURE(!this, "Impossible");
                    meWriterType = eDrawing;
                }
                break;
        }
    }

    bool Frame::IsInline() const
    {
        return mbIsInline;
    }

    void Frame::ForceTreatAsInline()
    {
        mbIsInline = true;
    }

    namespace hack
    {

        USHORT TransformWhichBetweenPools(const SfxItemPool &rDestPool,
            const SfxItemPool &rSrcPool, USHORT nWhich)
        {
            USHORT nSlotId = rSrcPool.GetSlotId(nWhich);
            if (IsValidSlotWhich(nSlotId, nWhich))
                nWhich = rDestPool.GetWhich(nSlotId);
            else
                nWhich = 0;
            return nWhich;
        }

        USHORT GetSetWhichFromSwDocWhich(const SfxItemSet &rSet,
            const SwDoc &rDoc, USHORT nWhich)
        {
            if (RES_WHICHHINT_END < *(rSet.GetRanges()))
            {
                nWhich = TransformWhichBetweenPools(*rSet.GetPool(),
                    rDoc.GetAttrPool(), nWhich);
            }
            return nWhich;
        }

        DrawingOLEAdaptor::DrawingOLEAdaptor(SdrOle2Obj &rObj,
            SfxObjectShell &rPers)
            : msOrigPersistName(rObj.GetPersistName()),
            mxIPRef(rObj.GetObjRef()), mrPers(rPers),
            mpGraphic( rObj.GetGraphic() )
        {
            rObj.AbandonObject();
        }

        bool DrawingOLEAdaptor::TransferToDoc( ::rtl::OUString &rName )
        {
            OSL_ENSURE(mxIPRef.is(), "Transferring invalid object to doc");
            if (!mxIPRef.is())
                return false;

            uno::Reference < container::XChild > xChild( mxIPRef, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( mrPers.GetModel() );

            bool bSuccess = mrPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( mxIPRef, rName );
            if (bSuccess)
            {
                if ( mpGraphic )
                    ::svt::EmbeddedObjectRef::SetGraphicToContainer( *mpGraphic,
                                                                    mrPers.GetEmbeddedObjectContainer(),
                                                                    rName,
                                                                    ::rtl::OUString() );

                mxIPRef = 0;
            }

            return bSuccess;
        }

        DrawingOLEAdaptor::~DrawingOLEAdaptor()
        {
            if (mxIPRef.is())
            {
                DBG_ASSERT( !mrPers.GetEmbeddedObjectContainer().HasEmbeddedObject( mxIPRef ), "Object in adaptor is inserted?!" );
                try
                {
                    uno::Reference < com::sun::star::util::XCloseable > xClose( mxIPRef, uno::UNO_QUERY );
                    if ( xClose.is() )
                        xClose->close(sal_True);
                }
                catch ( com::sun::star::util::CloseVetoException& )
                {
                }

                mxIPRef = 0;
            }
        }
    }

    namespace util
    {
        SV_IMPL_OP_PTRARR_SORT(AuthorInfos, AuthorInfo_Ptr)

        SwTwips MakeSafePositioningValue(SwTwips nIn)
        {
            if (nIn > SHRT_MAX)
                nIn = SHRT_MAX;
            else if (nIn < SHRT_MIN)
                nIn = SHRT_MIN;
            return nIn;
        }

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

        // #i38889# - by default put objects into the invisible layers.
        SetLayer::SetLayer(const SwDoc &rDoc)
            : mnHeavenLayer(rDoc.GetInvisibleHeavenId()),
              mnHellLayer(rDoc.GetInvisibleHellId()),
              mnFormLayer(rDoc.GetInvisibleControlsId())
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

        void GetPoolItems(const SfxItemSet &rSet, PoolItems &rItems, bool bExportParentItemSet )
        {
            if( bExportParentItemSet )
            {
                USHORT nTotal = rSet.TotalCount();
                for( USHORT nItem =0; nItem < nTotal; ++nItem )
                {
                    const SfxPoolItem* pItem = 0;
                    if( SFX_ITEM_SET == rSet.GetItemState( rSet.GetWhichByPos( nItem ), true, &pItem ) )
                    {
                        rItems[pItem->Which()] = pItem;
                    }
                }
            }
            else if( rSet.Count())
            {
                SfxItemIter aIter(rSet);
                if (const SfxPoolItem *pItem = aIter.GetCurItem())
                {
                    do
                        rItems[pItem->Which()] = pItem;
                    while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
                }
            }
        }

        const SfxPoolItem *SearchPoolItems(const PoolItems &rItems,
            sal_uInt16 eType)
        {
            sw::cPoolItemIter aIter = rItems.find(eType);
            if (aIter != rItems.end())
                return aIter->second;
            return 0;
        }

        void ClearOverridesFromSet(const SwFmtCharFmt &rFmt, SfxItemSet &rSet)
        {
            if (const SwCharFmt* pCharFmt = rFmt.GetCharFmt())
            {
                if (pCharFmt->GetAttrSet().Count())
                {
                    SfxItemIter aIter(pCharFmt->GetAttrSet());
                    const SfxPoolItem *pItem = aIter.GetCurItem();
                    do
                        rSet.ClearItem(pItem->Which());
                    while (!aIter.IsAtEnd() && 0 != (pItem = aIter.NextItem()));
                }
            }
        }

        ParaStyles GetParaStyles(const SwDoc &rDoc)
        {
            ParaStyles aStyles;
            typedef ParaStyles::size_type mysizet;

            const SwTxtFmtColls *pColls = rDoc.GetTxtFmtColls();
            mysizet nCount = pColls ? pColls->Count() : 0;
            aStyles.reserve(nCount);
            for (mysizet nI = 0; nI < nCount; ++nI)
                aStyles.push_back((*pColls)[ static_cast< USHORT >(nI) ]);
            return aStyles;
        }

        SwTxtFmtColl* GetParaStyle(SwDoc &rDoc, const String& rName)
        {
            // Search first in the Doc-Styles
            SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName(rName);
            if (!pColl)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pColl = rDoc.GetTxtCollFromPool(n, false);
            }
            return pColl;
        }

        SwCharFmt* GetCharStyle(SwDoc &rDoc, const String& rName)
        {
            SwCharFmt *pFmt = rDoc.FindCharFmtByName(rName);
            if (!pFmt)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pFmt = rDoc.GetCharFmtFromPool(n);
            }
            return pFmt;
        }

        // #i98791# - adjust sorting algorithm
        void SortByAssignedOutlineStyleListLevel(ParaStyles &rStyles)
        {
            std::sort(rStyles.begin(), rStyles.end(), outlinecmp());
        }

        /*
           Utility to extract flyfmts from a document, potentially from a
           selection.
           */
        Frames GetFrames(const SwDoc &rDoc, SwPaM *pPaM /*, bool bAll*/)
        {
            SwPosFlyFrms aFlys;
            rDoc.GetAllFlyFmts(aFlys, pPaM, true);
            sw::Frames aRet(SwPosFlyFrmsToFrames(aFlys));
            for (USHORT i = aFlys.Count(); i > 0;)
                delete aFlys[--i];
            return aRet;
        }

        Frames GetFramesInNode(const Frames &rFrames, const SwNode &rNode)
        {
            Frames aRet;
            my_copy_if(rFrames.begin(), rFrames.end(),
                std::back_inserter(aRet), anchoredto(rNode.GetIndex()));
            return aRet;
        }

        const SwNumFmt* GetNumFmtFromTxtNode(const SwTxtNode &rTxtNode)
        {
            const SwNumRule *pRule = 0;
            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetNumRule())
                )
            {
                return &(pRule->Get( static_cast< USHORT >(rTxtNode.GetActualListLevel()) ));
            }

            OSL_ENSURE(rTxtNode.GetDoc(), "No document for node?, suspicious");
            if (!rTxtNode.GetDoc())
                return 0;

            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetDoc()->GetOutlineNumRule())
                )
            {
                return &(pRule->Get( static_cast< USHORT >(rTxtNode.GetActualListLevel()) ));
            }

            return 0;
        }

        const SwNumRule* GetNumRuleFromTxtNode(const SwTxtNode &rTxtNode)
        {
            return GetNormalNumRuleFromTxtNode(rTxtNode);
        }

        const SwNumRule* GetNormalNumRuleFromTxtNode(const SwTxtNode &rTxtNode)
        {
            const SwNumRule *pRule = 0;

            if (
                rTxtNode.IsNumbered() && rTxtNode.IsCountedInList() &&
                0 != (pRule = rTxtNode.GetNumRule())
               )
            {
                return pRule;
            }
            return 0;
        }


        SwNoTxtNode *GetNoTxtNodeFromSwFrmFmt(const SwFrmFmt &rFmt)
        {
            const SwNodeIndex *pIndex = rFmt.GetCntnt().GetCntntIdx();
            OSL_ENSURE(pIndex, "No NodeIndex in SwFrmFmt ?, suspicious");
            if (!pIndex)
                return 0;
            SwNodeIndex aIdx(*pIndex, 1);
            return aIdx.GetNode().GetNoTxtNode();
        }

        bool HasPageBreak(const SwNode &rNd)
        {
            const SvxFmtBreakItem *pBreak = 0;
            if (rNd.IsTableNode() && rNd.GetTableNode())
            {
                const SwTable& rTable = rNd.GetTableNode()->GetTable();
                const SwFrmFmt* pApply = rTable.GetFrmFmt();
                OSL_ENSURE(pApply, "impossible");
                if (pApply)
                    pBreak = &(ItemGet<SvxFmtBreakItem>(*pApply, RES_BREAK));
            }
            else if (const SwCntntNode *pNd = rNd.GetCntntNode())
                pBreak = &(ItemGet<SvxFmtBreakItem>(*pNd, RES_BREAK));

            if (pBreak && pBreak->GetBreak() == SVX_BREAK_PAGE_BEFORE)
                return true;
            return false;
        }

        Polygon PolygonFromPolyPolygon(const PolyPolygon &rPolyPoly)
        {
            if(1 == rPolyPoly.Count())
            {
                return rPolyPoly[0];
            }
            else
            {
                // This method will now just concatenate the polygons contained
                // in the given PolyPolygon. Anything else which might be thought of
                // for reducing to a single polygon will just need nore power and
                // cannot create more correct results.
                sal_uInt32 nPointCount(0L);
                sal_uInt16 a;

                for(a = 0; a < rPolyPoly.Count(); a++)
                {
                    nPointCount += (sal_uInt32)rPolyPoly[a].GetSize();
                }

                if(nPointCount > 0x0000ffff)
                {
                    DBG_ERROR("PolygonFromPolyPolygon: too many points for a single polygon (!)");
                    nPointCount = 0x0000ffff;
                }

                Polygon aRetval((sal_uInt16)nPointCount);
                sal_uInt32 nAppendIndex(0L);

                for(a = 0; a < rPolyPoly.Count(); a++)
                {
                    const Polygon& rCandidate = rPolyPoly[a];

                    for(sal_uInt16 b(0); nAppendIndex <= nPointCount && b < rCandidate.GetSize(); b++)
                    {
                        aRetval[(sal_uInt16)nAppendIndex++] = rCandidate[b];
                    }
                }

                return aRetval;
            }
        }

        bool IsStarSymbol(const String &rFontName)
        {
            String sFamilyNm(GetFontToken(rFontName, 0));
            return (sFamilyNm.EqualsIgnoreCaseAscii("starsymbol") ||
                sFamilyNm.EqualsIgnoreCaseAscii("opensymbol"));
        }

        Size GetSwappedInSize(const SwNoTxtNode& rNd)
        {
            Size aGrTwipSz(rNd.GetTwipSize());
            if ((!aGrTwipSz.Width() || !aGrTwipSz.Height()))
            {
                SwGrfNode *pGrfNode = const_cast<SwGrfNode*>(rNd.GetGrfNode());
                if (pGrfNode && (GRAPHIC_NONE != pGrfNode->GetGrf().GetType()))
                {
                    bool bWasSwappedOut = pGrfNode->GetGrfObj().IsSwappedOut();
                    pGrfNode->SwapIn();
                    aGrTwipSz = pGrfNode->GetTwipSize();
                    if (bWasSwappedOut)
                        pGrfNode->SwapOut();
                }
            }

            OSL_ENSURE(aGrTwipSz.Width() && aGrTwipSz.Height(), "0 x 0 graphic ?");
            return aGrTwipSz;
        }

        void RedlineStack::open(const SwPosition& rPos, const SfxPoolItem& rAttr)
        {
            OSL_ENSURE(rAttr.Which() == RES_FLTR_REDLINE, "not a redline");
            maStack.push_back(new SwFltStackEntry(rPos,rAttr.Clone()));
        }


        class SameOpenRedlineType :
            public std::unary_function<const SwFltStackEntry*, bool>
        {
        private:
            RedlineType_t meType;
        public:
            SameOpenRedlineType(RedlineType_t eType) : meType(eType) {}
            bool operator()(const SwFltStackEntry *pEntry) const
            {
                const SwFltRedline *pTest = static_cast<const SwFltRedline *>
                    (pEntry->pAttr);
                return (pEntry->bLocked && (pTest->eType == meType));
            }
        };

        bool RedlineStack::close(const SwPosition& rPos, RedlineType_t eType)
        {
            //Search from end for same type
            myriter aResult = std::find_if(maStack.rbegin(), maStack.rend(),
                SameOpenRedlineType(eType));
            if (aResult != maStack.rend())
            {
                (*aResult)->SetEndPos(rPos);
                return true;
            }
            return false;
        }



        void RedlineStack::closeall(const SwPosition& rPos)
        {
            std::for_each(maStack.begin(), maStack.end(), CloseIfOpen(rPos));
        }


        void SetInDocAndDelete::operator()(SwFltStackEntry *pEntry)
        {
            SwPaM aRegion(pEntry->nMkNode);
            if (
                pEntry->MakeRegion(&mrDoc, aRegion, true) &&
                (*aRegion.GetPoint() != *aRegion.GetMark())
            )
            {
                mrDoc.SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_SHOW_INSERT |
                                         nsRedlineMode_t::REDLINE_SHOW_DELETE));
                const SwFltRedline *pFltRedline = static_cast<const SwFltRedline*>
                    (pEntry->pAttr);

                if (USHRT_MAX != pFltRedline->nAutorNoPrev)
                {
                    SwRedlineData aData(pFltRedline->eTypePrev,
                        pFltRedline->nAutorNoPrev, pFltRedline->aStampPrev, aEmptyStr,
                        0);

                    mrDoc.AppendRedline(new SwRedline(aData, aRegion), true);
                }

                SwRedlineData aData(pFltRedline->eType, pFltRedline->nAutorNo,
                        pFltRedline->aStamp, aEmptyStr, 0);

                mrDoc.AppendRedline(new SwRedline(aData, aRegion), true);
                mrDoc.SetRedlineMode((RedlineMode_t)(nsRedlineMode_t::REDLINE_NONE | nsRedlineMode_t::REDLINE_SHOW_INSERT |
                     nsRedlineMode_t::REDLINE_SHOW_DELETE ));
            }
            delete pEntry;
        }


        bool CompareRedlines::operator()(const SwFltStackEntry *pOneE,
            const SwFltStackEntry *pTwoE) const
        {
            const SwFltRedline *pOne= static_cast<const SwFltRedline*>
                (pOneE->pAttr);
            const SwFltRedline *pTwo= static_cast<const SwFltRedline*>
                (pTwoE->pAttr);

            //Return the earlier time, if two have the same time, prioritize
            //inserts over deletes
            if (pOne->aStamp == pTwo->aStamp)
                return (pOne->eType == nsRedlineType_t::REDLINE_INSERT && pTwo->eType != nsRedlineType_t::REDLINE_INSERT);
            else
                return (pOne->aStamp < pTwo->aStamp) ? true : false;
        }


        RedlineStack::~RedlineStack()
        {
            std::sort(maStack.begin(), maStack.end(), CompareRedlines());
            std::for_each(maStack.begin(), maStack.end(), SetInDocAndDelete(mrDoc));
        }

        USHORT WrtRedlineAuthor::AddName( const String& rNm )
        {
            USHORT nRet;
            typedef std::vector<String>::iterator myiter;
            myiter aIter = std::find(maAuthors.begin(), maAuthors.end(), rNm);
            if (aIter != maAuthors.end())
                nRet = static_cast< USHORT >(aIter - maAuthors.begin());
            else
            {
                nRet = static_cast< USHORT >(maAuthors.size());
                maAuthors.push_back(rNm);
            }
            return nRet;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
