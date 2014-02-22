/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tocntntanchoredobjectposition.hxx>
#include <anchoredobject.hxx>
#include <frame.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <sectfrm.hxx>

#include <tabfrm.hxx>
#include "rootfrm.hxx"
#include "viewopt.hxx"
#include "viewsh.hxx"
#include <frmfmt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/svdobj.hxx>
#include <pam.hxx>
#include <environmentofanchoredobject.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>

using namespace objectpositioning;
using namespace ::com::sun::star;


SwToCntntAnchoredObjectPosition::SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrm( 0 ),
      
      maOffsetToFrmAnchorPos( Point() ),
      mbAnchorToChar ( false ),
      mpToCharOrientFrm( 0 ),
      mpToCharRect( 0 ),
      
      mnToCharTopOfLine( 0 )
{}

SwToCntntAnchoredObjectPosition::~SwToCntntAnchoredObjectPosition()
{}

bool SwToCntntAnchoredObjectPosition::IsAnchoredToChar() const
{
    return mbAnchorToChar;
}

const SwFrm* SwToCntntAnchoredObjectPosition::ToCharOrientFrm() const
{
    return mpToCharOrientFrm;
}

const SwRect* SwToCntntAnchoredObjectPosition::ToCharRect() const
{
    return mpToCharRect;
}


SwTwips SwToCntntAnchoredObjectPosition::ToCharTopOfLine() const
{
    return mnToCharTopOfLine;
}

SwTxtFrm& SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() const
{
    OSL_ENSURE( GetAnchorFrm().ISA(SwTxtFrm),
            "SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() - wrong anchor frame type" );

    return static_cast<SwTxtFrm&>(GetAnchorFrm());
}


static bool lcl_DoesVertPosFits( const SwTwips _nRelPosY,
                          const SwTwips _nAvail,
                          const SwLayoutFrm* _pUpperOfOrientFrm,
                          const bool _bBrowse,
                          const bool _bGrowInTable,
                          SwLayoutFrm*& _orpLayoutFrmToGrow )
{
    bool bVertPosFits = false;

    if ( _nRelPosY <= _nAvail )
    {
        bVertPosFits = true;
    }
    else if ( _bBrowse )
    {
        if ( _pUpperOfOrientFrm->IsInSct() )
        {
            SwSectionFrm* pSctFrm =
                    const_cast<SwSectionFrm*>(_pUpperOfOrientFrm->FindSctFrm());
            bVertPosFits = pSctFrm->GetUpper()->Grow( _nRelPosY - _nAvail, sal_True ) > 0;
            
        }
        else
        {
            bVertPosFits = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, sal_True ) > 0;
            if ( bVertPosFits )
                _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
        }
    }
    else if ( _pUpperOfOrientFrm->IsInTab() && _bGrowInTable )
    {
        
        
        const SwTwips nTwipsGrown = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, sal_True );
        bVertPosFits = ( nTwipsGrown == ( _nRelPosY - _nAvail ) );
        if ( bVertPosFits )
            _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
    }

    return bVertPosFits;
}

