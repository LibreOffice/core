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

#include <svx/sdrselection.hxx>
#include <svx/svdview.hxx>
#include <svx/svdstr.hrc>
#include <svx/svdglob.hxx>
#include <svx/svdlegacy.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// selection implementations

namespace sdr
{
    namespace selection
    {
        bool SelectionComparator::operator()(SdrObject* pA, SdrObject* pB) const
        {
            // sort by NavigationPosition
            return pA->GetNavigationPosition() < pB->GetNavigationPosition();
        }

        bool IndicesComparator::operator()(SdrObject* pA, SdrObject* pB) const
        {
            // sort associative point index map by SdrObject's address
            return pA < pB;
        }

        void Selection::checkPointIndexCorrection()
        {
            for(IndicesMap::iterator aIndices(maPointIndices.begin()); aIndices != maPointIndices.end();)
            {
                SdrObject* pCandidate = aIndices->first;
                OSL_ENSURE(pCandidate, "Missing SdrObject pointer in point selection(!)");
                Indices aCurrent = aIndices->second;
                bool bErase(false);
                IndicesMap::iterator aNext(aIndices);

                aNext++;

                // for each SdrObject check if it's a polygon object at all
                if(pCandidate->IsPolygonObject())
                {
                    const sal_uInt32 nMax(pCandidate->GetObjectPointCount());

                    if(nMax)
                    {
                        // take all points out of the selection which are beyond the maximum point index
                        Indices::iterator aUpperBound(aCurrent.upper_bound(nMax - 1));

                        if(aUpperBound != aCurrent.end())
                        {
                            aCurrent.erase(aUpperBound, aCurrent.end());
                            bErase = aCurrent.empty();

                            if(!bErase)
                            {
                                // copy back if still points are selected
                                aIndices->second = aCurrent;
                            }
                        }
                    }
                    else
                    {
                        // maximum is 0, all points have to be removed
                        bErase = true;
                    }
                }
                else
                {
                    // not a PolygonObject
                    bErase = true;
                }

                if(bErase)
                {
                    maPointIndices.erase(aIndices);
                }

                aIndices = aNext;
            }
        }

        void Selection::checkGlueIndexCorrection()
        {
            for(IndicesMap::iterator aIndices(maGlueIndices.begin()); aIndices != maGlueIndices.end();)
            {
                SdrObject* pCandidate = aIndices->first;
                OSL_ENSURE(pCandidate, "Missing SdrObject pointer in glue selection(!)");
                bool bErase(false);
                IndicesMap::iterator aNext(aIndices);
                const SdrGluePointList* pGPL = pCandidate->GetGluePointList();

                aNext++;

                // for each SdrObject check if it has GluePoints at all
                if(pGPL)
                {
                    Indices aCurrent = aIndices->second;
                    Indices aNewList;

                    // check for each selected GluePoint if it exists at the SdrObject. If yes, take
                    // it over to a new list. Thus, all no longer existing GluePoints will be erased
                    for(Indices::const_iterator aIter(aCurrent.begin()); aIter != aCurrent.end(); aIter++)
                    {
                        if(SDRGLUEPOINT_NOTFOUND != pGPL->FindGluePoint(*aIter))
                        {
                            aNewList.insert(aNewList.end(), *aIter);
                        }
                    }

                    if(aNewList.empty())
                    {
                        // no glues left
                        bErase = true;
                    }
                    else
                    {
                        // copy back new list
                        aIndices->second = aNewList;
                    }
                }
                else
                {
                    // no glues at all
                    bErase = true;
                }

                if(bErase)
                {
                    maGlueIndices.erase(aIndices);
                }

                aIndices = aNext;
            }
        }

        void Selection::delayedSelectionChanged()
        {
            // trigger a delayed selection change
            SetTimeout(1);
            Start();
        }

        bool Selection::isSelectionChangePending() const
        {
            return IsActive();
        }

        void Selection::forceSelectionChange()
        {
            if(IsActive())
            {
                Stop();
                Timeout();
            }
        }

        void Selection::Timeout()
        {
            // check if marked points need to be corrected
            if(!maPointIndices.empty())
            {
                checkPointIndexCorrection();
            }

            // check if marked glues need to be corrected
            if(!maGlueIndices.empty())
            {
                checkGlueIndexCorrection();
            }

            // trigger selection change at SdrView
            mrSdrView.handleSelectionChange();
        }

