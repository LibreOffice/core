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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_HFFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_HFFRM_HXX

#include <tools/mempool.hxx>
#include "layfrm.hxx"

class SwHeadFootFrm : public SwLayoutFrm
{
protected:
    void FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs);
    void FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs);
    inline bool GetEatSpacing() const; // in hffrm.cxx

public:
    SwHeadFootFrm(SwFrameFormat * pFrm, SwFrm*, sal_uInt16 aType);
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual SwTwips GrowFrm( SwTwips,
                             bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips ShrinkFrm( SwTwips,
                               bool bTst = false, bool bInfo = false ) override;
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const override;
};

/// Header in the document layout, inside a page.
class SwHeaderFrm: public SwHeadFootFrm
{
public:
    SwHeaderFrm( SwFrameFormat* pFrm, SwFrm* pSib ) : SwHeadFootFrm(pFrm, pSib, FRM_HEADER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwHeaderFrm)
};

/// Footer in the document layout, inside a page.
class SwFooterFrm: public SwHeadFootFrm
{
public:
    SwFooterFrm( SwFrameFormat* pFrm, SwFrm* pSib ) : SwHeadFootFrm(pFrm, pSib, FRM_FOOTER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwFooterFrm)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