void SwToCntntAnchoredObjectPosition::CalcPosition()
{
    
    const SwFrmFmt& rFrmFmt = GetFrmFmt();

    
    
    const SwFrm* pFooter = GetAnchorFrm().FindFooterOrHeader();
    if ( pFooter && !pFooter->IsFooterFrm() )
        pFooter = NULL;

    
    
    bool bBrowse = GetAnchorFrm().IsInDocBody() && !GetAnchorFrm().IsInTab();
    if( bBrowse )
    {
        const SwViewShell *pSh = GetAnchorFrm().getRootFrm()->GetCurrShell();
        if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
            bBrowse = false;
    }

    
    const SvxLRSpaceItem &rLR = rFrmFmt.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrmFmt.GetULSpace();

    
    const SwFmtSurround& rSurround = rFrmFmt.GetSurround();
    const bool bNoSurround = rSurround.GetSurround() == SURROUND_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;

    
    SwEnvironmentOfAnchoredObject aEnvOfObj( DoesObjFollowsTextFlow() );

    
    const bool bGrow = DoesObjFollowsTextFlow() &&
                       ( !GetAnchorFrm().IsInTab() ||
                         !rFrmFmt.GetFrmSize().GetHeightPercent() );

    
    const SwTxtFrm& rAnchorTxtFrm = GetAnchorTxtFrm();
    SWRECTFN( (&rAnchorTxtFrm) )

    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    
    
    
    Point aRelPos( GetAnchoredObj().GetCurrRelPos() );

    SwTwips nRelDiff = 0;

    bool bMoveable = rAnchorTxtFrm.IsMoveable();

    
    const SwTxtFrm* pOrientFrm = &rAnchorTxtFrm;
    const SwTxtFrm* pAnchorFrmForVertPos = &rAnchorTxtFrm;
    {
        
        
        mbAnchorToChar = (FLY_AT_CHAR == rFrmFmt.GetAnchor().GetAnchorId());
        if ( mbAnchorToChar )
        {
            const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();
            
            
            
            if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                   !GetAnchoredObj().GetLastCharRect().Width() ) ||
                 !GetAnchoredObj().GetLastTopOfLine() )
            {
                
                
                
                GetAnchoredObj().CheckCharRectAndTopOfLine();

                
                
                if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                       !GetAnchoredObj().GetLastCharRect().Width() ) ||
                     !GetAnchoredObj().GetLastTopOfLine() )
                {
                    
                    if ( !mpVertPosOrientFrm )
                    {
                        mpVertPosOrientFrm = rAnchorTxtFrm.GetUpper();
                    }
                    return;
                }
            }
            mpToCharRect = &(GetAnchoredObj().GetLastCharRect());
            
            mnToCharTopOfLine = GetAnchoredObj().GetLastTopOfLine();
            pOrientFrm = &(const_cast<SwTxtFrm&>(rAnchorTxtFrm).GetFrmAtOfst(
                                rAnch.GetCntntAnchor()->nContent.GetIndex() ) );
            mpToCharOrientFrm = pOrientFrm;
        }
    }
    SWREFRESHFN( pOrientFrm )

    
    {

        
        SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );

        
        
        const SwLayoutFrm& rPageAlignLayFrm =
                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pOrientFrm );

        if ( aVert.GetVertOrient() != text::VertOrientation::NONE )
        {
            
            
            SwTwips nAlignAreaHeight;
            SwTwips nAlignAreaOffset;
            _GetVertAlignmentValues( *pOrientFrm, rPageAlignLayFrm,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            
            SwTwips nRelPosY = nAlignAreaOffset;
            const SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();
            
            const SwTwips nUpperSpace = bVert
                                        ? ( bVertL2R
                                            ? rLR.GetLeft()
                                            : rLR.GetRight() )
                                        : rUL.GetUpper();
            
            const SwTwips nLowerSpace = bVert
                                        ? ( bVertL2R
                                            ? rLR.GetLeft()
                                            : rLR.GetRight() )
                                        : rUL.GetLower();
            switch ( aVert.GetVertOrient() )
            {
                case text::VertOrientation::CHAR_BOTTOM:
                {
                    if ( mbAnchorToChar )
                    {
                        
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                        
                        if ( bVert && !bVertL2R )
                        {
                            nRelPosY += nObjHeight;
                        }
                        break;
                    }
                }
                
                case text::VertOrientation::TOP:
                {
                    
                    
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        nRelPosY += nUpperSpace;
                    }
                }
                break;
                
                case text::VertOrientation::LINE_TOP:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case text::VertOrientation::CENTER:
                {
                    nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                }
                break;
                
                case text::VertOrientation::LINE_CENTER:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                    }
                    else
                    {
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case text::VertOrientation::BOTTOM:
                {
                    if ( ( aVert.GetRelationOrient() == text::RelOrientation::FRAME ||
                           aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ) &&
                         bNoSurround )
                    {
                        
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                    }
                    else
                    {
                        
                        
                        if ( mbAnchorToChar &&
                             aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                        {
                            nRelPosY += nUpperSpace;
                        }
                        else
                        {
                            nRelPosY += nAlignAreaHeight -
                                        ( nObjHeight + nLowerSpace );
                        }
                    }
                }
                break;
                
                case text::VertOrientation::LINE_BOTTOM:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY += nUpperSpace;
                    }
                    else
                    {
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                default:
                break;
            }

            
            
            
            
            if ( DoesObjFollowsTextFlow() && pOrientFrm != &rAnchorTxtFrm )
            {
                
                
                const SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                      _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert ) );
            }

            
            
            {
                const SwTwips nTopOfAnch =
                                _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                const SwLayoutFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm(
                                            *(pOrientFrm->GetUpper()) );
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrm, nRelPosY,
                                              DoesObjFollowsTextFlow(),
                                              bCheckBottom );
            }

            
            
            {
                
                SwTwips nAttrRelPosY = nRelPosY - nAlignAreaOffset;
                
                if ( nAttrRelPosY != aVert.GetPos() )
                {
                    aVert.SetPos( nAttrRelPosY );
                    const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                    const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aVert );
                    const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
                }
            }

            
            
            
            if ( bVert )
            {
                aRelPos.X() = nRelPosY;
                maOffsetToFrmAnchorPos.X() = nAlignAreaOffset;
            }
            else
            {
                aRelPos.Y() = nRelPosY;
                maOffsetToFrmAnchorPos.Y() = nAlignAreaOffset;
            }
        }

        
        
        
        
        const SwLayoutFrm* pUpperOfOrientFrm = 0L;
        {
            
            
            
            
            if ( &rAnchorTxtFrm == pOrientFrm ||
                 ( rAnchorTxtFrm.FindPageFrm() == pOrientFrm->FindPageFrm() &&
                   aVert.GetVertOrient() == text::VertOrientation::NONE &&
                   aVert.GetRelationOrient() != text::RelOrientation::CHAR &&
                   aVert.GetRelationOrient() != text::RelOrientation::TEXT_LINE ) )
            {
                pUpperOfOrientFrm = rAnchorTxtFrm.GetUpper();
                pAnchorFrmForVertPos = &rAnchorTxtFrm;
            }
            else
            {
                pUpperOfOrientFrm = pOrientFrm->GetUpper();
                pAnchorFrmForVertPos = pOrientFrm;
            }
        }

        
        
        
        if ( pUpperOfOrientFrm->IsInSct() )
        {
            const SwSectionFrm* pSctFrm = pUpperOfOrientFrm->FindSctFrm();
            const bool bIgnoreSection = pUpperOfOrientFrm->IsSctFrm() ||
                                        ( pSctFrm->Lower()->IsColumnFrm() &&
                                          !pSctFrm->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrm = pSctFrm->GetUpper();
        }

        if ( aVert.GetVertOrient() == text::VertOrientation::NONE )
        {
            
            
            SwTwips nRelPosY = 0;
            
            
            
            SwTwips nVertOffsetToFrmAnchorPos( 0L );
            
            
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                   aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE ) )
            {
                
                
                SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                if ( aVert.GetRelationOrient() == text::RelOrientation::CHAR )
                {
                    nVertOffsetToFrmAnchorPos = (*fnRect->fnYDiff)(
                                        (ToCharRect()->*fnRect->fnGetBottom)(),
                                        nTopOfOrient );
                }
                else
                {
                    nVertOffsetToFrmAnchorPos = (*fnRect->fnYDiff)( ToCharTopOfLine(),
                                                                    nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrmAnchorPos - aVert.GetPos();
            }
            else
            {
                
                
                
                nVertOffsetToFrmAnchorPos = 0L;
                
                
                const SwTwips nTopOfOrient =
                        _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
                
                
                if ( aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
                {
                    
                    SwTwips nTopMargin = (pAnchorFrmForVertPos->*fnRect->fnGetTopMargin)();
                    if ( pAnchorFrmForVertPos->IsTxtFrm() )
                    {
                        nTopMargin -= static_cast<const SwTxtFrm*>(pAnchorFrmForVertPos)->
                            GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
                    }
                    nVertOffsetToFrmAnchorPos += nTopMargin;
                }
                
                
                
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    nVertOffsetToFrmAnchorPos += (*fnRect->fnYDiff)(
                                    (rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                                    nTopOfOrient );
                }
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    SwRect aPgPrtRect( rPageAlignLayFrm.Frm() );
                    if ( rPageAlignLayFrm.IsPageFrm() )
                    {
                        aPgPrtRect =
                            static_cast<const SwPageFrm&>(rPageAlignLayFrm).PrtWithoutHeaderAndFooter();
                    }
                    nVertOffsetToFrmAnchorPos += (*fnRect->fnYDiff)(
                                                (aPgPrtRect.*fnRect->fnGetTop)(),
                                                nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrmAnchorPos + aVert.GetPos();
            }

            
            
            
            
            
            

            
            
            if ( bVert )
                maOffsetToFrmAnchorPos.X() = nVertOffsetToFrmAnchorPos;
            else
                maOffsetToFrmAnchorPos.Y() = nVertOffsetToFrmAnchorPos;
            
            
            const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
            if( nRelPosY <= 0 )
            {
                
                
                const SwLayoutFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                
                
                
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrm, nRelPosY,
                                              DoesObjFollowsTextFlow(),
                                              bCheckBottom );
                if ( bVert )
                    aRelPos.X() = nRelPosY;
                else
                    aRelPos.Y() = nRelPosY;
            }
            else
            {
                SWREFRESHFN( pAnchorFrmForVertPos )
                SwTwips nAvail =
                    (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                        nTopOfAnch );
                const bool bInFtn = pAnchorFrmForVertPos->IsInFtn();
                while ( nRelPosY )
                {
                    
                    
                    
                    SwLayoutFrm* pLayoutFrmToGrow = 0L;
                    const bool bDoesVertPosFits = lcl_DoesVertPosFits(
                            nRelPosY, nAvail, pUpperOfOrientFrm, bBrowse,
                            bGrow, pLayoutFrmToGrow );

                    if ( bDoesVertPosFits )
                    {
                        SwTwips nTmpRelPosY =
                            (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                                nTopOfAnch ) -
                            nAvail + nRelPosY;
                        
                        
                        const SwFrm& rVertEnvironLayFrm =
                            aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                        
                        
                        
                        const bool bCheckBottom = !DoesObjFollowsTextFlow();
                        nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                         rVertEnvironLayFrm,
                                                         nTmpRelPosY,
                                                         DoesObjFollowsTextFlow(),
                                                         bCheckBottom );
                        if ( bVert )
                            aRelPos.X() = nTmpRelPosY;
                        else
                            aRelPos.Y() = nTmpRelPosY;

                        
                        
                        
                        if ( pLayoutFrmToGrow )
                        {
                            pLayoutFrmToGrow->Grow( nRelPosY - nAvail );
                        }
                        nRelPosY = 0;
                    }
                    else
                    {
                        
                        
                        
                        if ( DoesObjFollowsTextFlow() &&
                             !( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                                aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA ) &&
                             !GetAnchorFrm().IsInTab() )
                        {
                            if ( bMoveable )
                            {
                                
                                nRelPosY -= nAvail;
                                MakePageType eMakePage = bInFtn ? MAKEPAGE_NONE
                                                                : MAKEPAGE_APPEND;
                                const bool bInSct = pUpperOfOrientFrm->IsInSct();
                                if( bInSct )
                                    eMakePage = MAKEPAGE_NOSECTION;

                                const SwLayoutFrm* pTmp =
                                    pUpperOfOrientFrm->GetLeaf( eMakePage, sal_True, &rAnchorTxtFrm );
                                if ( pTmp &&
                                     ( !bInSct ||
                                       pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pTmp->FindSctFrm() ) ) )
                                {
                                    pUpperOfOrientFrm = pTmp;
                                    bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                                    SWREFRESHFN( pUpperOfOrientFrm )
                                    nAvail = (pUpperOfOrientFrm->Prt().*fnRect->fnGetHeight)();
                                }
                                else
                                {
                                    
                                    
                                    
                                    
                                    
                                    if( bInSct )
                                    {
                                        const SwFrm* pSct = pUpperOfOrientFrm->FindSctFrm();
                                        pUpperOfOrientFrm = pSct->GetUpper();
                                        nAvail = (*fnRect->fnYDiff)(
                                                   (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                                   (pSct->*fnRect->fnGetPrtBottom)() );
                                    }
                                    else
                                    {
#if OSL_DEBUG_LEVEL > 1
                                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - code under investigation by OD, please inform OD about this assertion!" );
#endif
                                        nRelDiff = nRelPosY;
                                        nRelPosY = 0;
                                    }
                                }
                            }
                            else
                            {
                                nRelPosY = 0;
                            }
                        }
                        else
                        {
                            
                            
                            const SwFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                            nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                          rVertEnvironLayFrm,
                                                          nRelPosY,
                                                          DoesObjFollowsTextFlow() );
                            if( bVert )
                                aRelPos.X() = nRelPosY;
                            else
                                aRelPos.Y() = nRelPosY;
                            nRelPosY = 0;
                        }
                    }
                } 
            } 
        } 

        
        
        
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
        if( bVert )
        {
            
            if ( !bVertL2R )
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                             ( aRelPos.X() - nRelDiff ) -
                                             aObjBoundRect.Width() );
            }
            else
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch +
                                             ( aRelPos.X() - nRelDiff ) );
            }
        }
        else
        {
            GetAnchoredObj().SetObjTop( nTopOfAnch +
                                        ( aRelPos.Y() - nRelDiff ) );
        }

        
        
        
        
        if ( pUpperOfOrientFrm->IsInSct() )
        {
            const SwSectionFrm* pSctFrm = pUpperOfOrientFrm->FindSctFrm();
            const bool bIgnoreSection = pUpperOfOrientFrm->IsSctFrm() ||
                                        ( pSctFrm->Lower()->IsColumnFrm() &&
                                          !pSctFrm->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrm = pSctFrm->GetUpper();
        }
        SwTwips nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                          (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
        if( nDist < 0 )
        {
            
            
            
            SwLayoutFrm* pLayoutFrmToGrow = 0L;
            if ( bBrowse && rAnchorTxtFrm.IsMoveable() )
            {
                if ( pUpperOfOrientFrm->IsInSct() )
                {
                    pLayoutFrmToGrow = const_cast<SwLayoutFrm*>(
                                    pUpperOfOrientFrm->FindSctFrm()->GetUpper());
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pLayoutFrmToGrow->*fnRect->fnGetPrtBottom)() );
                    if ( nDist >= 0 )
                    {
                        pLayoutFrmToGrow = 0L;
                    }
                }
                else
                {
                    pLayoutFrmToGrow =
                                    const_cast<SwLayoutFrm*>(pUpperOfOrientFrm);
                }
            }
            else if ( rAnchorTxtFrm.IsInTab() && bGrow )
            {
                pLayoutFrmToGrow = const_cast<SwLayoutFrm*>(pUpperOfOrientFrm);
            }
            if ( pLayoutFrmToGrow )
            {
                pLayoutFrmToGrow->Grow( -nDist );
            }
        }

        if ( DoesObjFollowsTextFlow() &&
             !( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA ) )
        {

            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
            
            
            
            if ( nDist < 0 && pOrientFrm->IsInTab() )
            {
                
                
                
                
                if ( pOrientFrm == &rAnchorTxtFrm &&
                     !pOrientFrm->GetFollow() &&
                     !pOrientFrm->GetIndPrev() )
                {
                    const_cast<SwTabFrm*>(pOrientFrm->FindTabFrm())
                                                    ->SetDoesObjsFit( sal_False );
                }
                else
                {
                    SwTwips nTmpRelPosY( 0L );
                    if ( bVert )
                        nTmpRelPosY = aRelPos.X() - nDist;
                    else
                        nTmpRelPosY = aRelPos.Y() + nDist;
                    const SwLayoutFrm& rVertEnvironLayFrm =
                        aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                    nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                     rVertEnvironLayFrm,
                                                     nTmpRelPosY,
                                                     DoesObjFollowsTextFlow(),
                                                     false );
                    if ( bVert )
                    {
                        aRelPos.X() = nTmpRelPosY;
                        
                        if ( !bVertL2R )
                        {
                            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                                         aRelPos.X() -
                                                         aObjBoundRect.Width() );
                        }
                        else
                        {
                            GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
                        }
                    }
                    else
                    {
                        aRelPos.Y() = nTmpRelPosY;
                        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
                    }
                    
                    
                    
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
                    if ( nDist < 0 &&
                         pOrientFrm == &rAnchorTxtFrm && !pOrientFrm->GetIndPrev() )
                    {
                        const_cast<SwTabFrm*>(pOrientFrm->FindTabFrm())
                                                        ->SetDoesObjsFit( sal_False );
                    }
                }
            }
            else
            {
                
                const bool bInFtn = rAnchorTxtFrm.IsInFtn();
                while( bMoveable && nDist < 0 )
                {
                    bool bInSct = pUpperOfOrientFrm->IsInSct();
                    if ( bInSct )
                    {
                        const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pTmp->*fnRect->fnGetPrtBottom)() );
                        
                        
                        
                        
                        
                        if ( nDist >= 0 )
                        {
                            break;
                        }
                    }
                    if ( !bInSct &&
                         (GetAnchoredObj().GetObjRect().*fnRect->fnGetTop)() ==
                                 (pUpperOfOrientFrm->*fnRect->fnGetPrtTop)() )
                        
                        break;

                    const SwLayoutFrm* pNextLay = pUpperOfOrientFrm->GetLeaf(
                                    ( bInSct
                                      ? MAKEPAGE_NOSECTION
                                      : ( bInFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND ) ),
                                    sal_True, &rAnchorTxtFrm );
                    
                    
                    
                    if ( bInFtn && pNextLay && !pNextLay->IsFtnFrm() )
                    {
                        pNextLay = 0L;
                    }
                    if ( pNextLay )
                    {
                        SWRECTFNX( pNextLay )
                        if ( !bInSct ||
                             ( pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pNextLay->FindSctFrm() ) &&
                               (pNextLay->Prt().*fnRectX->fnGetHeight)() ) )
                        {
                            SwTwips nTmpRelPosY =
                                (*fnRect->fnYDiff)( (pNextLay->*fnRect->fnGetPrtTop)(),
                                                    nTopOfAnch );
                            if ( bVert )
                                aRelPos.X() = nTmpRelPosY;
                            else
                                aRelPos.Y() = nTmpRelPosY;
                            pUpperOfOrientFrm = pNextLay;
                            SWREFRESHFN( pUpperOfOrientFrm )
                            bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                            if( bVertX )
                            {
                                
                                if ( !bVertL2R )
                                {
                                    GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                                                 aRelPos.X() -
                                                                 aObjBoundRect.Width() );
                                }
                                else
                                {
                                    GetAnchoredObj().SetObjLeft( nTopOfAnch +
                                                                 aRelPos.X() );
                                }
                            }
                            else
                                GetAnchoredObj().SetObjTop( nTopOfAnch +
                                                            aRelPos.Y() );
                            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
                        }
                        
                        else if ( bInSct )
                        {
                            const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                      (pTmp->*fnRect->fnGetPrtBottom)() );
                            if( nDist < 0 )
                                pUpperOfOrientFrm = pTmp;
                            else
                                break;
                        }
                    }
                    else if ( bInSct )
                    {
                        
                        
                        const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pTmp->*fnRect->fnGetPrtBottom)() );
                        if( nDist < 0 )
                            pUpperOfOrientFrm = pTmp;
                        else
                            break;
                    }
                    else
                        bMoveable = false;
                }
            }
        }

        
        mpVertPosOrientFrm = pUpperOfOrientFrm;

    }

    
    {
        
        SwFmtHoriOrient aHori( rFrmFmt.GetHoriOrient() );

        
        
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
        if( bVert )
        {
            
            if ( !bVertL2R )
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                             aRelPos.X() - aObjBoundRect.Width() );
            }
            else
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
            }
        }
        else
            GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );

        
        
        
        const SwFrm* pHoriOrientFrm = DoesObjFollowsTextFlow()
                                      ? &_GetHoriVirtualAnchor( *mpVertPosOrientFrm )
                                      : pOrientFrm;

        
        SwTwips nHoriOffsetToFrmAnchorPos( 0L );
        SwTwips nRelPosX = _CalcRelPosX( *pHoriOrientFrm, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( bVert ? aRelPos.X() : aRelPos.Y() ),
                                         nHoriOffsetToFrmAnchorPos );

        
        
        if ( bVert )
        {
            aRelPos.Y() = nRelPosX;
            maOffsetToFrmAnchorPos.Y() = nHoriOffsetToFrmAnchorPos;
        }
        else
        {
            aRelPos.X() = nRelPosX;
            maOffsetToFrmAnchorPos.X() = nHoriOffsetToFrmAnchorPos;
        }

        
        
        {
            SwTwips nAttrRelPosX = nRelPosX - nHoriOffsetToFrmAnchorPos;
            if ( aHori.GetHoriOrient() != text::HoriOrientation::NONE &&
                 aHori.GetPos() != nAttrRelPosX )
            {
                aHori.SetPos( nAttrRelPosX );
                const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aHori );
                const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
            }
        }
    }

    
    const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
    if( bVert )
    {
        
        if ( !bVertL2R )
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                         aRelPos.X() - aObjBoundRect.Width() );
        }
        else
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
        }
        GetAnchoredObj().SetObjTop( rAnchorTxtFrm.Frm().Top() +
                                    aRelPos.Y() );
    }
    else
    {
        GetAnchoredObj().SetObjLeft( rAnchorTxtFrm.Frm().Left() +
                                     aRelPos.X() );
        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
    }

    
    GetAnchoredObj().SetCurrRelPos( aRelPos );
}

/** determine frame for horizontal position */
const SwFrm& SwToCntntAnchoredObjectPosition::_GetHoriVirtualAnchor(
                                        const SwLayoutFrm& _rProposedFrm ) const
{
    const SwFrm* pHoriVirtAnchFrm = &_rProposedFrm;

    
    
    
    const SwFrm* pFrm = _rProposedFrm.Lower();
    while ( pFrm )
    {
        if ( pFrm->IsCntntFrm() &&
             GetAnchorTxtFrm().IsAnFollow( static_cast<const SwCntntFrm*>(pFrm) ) )
        {
            pHoriVirtAnchFrm = pFrm;
            break;
        }
        pFrm = pFrm->GetNext();
    }

    return *pHoriVirtAnchFrm;
}

const SwLayoutFrm& SwToCntntAnchoredObjectPosition::GetVertPosOrientFrm() const
{
    return *mpVertPosOrientFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
