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

class SwHeadFootFrame : public SwLayoutFrame
{
protected:
    void FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs);
    void FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs);
    inline bool GetEatSpacing() const; // in hffrm.cxx

public:
    SwHeadFootFrame(SwFrameFormat * pFrame, SwFrame*, sal_uInt16 aType);
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual SwTwips GrowFrame( SwTwips,
                             bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips ShrinkFrame( SwTwips,
                               bool bTst = false, bool bInfo = false ) override;
    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const override;
};

/// Header in the document layout, inside a page.
class SwHeaderFrame: public SwHeadFootFrame
{
public:
    SwHeaderFrame( SwFrameFormat* pFrame, SwFrame* pSib ) : SwHeadFootFrame(pFrame, pSib, FRM_HEADER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwHeaderFrame)
};

/// Footer in the document layout, inside a page.
class SwFooterFrame: public SwHeadFootFrame
{
public:
    SwFooterFrame( SwFrameFormat* pFrame, SwFrame* pSib ) : SwHeadFootFrame(pFrame, pSib, FRM_FOOTER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwFooterFrame)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
