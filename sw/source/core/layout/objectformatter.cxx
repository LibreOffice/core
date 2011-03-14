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
#include <objectformatter.hxx>
#include <objectformattertxtfrm.hxx>
#include <objectformatterlayfrm.hxx>
#include <anchoredobject.hxx>
#include <anchoreddrawobject.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <layact.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>

#include <vector>

// =============================================================================
// helper class <SwPageNumAndTypeOfAnchors>
// --> #i26945# - Additionally the type of the anchor text frame
// is collected - by type is meant 'master' or 'follow'.
// =============================================================================
class SwPageNumAndTypeOfAnchors
{
    private:
        struct tEntry
        {
            SwAnchoredObject* mpAnchoredObj;
            sal_uInt32 mnPageNumOfAnchor;
            bool mbAnchoredAtMaster;
        };

        std::vector< tEntry* > maObjList;

    public:
        inline SwPageNumAndTypeOfAnchors()
        {
        }
        inline ~SwPageNumAndTypeOfAnchors()
        {
            for ( std::vector< tEntry* >::iterator aIter = maObjList.begin();
                  aIter != maObjList.end(); ++aIter )
            {
                delete (*aIter);
            }
            maObjList.clear();
        }

        inline void Collect( SwAnchoredObject& _rAnchoredObj )
        {
            tEntry* pNewEntry = new tEntry();
            pNewEntry->mpAnchoredObj = &_rAnchoredObj;
            // --> #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
            // is replaced by method <FindPageFrmOfAnchor()>. It's return value
            // have to be checked.
            SwPageFrm* pPageFrmOfAnchor = _rAnchoredObj.FindPageFrmOfAnchor();
            if ( pPageFrmOfAnchor )
            {
                pNewEntry->mnPageNumOfAnchor = pPageFrmOfAnchor->GetPhyPageNum();
            }
            else
            {
                pNewEntry->mnPageNumOfAnchor = 0;
            }
            // <--
            // --> #i26945# - collect type of anchor
            SwTxtFrm* pAnchorCharFrm = _rAnchoredObj.FindAnchorCharFrm();
            if ( pAnchorCharFrm )
            {
                pNewEntry->mbAnchoredAtMaster = !pAnchorCharFrm->IsFollow();
            }
            else
            {
                pNewEntry->mbAnchoredAtMaster = true;
            }
            // <--
            maObjList.push_back( pNewEntry );
        }

        inline SwAnchoredObject* operator[]( sal_uInt32 _nIndex )
        {
            SwAnchoredObject* bRetObj = 0L;

            if ( _nIndex < Count())
            {
                bRetObj = maObjList[_nIndex]->mpAnchoredObj;
            }

            return bRetObj;
        }

        inline sal_uInt32 GetPageNum( sal_uInt32 _nIndex ) const
        {
            sal_uInt32 nRetPgNum = 0L;

            if ( _nIndex < Count())
            {
                nRetPgNum = maObjList[_nIndex]->mnPageNumOfAnchor;
            }

            return nRetPgNum;
        }

        // --> #i26945#
        inline bool AnchoredAtMaster( sal_uInt32 _nIndex )
        {
            bool bAnchoredAtMaster( true );

            if ( _nIndex < Count())
            {
                bAnchoredAtMaster = maObjList[_nIndex]->mbAnchoredAtMaster;
            }

            return bAnchoredAtMaster;
        }
        // <--

        inline sal_uInt32 Count() const
        {
            return maObjList.size();
        }
};

// =============================================================================
// implementation of class <SwObjectFormatter>
// =============================================================================
SwObjectFormatter::SwObjectFormatter( const SwPageFrm& _rPageFrm,
                                      SwLayAction* _pLayAction,
                                      const bool _bCollectPgNumOfAnchors )
    : mrPageFrm( _rPageFrm ),
      mbFormatOnlyAsCharAnchored( false ),
      mbConsiderWrapOnObjPos( _rPageFrm.GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) ),
      mpLayAction( _pLayAction ),
      // --> #i26945#
      mpPgNumAndTypeOfAnchors( _bCollectPgNumOfAnchors ? new SwPageNumAndTypeOfAnchors() : 0L )
      // <--
{
}

