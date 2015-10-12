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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_CNTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_CNTFRM_HXX

#include "frame.hxx"
#include "flowfrm.hxx"
#include "cshtyp.hxx"
#include "node.hxx"

class SwLayoutFrm;
class SwContentNode;
class SwBorderAttrs;
class SwAttrSetChg;
class SwTextFrm;

// implemented in cntfrm.cxx, used in cntfrm.cxx and crsrsh.cxx
extern bool GetFrmInPage( const SwContentFrm*, SwWhichPage, SwPosPage, SwPaM* );

class SwContentFrm: public SwFrm, public SwFlowFrm
{
    friend void MakeNxt( SwFrm *pFrm, SwFrm *pNxt );    // calls MakePrtArea

    // parameter <bObjsInNewUpper>  indicates that objects exist in remaining
    // area of new upper
    bool _WouldFit( SwTwips nSpace,
                    SwLayoutFrm *pNewUpper,
                    bool bTstMove,
                    const bool bObjsInNewUpper );

    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );

    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool, bool& ) override;

    const SwContentFrm* ImplGetNextContentFrm( bool bFwd ) const;

protected:
    bool MakePrtArea( const SwBorderAttrs & );

    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) override;

    SwContentFrm( SwContentNode * const, SwFrm* );

    virtual void DestroyImpl() override;
    virtual ~SwContentFrm();

public:
    TYPEINFO_OVERRIDE(); // already in base class

    virtual void Cut() override;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) override;

    inline const SwContentNode *GetNode() const;
    inline       SwContentNode *GetNode();

    inline const SwContentFrm *GetFollow() const;
    inline       SwContentFrm *GetFollow();
    inline const SwContentFrm *GetPrecede() const;
    SwTextFrm* FindMaster() const;

    // layout dependent cursor travelling
    virtual bool LeftMargin( SwPaM * ) const;
    virtual bool RightMargin( SwPaM *, bool bAPI = false ) const;
    virtual bool UnitUp( SwPaM *, const SwTwips nOffset = 0,
                         bool bSetInReadOnly = false ) const;
    virtual bool UnitDown( SwPaM *, const SwTwips nOffset = 0,
                           bool bSetInReadOnly = false ) const;

    // nMaxHeight is the required height
    // bSplit indicates that the paragraph has to be split
    // bTst indicates that we are currently doing a test formatting
    virtual bool WouldFit( SwTwips &nMaxHeight, bool &bSplit, bool bTst );

    bool MoveFootnoteCntFwd( bool, SwFootnoteBossFrm* ); // called by MoveFwd if content

    inline  SwContentFrm* GetNextContentFrm() const;
    inline  SwContentFrm* GetPrevContentFrm() const;
    static bool CalcLowers( SwLayoutFrm* pLay, const SwLayoutFrm* pDontLeave, long nBottom, bool bSkipRowSpanCells );
    void RegisterToNode( SwContentNode& );
};

inline SwContentFrm* SwContentFrm::GetNextContentFrm() const
{
    if ( GetNext() && GetNext()->IsContentFrm() )
        return const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(GetNext()));
    else
        return const_cast<SwContentFrm*>(ImplGetNextContentFrm( true ));
}

inline SwContentFrm* SwContentFrm::GetPrevContentFrm() const
{
    if ( GetPrev() && GetPrev()->IsContentFrm() )
        return const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(GetPrev()));
    else
        return const_cast<SwContentFrm*>(ImplGetNextContentFrm( false ));
}

inline SwContentNode *SwContentFrm::GetNode()
{
    return static_cast< SwContentNode* >( GetDep() );
}
inline const SwContentNode *SwContentFrm::GetNode() const
{
    return static_cast< const SwContentNode* >( GetDep() );
}

inline const SwContentFrm *SwContentFrm::GetFollow() const
{
    return static_cast<const SwContentFrm*>(SwFlowFrm::GetFollow());
}
inline SwContentFrm *SwContentFrm::GetFollow()
{
    return static_cast<SwContentFrm*>(SwFlowFrm::GetFollow());
}

inline const SwContentFrm *SwContentFrm::GetPrecede() const
{
    return static_cast<const SwContentFrm*>(SwFlowFrm::GetPrecede());
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
