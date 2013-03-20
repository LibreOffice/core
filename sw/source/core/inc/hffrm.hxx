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

#pragma once
#if 1

#include <tools/mempool.hxx>
#include "layfrm.hxx"

class SwHeadFootFrm : public SwLayoutFrm
{
protected:
    void FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs);
    void FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs);
    inline sal_Bool GetEatSpacing() const; // in hffrm.cxx

public:
    SwHeadFootFrm(SwFrmFmt * pFrm, SwFrm*, sal_uInt16 aType);
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual SwTwips GrowFrm( SwTwips,
                             sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual SwTwips ShrinkFrm( SwTwips,
                               sal_Bool bTst = sal_False, sal_Bool bInfo = sal_False );
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const;
};

/// Header in the document layout, inside a page.
class SwHeaderFrm: public SwHeadFootFrm
{
public:
    SwHeaderFrm( SwFrmFmt* pFrm, SwFrm* pSib ) : SwHeadFootFrm(pFrm, pSib, FRMC_HEADER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwHeaderFrm)
};

/// Footer in the document layout, inside a page.
class SwFooterFrm: public SwHeadFootFrm
{
public:
    SwFooterFrm( SwFrmFmt* pFrm, SwFrm* pSib ) : SwHeadFootFrm(pFrm, pSib, FRMC_FOOTER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwFooterFrm)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
