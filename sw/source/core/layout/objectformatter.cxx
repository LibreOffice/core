/*************************************************************************
 *
 *  $RCSfile: objectformatter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:11:16 $
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
#ifndef _OBJECTFORMATTER_HXX
#include <objectformatter.hxx>
#endif

#ifndef _OBJECTFORMATTERTXTFRM_HXX
#include <objectformattertxtfrm.hxx>
#endif
#ifndef _OBJECTFORMATTERLAYFRM_HXX
#include <objectformatterlayfrm.hxx>
#endif

#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
#ifndef _ANCHOREDDRAWOBJECT_HXX
#include <anchoreddrawobject.hxx>
#endif
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif

#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif

#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#include <vector>

// =============================================================================
// helper class <SwPageNumOfAnchors>
// =============================================================================
class SwPageNumOfAnchors
{
    private:
        struct tEntry
        {
            SwAnchoredObject* mpAnchoredObj;
            sal_uInt32 mnPageNumOfAnchor;
        };

        std::vector< tEntry* > maObjList;

    public:
        inline SwPageNumOfAnchors()
        {
        }
        inline ~SwPageNumOfAnchors()
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
            pNewEntry->mnPageNumOfAnchor =
                            _rAnchoredObj.GetPageFrmOfAnchor().GetPhyPageNum();
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

        inline sal_uInt32 GetPageNum( sal_uInt32 _nIndex )
        {
            sal_uInt32 nRetPgNum = 0L;

            if ( _nIndex < Count())
            {
                nRetPgNum = maObjList[_nIndex]->mnPageNumOfAnchor;
            }

            return nRetPgNum;
        }

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
      mbConsiderWrapOnObjPos( _rPageFrm.GetFmt()->GetDoc()->ConsiderWrapOnObjPos() ),
      mpLayAction( _pLayAction ),
      mpPgNumOfAnchors( _bCollectPgNumOfAnchors ? new SwPageNumOfAnchors() : 0L )
{
}

SwObjectFormatter::~SwObjectFormatter()
{
    delete mpPgNumOfAnchors;
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
        ASSERT( false,
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

/** method to format all as-character anchored floating screen objects
    at the given anchor frame

    @author OD
*/
bool SwObjectFormatter::FormatAsCharAnchoredObjsAtFrm( SwFrm& _rAnchorFrm,
                                                       const SwPageFrm& _rPageFrm,
                                                       SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( _rAnchorFrm, _rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        // format as-character anchored floating screen objects
        pObjFormatter->SetFormatOnlyAsCharAnchored();
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

    ASSERT( _pAnchorFrm || _rAnchoredObj.GetAnchorFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing anchor frame" );
    SwFrm& rAnchorFrm = _pAnchorFrm ? *(_pAnchorFrm) : *(_rAnchoredObj.AnchorFrm());

    ASSERT( _pPageFrm || rAnchorFrm.FindPageFrm(),
            "<SwObjectFormatter::FormatObj(..)> - missing page frame" );
    const SwPageFrm& rPageFrm = _pPageFrm ? *(_pPageFrm) : *(rAnchorFrm.FindPageFrm());

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( rAnchorFrm, rPageFrm, _pLayAction );

    if ( pObjFormatter )
    {
        // format given floating screen object
        bSuccess = pObjFormatter->DoFormatObj( _rAnchoredObj );
    }
    delete pObjFormatter;

    return bSuccess;
}

const SwPageFrm& SwObjectFormatter::GetPageFrm() const
{
    return mrPageFrm;
}

const bool SwObjectFormatter::FormatOnlyAsCharAnchored() const
{
    return mbFormatOnlyAsCharAnchored;
}

const bool SwObjectFormatter::ConsiderWrapOnObjPos() const
{
    return mbConsiderWrapOnObjPos;
}

SwLayAction* SwObjectFormatter::GetLayAction()
{
    return mpLayAction;
}

/** method to restrict the format of floating screen objects to
    as-character anchored ones

    @author OD
*/
void SwObjectFormatter::SetFormatOnlyAsCharAnchored()
{
    mbFormatOnlyAsCharAnchored = true;
}

/** helper method for method <_FormatObj(..)> - performs the intrinsic format
    of the layout of the given layout frame and all its lower layout frames.

    OD 2004-06-28 #i28701#
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

    OD 2004-06-28 #i28701#

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
        if ( pCntnt->IsTxtFrm() &&
             !SwObjectFormatter::FormatObjsAtFrm( *pCntnt, mrPageFrm, GetLayAction() ) )
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

    OD 2004-06-28 #i28701#

    @author OD
*/
void SwObjectFormatter::_FormatObj( SwAnchoredObject& _rAnchoredObj )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
    {
        return;
    }

    // collect anchor object and its 'anchor' page number, if requested
    if ( mpPgNumOfAnchors )
    {
        mpPgNumOfAnchors->Collect( _rAnchoredObj );
    }

    if ( _rAnchoredObj.ISA(SwFlyFrm) )
    {
        SwFlyFrm& rFlyFrm = static_cast<SwFlyFrm&>(_rAnchoredObj);
        do {
            if ( mpLayAction )
            {
                mpLayAction->FormatLayoutFly( &rFlyFrm );
            }
            else
            {
                _FormatLayout( rFlyFrm );
            }
            SwObjectFormatter::FormatObjsAtFrm( rFlyFrm, mrPageFrm, mpLayAction );
            if ( mpLayAction )
            {
                mpLayAction->_FormatFlyCntnt( &rFlyFrm );
            }
            else
            {
                _FormatObjCntnt( rFlyFrm );
            }
        } while ( !rFlyFrm.IsValid() &&
                  rFlyFrm.GetAnchorFrm() == &GetAnchorFrm() );
    }
    else if ( _rAnchoredObj.ISA(SwAnchoredDrawObject) )
    {
        _rAnchoredObj.MakeObjPos();
    }
}

/** invokes the intrinsic format method for all floating screen objects,
    anchored at anchor frame on the given page frame

    OD 2004-06-28 #i28701#

    @author OD
*/
bool SwObjectFormatter::_FormatObjsAtFrm()
{
    if ( !GetAnchorFrm().GetDrawObjs() )
    {
        // nothing to do, if no floating screen object is registered at the anchor frame.
        return true;
    }

    bool bSuccess( true );

    sal_uInt32 i = 0;
    for ( ; i < GetAnchorFrm().GetDrawObjs()->Count(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*GetAnchorFrm().GetDrawObjs())[i];

        // check, if object's anchor is on the given page frame or
        // object is registered at the given page frame.
        if ( &pAnchoredObj->GetPageFrmOfAnchor() == &mrPageFrm ||
             pAnchoredObj->GetPageFrm() == &mrPageFrm )
        {
            // if format of object fails, stop formatting and pass fail to
            // calling method via the return value.
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            // considering changes at <GetAnchorFrm().GetDrawObjs()> during
            // format of the object.
            if ( !GetAnchorFrm().GetDrawObjs() ||
                 i > GetAnchorFrm().GetDrawObjs()->Count() )
            {
                break;
            }
            else
            {
                sal_uInt32 nActPosOfObj =
                    GetAnchorFrm().GetDrawObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == GetAnchorFrm().GetDrawObjs()->Count() ||
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
    } // end of loop on <GetAnchorFrm().GetDrawObjs()>

    return bSuccess;
}

/** accessor to collected anchored object

    OD 2004-07-05 #i28701#

    @author OD
*/
SwAnchoredObject* SwObjectFormatter::GetCollectedObj( const sal_uInt32 _nIndex )
{
    return mpPgNumOfAnchors ? (*mpPgNumOfAnchors)[_nIndex] : 0L;
}

/** accessor to 'anchor' page number of collected anchored object

    OD 2004-07-05 #i28701#

    @author OD
*/
sal_uInt32 SwObjectFormatter::GetPgNumOfCollected( const sal_uInt32 _nIndex )
{
    return mpPgNumOfAnchors ? mpPgNumOfAnchors->GetPageNum(_nIndex) : 0L;
}

/** accessor to total number of collected anchored objects

    OD 2004-07-05 #i28701#

    @author OD
*/
sal_uInt32 SwObjectFormatter::CountOfCollected()
{
    return mpPgNumOfAnchors ? mpPgNumOfAnchors->Count() : 0L;
}
