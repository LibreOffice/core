/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hffrm.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:49:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _HFFRM_HXX
#define _HFFRM_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "layfrm.hxx"


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

#endif  //_HFFRM_HXX