        void Selection::createIndexDescription(bool bPoints)
        {
            if(!maSet.empty())
            {
                IndicesMap& rIndices = bPoints ? maPointIndices : maGlueIndices;

                if(!rIndices.empty())
                {
                    SelectionSet::const_iterator aIter(maSet.begin());
                    sal_uInt32 nIndicesCount(0);
                    SdrObjectVector aObjectsWithIndices;

                    for(;aIter != maSet.end(); aIter++)
                    {
                        IndicesMap::const_iterator aIndices(rIndices.find(*aIter));

                        if(aIndices != rIndices.end())
                        {
                            nIndicesCount += aIndices->second.size();
                            aObjectsWithIndices.push_back(*aIter);
                        }
                    }

                    if(aObjectsWithIndices.size())
                    {
                        String aObjectName;
                        String aGlueText;

                        if(1 == aObjectsWithIndices.size())
                        {
                            aObjectsWithIndices[0]->TakeObjNameSingul(aObjectName);
                            aGlueText = (ImpGetResStr(bPoints ? STR_ViewMarkedPoint : STR_ViewMarkedGluePoint));
                        }
                        else
                        {
                            aObjectsWithIndices[0]->TakeObjNamePlural(aObjectName);
                            String aCompare;
                            bool bEqual(true);

                            for(sal_uInt32 a(1); bEqual && a < aObjectsWithIndices.size(); a++)
                            {
                                aObjectsWithIndices[a]->TakeObjNamePlural(aCompare);
                                bEqual = aObjectName.Equals(aCompare);
                            }

                            if(!bEqual)
                            {
                                aObjectName = ImpGetResStr(STR_ObjNamePlural);
                            }

                            aObjectName.Insert(sal_Unicode(' '), 0);
                            aObjectName.Insert(UniString::CreateFromInt32(aObjectsWithIndices.size()), 0);

                            aGlueText = (ImpGetResStr(bPoints ? STR_ViewMarkedPoints : STR_ViewMarkedGluePoints));
                            aGlueText.SearchAndReplaceAscii("%2", UniString::CreateFromInt32(nIndicesCount));
                        }

                        aGlueText.SearchAndReplaceAscii("%1", aObjectName);

                        if(bPoints)
                        {
                            maPointDescription = aGlueText;
                        }
                        else
                        {
                            maGlueDescription = aGlueText;
                        }
                    }
                }
            }
        }

        Selection::Selection(SdrView& rSdrView)
        :   boost::noncopyable(),
            SfxListener(),
            Timer(),
            mrSdrView(rSdrView),
            maSet(),
            maPointIndices(),
            maGlueIndices(),
            maVector(),
            maPointDescription(),
            maGlueDescription(),
            maSnapRange()
        {
        }

        void Selection::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
        {
            const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

            if(pSdrHint)
            {
                OSL_ENSURE(pSdrHint->GetSdrHintObject(),
                    "OOps, no SdrObject in Selection::Notify hint (!)");
                OSL_ENSURE(pSdrHint->GetSdrHintObject() && isSdrObject(*pSdrHint->GetSdrHintObject()),
                    "OOps, SdrObject in Selection::Notify is NOT part of the selection (!)");

                switch(pSdrHint->GetSdrHintKind())
                {
                    case HINT_OBJCHG_MOVE:
                    case HINT_OBJCHG_RESIZE:
                    case HINT_OBJCHG_ATTR:
                    {
                        // the BoundRange/SnapRange of a selected SdrObject
                        // may have changed. Since this may happen often and
                        // mainly based on the selection, do not trigger a
                        // selection change immediately, but start a timer
                        // to do so later
                        delayedSelectionChanged();

                        // reset buffered SnapRange
                        resetBufferedSelectionInformation();
                        break;
                    }
                    case HINT_OBJINSERTED:
                    {
                        // this should not happen since this would mean that an already
                        // selected SdrObject (only these are listened to) was inserted
                        // to a SdrObjList
                        OSL_ENSURE(false, "Caution: already selected SdrObject was added to the SdrModel tree (!)");
                        break;
                    }
                    case HINT_OBJREMOVED:
                    {
                        // a selected SdrObject was removed from SdrObjList, thus is no
                        // longer active in the SdrModel. Remove from selection, too.
                        // This also triggers a delayedSelectionChanged()
                        if(pSdrHint->GetSdrHintObject())
                        {
                            removeSdrObject(*pSdrHint->GetSdrHintObject());
                        }
                        break;
                    }
                    case HINT_SDROBJECTDYING:
                    {
                        if(pSdrHint->GetSdrHintObject())
                        {
                            // If selected object is deleted, trigger an assertion since this potentially leads to problems.
                            // In the next step with RefCounted Sdrobjects this problem will be solved (in the middle run
                            // anyways) so this may be removed again later.
                            if(isSdrObject(*pSdrHint->GetSdrHintObject()))
                            {
                                OSL_ENSURE(false, "Caution: SdrObject which is active part of a selection is deleted. \
                                    This should be avoided by managing the selection before calling changing methods (!)");

                                // remove the deleted SdrObject from the selection. This will trigger
                                // a delayedSelectionChanged to react. All selection
                                // users have to forget the deleted SdrObject ASAP
                                removeSdrObject(*pSdrHint->GetSdrHintObject());
                            }
                        }
                        break;
                    }
                }
            }
        }

