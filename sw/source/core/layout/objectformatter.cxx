/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <objectformattertxtfrm.hxx>
#include <objectformatterlayfrm.hxx>
#include <anchoreddrawobject.hxx>
#include <sortedobjs.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <layact.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>

#include <vector>

// --> #i26945# - Additionally the type of the anchor text frame
// is collected - by type is meant 'master' or 'follow'.
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
        SwPageNumAndTypeOfAnchors()
        {
        }
        ~SwPageNumAndTypeOfAnchors()
        {
            for ( std::vector< tEntry* >::iterator aIter = maObjList.begin();
                  aIter != maObjList.end(); ++aIter )
            {
                delete (*aIter);
            }
            maObjList.clear();
        }

        void Collect( SwAnchoredObject& _rAnchoredObj )
        {
            tEntry* pNewEntry = new tEntry;
            pNewEntry->mpAnchoredObj = &_rAnchoredObj;
            // #i33751#, #i34060# - method <GetPageFrameOfAnchor()>
            // is replaced by method <FindPageFrameOfAnchor()>. It's return value
            // have to be checked.
            SwPageFrame* pPageFrameOfAnchor = _rAnchoredObj.FindPageFrameOfAnchor();
            if ( pPageFrameOfAnchor )
            {
                pNewEntry->mnPageNumOfAnchor = pPageFrameOfAnchor->GetPhyPageNum();
            }
            else
            {
                pNewEntry->mnPageNumOfAnchor = 0;
            }
            // --> #i26945# - collect type of anchor
            SwTextFrame* pAnchorCharFrame = _rAnchoredObj.FindAnchorCharFrame();
            if ( pAnchorCharFrame )
            {
                pNewEntry->mbAnchoredAtMaster = !pAnchorCharFrame->IsFollow();
            }
            else
            {
                pNewEntry->mbAnchoredAtMaster = true;
            }
            maObjList.push_back( pNewEntry );
        }

        SwAnchoredObject* operator[]( sal_uInt32 _nIndex )
        {
            SwAnchoredObject* bRetObj = nullptr;

            if ( _nIndex < Count())
            {
                bRetObj = maObjList[_nIndex]->mpAnchoredObj;
            }

            return bRetObj;
        }

        sal_uInt32 GetPageNum( sal_uInt32 _nIndex ) const
        {
            sal_uInt32 nRetPgNum = 0L;

            if ( _nIndex < Count())
            {
                nRetPgNum = maObjList[_nIndex]->mnPageNumOfAnchor;
            }

            return nRetPgNum;
        }

        // --> #i26945#
        bool AnchoredAtMaster( sal_uInt32 _nIndex )
        {
            bool bAnchoredAtMaster( true );

            if ( _nIndex < Count())
            {
                bAnchoredAtMaster = maObjList[_nIndex]->mbAnchoredAtMaster;
            }

            return bAnchoredAtMaster;
        }

        sal_uInt32 Count() const
        {
            return maObjList.size();
        }
};

SwObjectFormatter::SwObjectFormatter( const SwPageFrame& _rPageFrame,
                                      SwLayAction* _pLayAction,
                                      const bool _bCollectPgNumOfAnchors )
    : mrPageFrame( _rPageFrame ),
      mbConsiderWrapOnObjPos( _rPageFrame.GetFormat()->getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) ),
      mpLayAction( _pLayAction ),
      // --> #i26945#
      mpPgNumAndTypeOfAnchors( _bCollectPgNumOfAnchors ? new SwPageNumAndTypeOfAnchors() : nullptr )
{
}

SwObjectFormatter::~SwObjectFormatter()
{
}