SwObjectFormatter::~SwObjectFormatter()
{
    delete mpPgNumAndTypeOfAnchors;
}

SwObjectFormatter* SwObjectFormatter::CreateObjFormatter(
                                                      SwFrm& _rAnchorFrm,
                                                      const SwPageFrm& _rPageFrm,
                                                      SwLayAction* _pLayAction )
{
    SwObjectFormatter* pObjFormatter = 0L;
    if ( _rAnchorFrm.IsTxtFrm() )
    {
        pObjFormatter = SwObjectFormatterTxtFrm::CreateObjFormatter(
                                            static_cast<SwTxtFrm&>(_rAnchorFrm),
                                            _rPageFrm, _pLayAction );
    }
    else if ( _rAnchorFrm.IsLayoutFrm() )
    {
        pObjFormatter = SwObjectFormatterLayFrm::CreateObjFormatter(
                                        static_cast<SwLayoutFrm&>(_rAnchorFrm),
                                        _rPageFrm, _pLayAction );
    }
    else
    {
        OSL_ENSURE( false,
                "<SwObjectFormatter::CreateObjFormatter(..)> - unexcepted type of anchor frame" );
    }

    return pObjFormatter;
}

/** method to format all floating screen objects at the given anchor frame

    @author OD
*/
bool SwObjectFormatter::FormatObjsAtFrm( SwFrm& _rAnchorFrm,
                                         const SwPageFrm& _rPageFrm,
                                         SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( _rAnchorFrm, _rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        // format anchored floating screen objects
        bSuccess = pObjFormatter->DoFormatObjs();
    }
    delete pObjFormatter;

    return bSuccess;
}

