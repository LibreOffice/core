/*************************************************************************
 *
 *  $RCSfile: objectformattertxtfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:11:47 $
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
#ifndef _OBJECTFORMATTERTXTFRM_HXX
#include <objectformattertxtfrm.hxx>
#endif

#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

#ifndef _LAYOUTER_HXX
#include <layouter.hxx>
#endif

#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif

#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif

// =============================================================================
// implementation of class <SwObjectFormatterTxtFrm>
// =============================================================================
SwObjectFormatterTxtFrm::SwObjectFormatterTxtFrm( SwTxtFrm& _rAnchorTxtFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwTxtFrm* _pMasterAnchorTxtFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction, true ),
      mrAnchorTxtFrm( _rAnchorTxtFrm ),
      mpMasterAnchorTxtFrm( _pMasterAnchorTxtFrm )
{
}

SwObjectFormatterTxtFrm::~SwObjectFormatterTxtFrm()
{
}

SwObjectFormatterTxtFrm* SwObjectFormatterTxtFrm::CreateObjFormatter(
                                                SwTxtFrm& _rAnchorTxtFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    SwObjectFormatterTxtFrm* pObjFormatter = 0L;

    // determine 'master' of <_rAnchorTxtFrm>, if anchor frame is a follow text frame.
    SwTxtFrm* pMasterOfAnchorFrm = 0L;
    if ( _rAnchorTxtFrm.IsFollow() )
    {
        pMasterOfAnchorFrm = _rAnchorTxtFrm.FindMaster();
        while ( pMasterOfAnchorFrm->IsFollow() )
        {
            pMasterOfAnchorFrm = pMasterOfAnchorFrm->FindMaster();
        }
    }

    // create object formatter, if floating screen objects are registered
    // at anchor frame (or at 'master' anchor frame)
    if ( _rAnchorTxtFrm.GetDrawObjs() ||
         ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->GetDrawObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterTxtFrm( _rAnchorTxtFrm, _rPageFrm,
                                         pMasterOfAnchorFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterTxtFrm::GetAnchorFrm()
{
    return mrAnchorTxtFrm;
}

bool SwObjectFormatterTxtFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
    {
        return true;
    }

    bool bSuccess( true );

    if ( _rAnchoredObj.IsFormatPossible() )
    {
        _rAnchoredObj.SetRestartLayoutProcess( false );

        _FormatObj( _rAnchoredObj );

        // check, if layout process has to be restarted.
        // if yes, perform needed invalidations.
        if ( _rAnchoredObj.RestartLayoutProcess() )
        {
            bSuccess = false;
            _InvalidatePrevObjs( _rAnchoredObj );
            _InvalidateFollowObjs( _rAnchoredObj, true );
        }

        // format anchor text frame, if wrapping style influence of the object
        // has to be considered and it's <NONE_SUCCESSIVE_POSITIONED>
        if ( bSuccess &&
             _rAnchoredObj.ConsiderObjWrapInfluenceOnObjPos() &&
             _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                GetWrapInfluenceOnObjPos() ==
                    text::WrapInfluenceOnPosition::NONE_SUCCESSIVE_POSITIONED )
        {
            mrAnchorTxtFrm.Calc();
        }
    }

    return bSuccess;
}

bool SwObjectFormatterTxtFrm::DoFormatObjs()
{
    if ( !mrAnchorTxtFrm.IsValid() )
    {
        if ( GetLayAction() &&
             mrAnchorTxtFrm.FindPageFrm() != &GetPageFrm() )
        {
            // notify layout action, thus is can restart the layout process on
            // a previous page.
            GetLayAction()->SetAgain();
        }
        else
        {
            // the anchor text frame has to be valid, thus assert.
            ASSERT( false,
                    "<SwObjectFormatterTxtFrm::DoFormatObjs()> called for invalidate anchor text frame." );
        }

        return false;
    }

    bool bSuccess( true );

    if ( mrAnchorTxtFrm.IsFollow() )
    {
        // Only floating screen objects anchored as-character are directly
        // registered at a follow text frame. The other floating screen objects
        // are registered at the 'master' anchor text frame.
        // Thus, format the other floating screen objects through the 'master'
        // anchor text frame
        ASSERT( mpMasterAnchorTxtFrm,
                "SwObjectFormatterTxtFrm::DoFormatObjs() - missing 'master' anchor text frame" );
        SwObjectFormatterTxtFrm aObjFormatterMasterTxtFrm( *(mpMasterAnchorTxtFrm),
                                                           GetPageFrm(),
                                                           0L,
                                                           GetLayAction() );
        bSuccess = aObjFormatterMasterTxtFrm.DoFormatObjs();

        if ( bSuccess )
        {
            // format of as-character anchored floating screen objects - no failure
            // excepted on the format of these objects.
            _FormatObjsAtFrm();
        }
    }
    else
    {
        bSuccess = _FormatObjsAtFrm();
    }

    if ( bSuccess && ConsiderWrapOnObjPos() )
    {
        const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

        // Format anchor text frame after its objects are formatted.
        // Note: The format of the anchor frame also formats the invalid
        //       previous frames of the anchor frame. The format of the previous
        //       frames is needed to get a correct result of format of the
        //       anchor frame for the following check for moved forward anchors
        mrAnchorTxtFrm.Calc();

        sal_uInt32 nToPageNum( 0L );
        SwAnchoredObject* pObj = 0L;
        if ( !mrAnchorTxtFrm.IsFollow() )
        {
            pObj = _GetFirstObjWithMovedFwdAnchor(
                    text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED,
                    nToPageNum );
        }
        if ( pObj )
        {
            // Object found, whose anchor is moved forward

            if ( bDoesAnchorHadPrev )
            {
                // Indicate that anchor text frame has to move forward and
                // invalidate its position to force a re-format.
                SwLayouter::InsertMovedFwdFrm( *(GetPageFrm().GetFmt()->GetDoc()),
                                               mrAnchorTxtFrm,
                                               nToPageNum );
                mrAnchorTxtFrm.InvalidatePos();
            }

            // Indicate restart of the layout process
            bSuccess = false;

            // If needed, invalidate previous objects anchored at same anchor
            // text frame.
            _InvalidatePrevObjs( *pObj );

            // Invalidate object and following objects for the restart of the
            // layout process
            _InvalidateFollowObjs( *pObj, true );
        }
    }

    return bSuccess;
}

void SwObjectFormatterTxtFrm::_InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj )
{
    // invalidate all previous objects, whose wrapping influence on the object
    // positioning is <NONE_CONCURRENT_POSIITIONED>.
    // Note: list of objects at anchor frame is sorted by this property.
    if ( _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                GetWrapInfluenceOnObjPos() ==
                    text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED )
    {
        const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
        if ( pObjs )
        {
            // determine start index
            sal_Int32 i = pObjs->ListPosOf( _rAnchoredObj ) - 1;
            for ( ; i >= 0; --i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                     GetWrapInfluenceOnObjPos() ==
                        text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED )
                {
                    pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                }
            }
        }
    }
}

void SwObjectFormatterTxtFrm::_InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
                                                     const bool _bInclObj )
{
    if ( _bInclObj )
    {
        _rAnchoredObj.InvalidateObjPosForConsiderWrapInfluence( true );
    }

    const SwSortedObjs* pObjs = GetPageFrm().GetSortedObjs();
    if ( pObjs )
    {
        // determine start index
        sal_uInt32 i = pObjs->ListPosOf( _rAnchoredObj ) + 1;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
        }
    }
}

SwAnchoredObject* SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum )
{
    ASSERT( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::NONE_SUCCESSIVE_POSITIONED ||
            _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::NONE_CONCURRENT_POSITIONED,
            "<SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(..)> - invalid value for parameter <_nWrapInfluenceOnPosition>" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    sal_uInt32 i = 0L;
    for ( ; i < CountOfCollected(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = GetCollectedObj(i);
        if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() &&
             pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                   GetWrapInfluenceOnObjPos() == _nWrapInfluenceOnPosition )
        {
            sal_uInt32 nFromPageNum = GetPgNumOfCollected(i);
            sal_uInt32 nPageNum = pAnchoredObj->GetPageFrmOfAnchor().GetPhyPageNum();
            if ( nPageNum > nFromPageNum )
            {
                _noToPageNum = nPageNum;
                pRetAnchoredObj = pAnchoredObj;
                break;
            }
        }
    }

    return pRetAnchoredObj;
}
