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
#ifndef _HFFRM_HXX
#define _HFFRM_HXX

#include <tools/mempool.hxx>

#include "layfrm.hxx"
namespace binfilter {


class SwHeadFootFrm : public SwLayoutFrm
{
protected:
    void FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs);
    void FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs);
    inline BOOL GetEatSpacing() const; // in hffrm.cxx
public:
    SwHeadFootFrm(SwFrmFmt * pFrm, USHORT aType);
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
    virtual SwTwips GrowFrm( SwTwips, 
                             BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips ShrinkFrm( SwTwips, 
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
};

class SwHeaderFrm: public SwHeadFootFrm
{
public:
    SwHeaderFrm( SwFrmFmt* pFrm ) : SwHeadFootFrm(pFrm, FRMC_HEADER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwHeaderFrm)
};

class SwFooterFrm: public SwHeadFootFrm
{
public:
    SwFooterFrm( SwFrmFmt* pFrm ) : SwHeadFootFrm(pFrm, FRMC_FOOTER) {};

    DECL_FIXEDMEMPOOL_NEWDEL(SwFooterFrm)
};

} //namespace binfilter
#endif	//_HFFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