/** method to format a given floating screen object

    @author OD
*/
bool SwObjectFormatter::FormatObj( SwAnchoredObject& _rAnchoredObj,
                                   SwFrm* _pAnchorFrm,
                                   const SwPageFrm* _pPageFrm,
                                   SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    OSL_ENSURE( _pAnchorFrm || _rAnchoredObj.GetAnchorFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing anchor frame" );
    SwFrm& rAnchorFrm = _pAnchorFrm ? *(_pAnchorFrm) : *(_rAnchoredObj.AnchorFrm());

    OSL_ENSURE( _pPageFrm || rAnchorFrm.FindPageFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing page frame" );
    const SwPageFrm& rPageFrm = _pPageFrm ? *(_pPageFrm) : *(rAnchorFrm.FindPageFrm());

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( rAnchorFrm, rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        // format given floating screen object
        // --> #i40147# - check for moved forward anchor frame
        bSuccess = pObjFormatter->DoFormatObj( _rAnchoredObj, true );
        // <--
    }
    delete pObjFormatter;

    return bSuccess;
}

/** helper method for method <_FormatObj(..)> - performs the intrinsic format
    of the layout of the given layout frame and all its lower layout frames.

    #i28701#
    IMPORTANT NOTE:
    Method corresponds to methods <SwLayAction::FormatLayoutFly(..)> and
    <SwLayAction::FormatLayout(..)>. Thus, its code for the formatting have
    to be synchronised.

    @author OD
*/
void SwObjectFormatter::_FormatLayout( SwLayoutFrm& _rLayoutFrm )
{
    _rLayoutFrm.Calc();

    SwFrm* pLowerFrm = _rLayoutFrm.Lower();
    while ( pLowerFrm )
    {
        if ( pLowerFrm->IsLayoutFrm() )
        {
            _FormatLayout( *(static_cast<SwLayoutFrm*>(pLowerFrm)) );
        }
        pLowerFrm = pLowerFrm->GetNext();
    }
}

/** helper method for method <_FormatObj(..)> - performs the intrinsic
    format of the content of the given floating screen object.

    #i28701#

    @author OD
*/
void SwObjectFormatter::_FormatObjCntnt( SwAnchoredObject& _rAnchoredObj )
{
    if ( !_rAnchoredObj.ISA(SwFlyFrm) )
    {
        // only Writer fly frames have content
        return;
    }

    SwFlyFrm& rFlyFrm = static_cast<SwFlyFrm&>(_rAnchoredObj);
    SwCntntFrm* pCntnt = rFlyFrm.ContainsCntnt();

    while ( pCntnt )
    {
        // format content
        pCntnt->OptCalc();

        // format floating screen objects at content text frame
        // --> #i23129#, #i36347# - pass correct page frame to
        // the object formatter
        if ( pCntnt->IsTxtFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm( *pCntnt,
                                                  *(pCntnt->FindPageFrm()),
                                                  GetLayAction() ) )
        // <--
        {
            // restart format with first content
            pCntnt = rFlyFrm.ContainsCntnt();
            continue;
        }

        // continue with next content
        pCntnt = pCntnt->GetNextCntntFrm();
    }
}

/** performs the intrinsic format of a given floating screen object and its content.

    #i28701#

    @author OD
*/
void SwObjectFormatter::_FormatObj( SwAnchoredObject& _rAnchoredObj )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !(_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
    {
        return;
    }

    // collect anchor object and its 'anchor' page number, if requested
    if ( mpPgNumAndTypeOfAnchors )
    {
        mpPgNumAndTypeOfAnchors->Collect( _rAnchoredObj );
    }

    if ( _rAnchoredObj.ISA(SwFlyFrm) )
    {
        SwFlyFrm& rFlyFrm = static_cast<SwFlyFrm&>(_rAnchoredObj);
        // --> #i34753# - reset flag, which prevents a positioning
        if ( rFlyFrm.IsFlyLayFrm() )
        {
            static_cast<SwFlyLayFrm&>(rFlyFrm).SetNoMakePos( false );
        }
        // <--

        // #i81146# new loop control
        sal_uInt16 nLoopControlRuns = 0;
        const sal_uInt16 nLoopControlMax = 15;

        do {
            if ( mpLayAction )
            {
                mpLayAction->FormatLayoutFly( &rFlyFrm );
                // --> consider, if the layout action
                // has to be restarted due to a delete of a page frame.
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
                // <--
            }
            else
            {
                _FormatLayout( rFlyFrm );
            }
            // --> #i34753# - prevent further positioning, if
            // to-page|to-fly anchored Writer fly frame is already clipped.
            if ( rFlyFrm.IsFlyLayFrm() && rFlyFrm.IsClipped() )
            {
                static_cast<SwFlyLayFrm&>(rFlyFrm).SetNoMakePos( true );
            }
            // <--
            // --> #i23129#, #i36347# - pass correct page frame
            // to the object formatter
            SwObjectFormatter::FormatObjsAtFrm( rFlyFrm,
                                                *(rFlyFrm.FindPageFrm()),
                                                mpLayAction );
            // <--
            if ( mpLayAction )
            {
                mpLayAction->_FormatFlyCntnt( &rFlyFrm );
                // --> consider, if the layout action
                // has to be restarted due to a delete of a page frame.
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
                // <--
            }
            else
            {
                _FormatObjCntnt( rFlyFrm );
            }

            if ( ++nLoopControlRuns >= nLoopControlMax )
            {
#if OSL_DEBUG_LEVEL > 1
                OSL_ENSURE( false, "LoopControl in SwObjectFormatter::_FormatObj: Stage 3!!!" );
#endif
                rFlyFrm.ValidateThisAndAllLowers( 2 );
                nLoopControlRuns = 0;
            }

        // --> #i57917#
        // stop formatting of anchored object, if restart of layout process is requested.
        } while ( !rFlyFrm.IsValid() &&
                  !_rAnchoredObj.RestartLayoutProcess() &&
                  rFlyFrm.GetAnchorFrm() == &GetAnchorFrm() );
        // <--
    }
    else if ( _rAnchoredObj.ISA(SwAnchoredDrawObject) )
    {
        _rAnchoredObj.MakeObjPos();
    }
}

/** invokes the intrinsic format method for all floating screen objects,
    anchored at anchor frame on the given page frame

    #i28701#
    #i26945# - for format of floating screen objects for
    follow text frames, the 'master' text frame is passed to the method.
    Thus, the objects, whose anchor character is inside the follow text
    frame can be formatted.

    @author OD
*/
bool SwObjectFormatter::_FormatObjsAtFrm( SwTxtFrm* _pMasterTxtFrm )
{
    // --> #i26945#
    SwFrm* pAnchorFrm( 0L );
    if ( GetAnchorFrm().IsTxtFrm() &&
         static_cast<SwTxtFrm&>(GetAnchorFrm()).IsFollow() &&
         _pMasterTxtFrm )
    {
        pAnchorFrm = _pMasterTxtFrm;
    }
    else
    {
        pAnchorFrm = &GetAnchorFrm();
    }
    // <--
    if ( !pAnchorFrm->GetDrawObjs() )
    {
        // nothing to do, if no floating screen object is registered at the anchor frame.
        return true;
    }

    bool bSuccess( true );

    sal_uInt32 i = 0;
    for ( ; i < pAnchorFrm->GetDrawObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pAnchorFrm->GetDrawObjs())[i];

        // check, if object's anchor is on the given page frame or
        // object is registered at the given page frame.
        // --> #i26945# - check, if the anchor character of the
        // anchored object is located in a follow text frame. If this anchor
        // follow text frame differs from the given anchor frame, the given
        // anchor frame is a 'master' text frame of the anchor follow text frame.
        // If the anchor follow text frame is in the same body as its 'master'
        // text frame, do not format the anchored object.
        // E.g., this situation can occur during the table row splitting algorithm.
        SwTxtFrm* pAnchorCharFrm = pAnchoredObj->FindAnchorCharFrm();
        const bool bAnchoredAtFollowInSameBodyAsMaster =
                pAnchorCharFrm && pAnchorCharFrm->IsFollow() &&
                pAnchorCharFrm != pAnchoredObj->GetAnchorFrm() &&
                pAnchorCharFrm->FindBodyFrm() ==
                    static_cast<SwTxtFrm*>(pAnchoredObj->AnchorFrm())->FindBodyFrm();
        if ( bAnchoredAtFollowInSameBodyAsMaster )
        {
            continue;
        }
        // <--
        // --> #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
        // is replaced by method <FindPageFrmOfAnchor()>. It's return value
        // have to be checked.
        SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
        OSL_ENSURE( pPageFrmOfAnchor,
                "<SwObjectFormatter::_FormatObjsAtFrm()> - missing page frame." );
        // --> #i26945#
        if ( pPageFrmOfAnchor && pPageFrmOfAnchor == &mrPageFrm )
        // <--
        {
            // if format of object fails, stop formatting and pass fail to
            // calling method via the return value.
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            // considering changes at <pAnchorFrm->GetDrawObjs()> during
            // format of the object.
            if ( !pAnchorFrm->GetDrawObjs() ||
                 i > pAnchorFrm->GetDrawObjs()->Count() )
            {
                break;
            }
            else
            {
                sal_uInt32 nActPosOfObj =
                    pAnchorFrm->GetDrawObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == pAnchorFrm->GetDrawObjs()->Count() ||
                     nActPosOfObj > i )
                {
                    --i;
                }
                else if ( nActPosOfObj < i )
                {
                    i = nActPosOfObj;
                }
            }
        }
    } // end of loop on <pAnchorFrm->.GetDrawObjs()>

    return bSuccess;
}

/** accessor to collected anchored object

    #i28701#

    @author OD
*/
SwAnchoredObject* SwObjectFormatter::GetCollectedObj( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? (*mpPgNumAndTypeOfAnchors)[_nIndex] : 0L;
}

/** accessor to 'anchor' page number of collected anchored object

    #i28701#

    @author OD
*/
sal_uInt32 SwObjectFormatter::GetPgNumOfCollected( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->GetPageNum(_nIndex) : 0L;
}

/** accessor to 'anchor' type of collected anchored object

    #i26945#

    @author OD
*/
bool SwObjectFormatter::IsCollectedAnchoredAtMaster( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors
           ? mpPgNumAndTypeOfAnchors->AnchoredAtMaster(_nIndex)
           : true;
}

/** accessor to total number of collected anchored objects

    #i28701#

    @author OD
*/
sal_uInt32 SwObjectFormatter::CountOfCollected()
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->Count() : 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