SwObjectFormatter* SwObjectFormatter::CreateObjFormatter(
                                                      SwFrame& _rAnchorFrame,
                                                      const SwPageFrame& _rPageFrame,
                                                      SwLayAction* _pLayAction )
{
    SwObjectFormatter* pObjFormatter = nullptr;
    if ( _rAnchorFrame.IsTextFrame() )
    {
        pObjFormatter = SwObjectFormatterTextFrame::CreateObjFormatter(
                                            static_cast<SwTextFrame&>(_rAnchorFrame),
                                            _rPageFrame, _pLayAction );
    }
    else if ( _rAnchorFrame.IsLayoutFrame() )
    {
        pObjFormatter = SwObjectFormatterLayFrame::CreateObjFormatter(
                                        static_cast<SwLayoutFrame&>(_rAnchorFrame),
                                        _rPageFrame, _pLayAction );
    }
    else
    {
        OSL_FAIL( "<SwObjectFormatter::CreateObjFormatter(..)> - unexpected type of anchor frame" );
    }

    return pObjFormatter;
}

/** method to format all floating screen objects at the given anchor frame
*/
bool SwObjectFormatter::FormatObjsAtFrame( SwFrame& _rAnchorFrame,
                                         const SwPageFrame& _rPageFrame,
                                         SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( _rAnchorFrame, _rPageFrame, _pLayAction );

    if ( pObjFormatter )
    {
        // format anchored floating screen objects
        bSuccess = pObjFormatter->DoFormatObjs();
    }
    delete pObjFormatter;

    return bSuccess;
}

/** method to format a given floating screen object
*/
bool SwObjectFormatter::FormatObj( SwAnchoredObject& _rAnchoredObj,
                                   SwFrame* _pAnchorFrame,
                                   const SwPageFrame* _pPageFrame,
                                   SwLayAction* _pLayAction )
{
    bool bSuccess( true );

    OSL_ENSURE( _pAnchorFrame || _rAnchoredObj.GetAnchorFrame(),
            "<SwObjectFormatter::FormatObj(..)> - missing anchor frame" );
    SwFrame& rAnchorFrame = _pAnchorFrame ? *_pAnchorFrame : *(_rAnchoredObj.AnchorFrame());

    OSL_ENSURE( _pPageFrame || rAnchorFrame.FindPageFrame(),
            "<SwObjectFormatter::FormatObj(..)> - missing page frame" );
    const SwPageFrame& rPageFrame = _pPageFrame ? *_pPageFrame : *(rAnchorFrame.FindPageFrame());

    // create corresponding object formatter
    SwObjectFormatter* pObjFormatter =
        SwObjectFormatter::CreateObjFormatter( rAnchorFrame, rPageFrame, _pLayAction );

    if ( pObjFormatter )
    {
        // format given floating screen object
        // --> #i40147# - check for moved forward anchor frame
        bSuccess = pObjFormatter->DoFormatObj( _rAnchoredObj, true );
    }
    delete pObjFormatter;

    return bSuccess;
}

/** helper method for method <FormatObj_(..)> - performs the intrinsic format
    of the layout of the given layout frame and all its lower layout frames.

    #i28701#
    IMPORTANT NOTE:
    Method corresponds to methods <SwLayAction::FormatLayoutFly(..)> and
    <SwLayAction::FormatLayout(..)>. Thus, its code for the formatting have
    to be synchronised.
*/
void SwObjectFormatter::FormatLayout_( SwLayoutFrame& _rLayoutFrame )
{
    _rLayoutFrame.Calc(_rLayoutFrame.getRootFrame()->GetCurrShell()->GetOut());

    SwFrame* pLowerFrame = _rLayoutFrame.Lower();
    while ( pLowerFrame )
    {
        if ( pLowerFrame->IsLayoutFrame() )
        {
            FormatLayout_( *(static_cast<SwLayoutFrame*>(pLowerFrame)) );
        }
        pLowerFrame = pLowerFrame->GetNext();
    }
}