        const SdrObjectVector& Selection::getVector() const
        {
            if(maVector.empty() && !maSet.empty())
            {
                const_cast< Selection* >(this)->maVector = SdrObjectVector(maSet.begin(), maSet.end());
            }

            return maVector;
        }

        SdrObject* Selection::getSingle() const
        {
            if(maSet.empty())
            {
                return 0;
            }
            else if(maSet.size() > 1)
            {
                return 0;
            }
            else
            {
                return *maSet.begin();
            }
        }

        bool Selection::empty() const
        {
            return maSet.empty();
        }

        sal_uInt32 Selection::size() const
        {
            if(maSet.empty())
            {
                return 0;
            }
            else
            {
                return maSet.size();
            }
        }

        bool Selection::isSdrObject(const SdrObject& rObject) const
        {
            if(maSet.empty())
            {
                return false;
            }
            else
            {
                return 0 != maSet.count(const_cast< SdrObject* >(&rObject));
            }
        }

        void Selection::removeSdrObject(const SdrObject& rObject)
        {
            if(!maSet.empty())
            {
                SelectionSet::iterator aFound(maSet.find(const_cast< SdrObject* >(&rObject)));

                if(aFound != maSet.end())
                {
                    EndListening(const_cast< SdrObject& >(rObject));
                    maSet.erase(aFound);

                    if(!maPointIndices.empty())
                    {
                        IndicesMap::iterator aPointIndex(maPointIndices.find(const_cast< SdrObject* >(&rObject)));

                        if(aPointIndex != maPointIndices.end())
                        {
                            maPointIndices.erase(aPointIndex);
                            resetBufferedPointInformation();
                        }
                    }

                    if(!maGlueIndices.empty())
                    {
                        IndicesMap::iterator aPointIndex(maGlueIndices.find(const_cast< SdrObject* >(&rObject)));

                        if(aPointIndex != maGlueIndices.end())
                        {
                            maGlueIndices.erase(aPointIndex);
                            resetBufferedGlueInformation();
                        }
                    }

                    resetBufferedSelectionInformation();
                    delayedSelectionChanged();
                }
            }
        }

        void Selection::addSdrObject(const SdrObject& rObject)
        {
            SelectionSet::iterator aFound(maSet.find(const_cast< SdrObject* >(&rObject)));

            if(aFound == maSet.end())
            {
                StartListening(const_cast< SdrObject& >(rObject));
                maSet.insert(const_cast< SdrObject* >(&rObject));
                maPointIndices.clear();
                maGlueIndices.clear();
                resetBufferedSelectionInformation();
                resetBufferedPointInformation();
                resetBufferedGlueInformation();
                delayedSelectionChanged();
            }
        }

        void Selection::clear()
        {
            if(!maSet.empty())
            {
                for(SelectionSet::const_iterator aIter(maSet.begin()); aIter != maSet.end(); aIter++)
                {
                    EndListening(const_cast< SdrObject& >(**aIter));
                }

                maSet.clear();
                maPointIndices.clear();
                maGlueIndices.clear();
                resetBufferedSelectionInformation();
                resetBufferedPointInformation();
                resetBufferedGlueInformation();
                delayedSelectionChanged();
            }
        }

        void Selection::setSdrObjects(const SdrObjectVector& rSdrObjectVector)
        {
            if(rSdrObjectVector.empty())
            {
                clear();
            }
            else
            {
                const SelectionSet aNewSet(rSdrObjectVector.begin(), rSdrObjectVector.end());

                if(aNewSet != maSet)
                {
                    SelectionSet::const_iterator aIter;

                    for(aIter = maSet.begin(); aIter != maSet.end(); aIter++)
                    {
                        EndListening(const_cast< SdrObject& >(**aIter));
                    }

                    maSet = aNewSet;
                    maPointIndices.clear();
                    maGlueIndices.clear();
                    resetBufferedSelectionInformation();
                    resetBufferedPointInformation();
                    resetBufferedGlueInformation();

                    for(aIter = maSet.begin(); aIter != maSet.end(); aIter++)
                    {
                        StartListening(const_cast< SdrObject& >(**aIter));
                    }

                    delayedSelectionChanged();
                }
            }
        }

        sdr::selection::Indices Selection::getIndicesForSdrObject(const SdrObject& rObject, bool bPoints) const
        {
            const IndicesMap& rIndices = bPoints ? maPointIndices : maGlueIndices;

            if(!rIndices.empty())
            {
                const IndicesMap::const_iterator aPointIndex(rIndices.find(const_cast< SdrObject* >(&rObject)));

                if(aPointIndex != rIndices.end())
                {
                    return aPointIndex->second;
                }
            }

            return sdr::selection::Indices();
        }

        void Selection::setIndicesForSdrObject(const SdrObject& rObject, const sdr::selection::Indices& rNew, bool bPoints)
        {
            if(rNew.empty())
            {
                clearIndicesForSdrObject(rObject, bPoints);
            }
            else
            {
                if(!maSet.empty() && 0 != maSet.count(const_cast< SdrObject* >(&rObject)))
                {
                    IndicesMap& rIndices = bPoints ? maPointIndices : maGlueIndices;
                    const IndicesMap::iterator aFound(rIndices.find(const_cast< SdrObject* >(&rObject)));
                    bool bChanged(true);

                    if(aFound != rIndices.end())
                    {
                        bChanged = (aFound->second != rNew);
                        aFound->second = rNew;
                    }
                    else
                    {
                        rIndices.insert(::std::pair< SdrObject*, Indices >(const_cast< SdrObject* >(&rObject), rNew));
                    }

                    if(bChanged)
                    {
                        if(bPoints)
                        {
                            resetBufferedPointInformation();
                        }
                        else
                        {
                            resetBufferedGlueInformation();
                        }

                        delayedSelectionChanged();
                    }
                }
                else
                {
                    OSL_ENSURE(false, "Tried to set indices for non-selected SrObject (!)");
                }
            }
        }

        void Selection::clearIndicesForSdrObject(const SdrObject& rObject, bool bPoints)
        {
            if(!maSet.empty() && 0 != maSet.count(const_cast< SdrObject* >(&rObject)))
            {
                // clear if entry exists
                IndicesMap& rIndices = bPoints ? maPointIndices : maGlueIndices;

                if(!rIndices.empty())
                {
                    const IndicesMap::iterator aFound(rIndices.find(const_cast< SdrObject* >(&rObject)));

                    if(aFound != rIndices.end())
                    {
                        const bool bWasUsed(0 != aFound->second.size());
                        rIndices.erase(aFound);

                        if(bWasUsed)
                        {
                            if(bPoints)
                            {
                                resetBufferedPointInformation();
                            }
                            else
                            {
                                resetBufferedGlueInformation();
                            }

                            delayedSelectionChanged();
                        }
                    }
                }
            }
            else
            {
                OSL_ENSURE(false, "Tried to clear indices for non-selected SrObject (!)");
            }
        }

        const String& Selection::getIndexDescription(bool bPoints) const
        {
            if(0 == (bPoints ? maPointDescription.Len() : maGlueDescription.Len()))
            {
                const_cast< Selection* >(this)->createIndexDescription(bPoints);
            }

            return bPoints ? maPointDescription : maGlueDescription;
        }

        bool Selection::hasIndices(bool bPoints) const
        {
            const IndicesMap& rIndices = bPoints ? maPointIndices : maGlueIndices;

            return !rIndices.empty();
        }

        const basegfx::B2DRange& Selection::getSnapRange() const
        {
            if(maSnapRange.isEmpty())
            {
                const SdrObjectVector& rVector = getVector();

                if(!rVector.empty())
                {
                    const_cast< Selection* >(this)->maSnapRange = sdr::legacy::GetAllObjSnapRange(rVector);
                }
            }

            return maSnapRange;
        }
    } // end of namespace selection
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