/** helper method for method <FormatObj_(..)> - performs the intrinsic
    format of the content of the given floating screen object.

    #i28701#
*/
void SwObjectFormatter::FormatObjContent( SwAnchoredObject& _rAnchoredObj )
{
    if ( dynamic_cast<const SwFlyFrame*>( &_rAnchoredObj) ==  nullptr )
    {
        // only Writer fly frames have content
        return;
    }

    SwFlyFrame& rFlyFrame = static_cast<SwFlyFrame&>(_rAnchoredObj);
    SwContentFrame* pContent = rFlyFrame.ContainsContent();

    while ( pContent )
    {
        // format content
        pContent->OptCalc();

        // format floating screen objects at content text frame
        // #i23129#, #i36347# - pass correct page frame to
        // the object formatter
        if ( pContent->IsTextFrame() &&
             !SwObjectFormatter::FormatObjsAtFrame( *pContent,
                                                  *(pContent->FindPageFrame()),
                                                  GetLayAction() ) )
        {
            // restart format with first content
            pContent = rFlyFrame.ContainsContent();
            continue;
        }

        // continue with next content
        pContent = pContent->GetNextContentFrame();
    }
}

/** performs the intrinsic format of a given floating screen object and its content.

    #i28701#
*/
void SwObjectFormatter::FormatObj_( SwAnchoredObject& _rAnchoredObj )
{
    // collect anchor object and its 'anchor' page number, if requested
    if ( mpPgNumAndTypeOfAnchors )
    {
        mpPgNumAndTypeOfAnchors->Collect( _rAnchoredObj );
    }

    if ( dynamic_cast<const SwFlyFrame*>( &_rAnchoredObj) !=  nullptr )
    {
        SwFlyFrame& rFlyFrame = static_cast<SwFlyFrame&>(_rAnchoredObj);
        // --> #i34753# - reset flag, which prevents a positioning
        if ( rFlyFrame.IsFlyLayFrame() )
        {
            static_cast<SwFlyLayFrame&>(rFlyFrame).SetNoMakePos( false );
        }

        // #i81146# new loop control
        int nLoopControlRuns = 0;
        const int nLoopControlMax = 15;

        do {
            if ( mpLayAction )
            {
                mpLayAction->FormatLayoutFly( &rFlyFrame );
                // --> consider, if the layout action
                // has to be restarted due to a delete of a page frame.
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
            }
            else
            {
                FormatLayout_( rFlyFrame );
            }
            // --> #i34753# - prevent further positioning, if
            // to-page|to-fly anchored Writer fly frame is already clipped.
            if ( rFlyFrame.IsFlyLayFrame() && rFlyFrame.IsClipped() )
            {
                static_cast<SwFlyLayFrame&>(rFlyFrame).SetNoMakePos( true );
            }
            // #i23129#, #i36347# - pass correct page frame
            // to the object formatter
            SwObjectFormatter::FormatObjsAtFrame( rFlyFrame,
                                                *(rFlyFrame.FindPageFrame()),
                                                mpLayAction );
            if ( mpLayAction )
            {
                mpLayAction->FormatFlyContent( &rFlyFrame );
                // --> consider, if the layout action
                // has to be restarted due to a delete of a page frame.
                if ( mpLayAction->IsAgain() )
                {
                    break;
                }
            }
            else
            {
                FormatObjContent( rFlyFrame );
            }

            if ( ++nLoopControlRuns >= nLoopControlMax )
            {
                OSL_FAIL( "LoopControl in SwObjectFormatter::FormatObj_: Stage 3!!!" );
                rFlyFrame.ValidateThisAndAllLowers( 2 );
                nLoopControlRuns = 0;
            }

        // --> #i57917#
        // stop formatting of anchored object, if restart of layout process is requested.
        } while ( !rFlyFrame.IsValid() &&
                  !_rAnchoredObj.RestartLayoutProcess() &&
                  rFlyFrame.GetAnchorFrame() == &GetAnchorFrame() );
    }
    else if ( dynamic_cast<const SwAnchoredDrawObject*>( &_rAnchoredObj) !=  nullptr )
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
*/
bool SwObjectFormatter::FormatObjsAtFrame_( SwTextFrame* _pMasterTextFrame )
{
    // --> #i26945#
    SwFrame* pAnchorFrame( nullptr );
    if ( GetAnchorFrame().IsTextFrame() &&
         static_cast<SwTextFrame&>(GetAnchorFrame()).IsFollow() &&
         _pMasterTextFrame )
    {
        pAnchorFrame = _pMasterTextFrame;
    }
    else
    {
        pAnchorFrame = &GetAnchorFrame();
    }
    if ( !pAnchorFrame->GetDrawObjs() )
    {
        // nothing to do, if no floating screen object is registered at the anchor frame.
        return true;
    }

    bool bSuccess( true );

    for ( size_t i = 0; i < pAnchorFrame->GetDrawObjs()->size(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = (*pAnchorFrame->GetDrawObjs())[i];

        // check, if object's anchor is on the given page frame or
        // object is registered at the given page frame.
        // --> #i26945# - check, if the anchor character of the
        // anchored object is located in a follow text frame. If this anchor
        // follow text frame differs from the given anchor frame, the given
        // anchor frame is a 'master' text frame of the anchor follow text frame.
        // If the anchor follow text frame is in the same body as its 'master'
        // text frame, do not format the anchored object.
        // E.g., this situation can occur during the table row splitting algorithm.
        SwTextFrame* pAnchorCharFrame = pAnchoredObj->FindAnchorCharFrame();
        const bool bAnchoredAtFollowInSameBodyAsMaster =
                pAnchorCharFrame && pAnchorCharFrame->IsFollow() &&
                pAnchorCharFrame != pAnchoredObj->GetAnchorFrame() &&
                pAnchorCharFrame->FindBodyFrame() ==
                    static_cast<SwTextFrame*>(pAnchoredObj->AnchorFrame())->FindBodyFrame();
        if ( bAnchoredAtFollowInSameBodyAsMaster )
        {
            continue;
        }
        // #i33751#, #i34060# - method <GetPageFrameOfAnchor()>
        // is replaced by method <FindPageFrameOfAnchor()>. It's return value
        // have to be checked.
        SwPageFrame* pPageFrameOfAnchor = pAnchoredObj->FindPageFrameOfAnchor();
        OSL_ENSURE( pPageFrameOfAnchor,
                "<SwObjectFormatter::FormatObjsAtFrame_()> - missing page frame." );
        // --> #i26945#
        if ( pPageFrameOfAnchor && pPageFrameOfAnchor == &mrPageFrame )
        {
            // if format of object fails, stop formatting and pass fail to
            // calling method via the return value.
            if ( !DoFormatObj( *pAnchoredObj ) )
            {
                bSuccess = false;
                break;
            }

            // considering changes at <pAnchorFrame->GetDrawObjs()> during
            // format of the object.
            if ( !pAnchorFrame->GetDrawObjs() ||
                 i > pAnchorFrame->GetDrawObjs()->size() )
            {
                break;
            }
            else
            {
                const size_t nActPosOfObj =
                    pAnchorFrame->GetDrawObjs()->ListPosOf( *pAnchoredObj );
                if ( nActPosOfObj == pAnchorFrame->GetDrawObjs()->size() ||
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
    } // end of loop on <pAnchorFrame->.GetDrawObjs()>

    return bSuccess;
}

/** accessor to collected anchored object

    #i28701#
*/
SwAnchoredObject* SwObjectFormatter::GetCollectedObj( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? (*mpPgNumAndTypeOfAnchors)[_nIndex] : nullptr;
}

/** accessor to 'anchor' page number of collected anchored object

    #i28701#
*/
sal_uInt32 SwObjectFormatter::GetPgNumOfCollected( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->GetPageNum(_nIndex) : 0L;
}

/** accessor to 'anchor' type of collected anchored object

    #i26945#
*/
bool SwObjectFormatter::IsCollectedAnchoredAtMaster( const sal_uInt32 _nIndex )
{
    return mpPgNumAndTypeOfAnchors == nullptr
           || mpPgNumAndTypeOfAnchors->AnchoredAtMaster(_nIndex);
}

/** accessor to total number of collected anchored objects

    #i28701#
*/
sal_uInt32 SwObjectFormatter::CountOfCollected()
{
    return mpPgNumAndTypeOfAnchors ? mpPgNumAndTypeOfAnchors->Count() : 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
