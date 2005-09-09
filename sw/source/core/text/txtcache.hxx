/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtcache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:04:35 $
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
#ifndef _TXTCACHE_HXX
#define _TXTCACHE_HXX

#include <sal/types.h>

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#include "swcache.hxx"

class SwParaPortion;
class SwTxtFrm;

class SwTxtLine : public SwCacheObj
{
    SwParaPortion *pLine;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtLine)

    SwTxtLine( SwTxtFrm *pFrm, SwParaPortion *pNew = 0 );
    virtual ~SwTxtLine();

    inline       SwParaPortion *GetPara()       { return pLine; }
    inline const SwParaPortion *GetPara() const { return pLine; }

    inline void SetPara( SwParaPortion *pNew ) { pLine = pNew; }
};


class SwTxtLineAccess : public SwCacheAccess
{

protected:
    virtual SwCacheObj *NewObj();

public:
    SwTxtLineAccess( const SwTxtFrm *pOwner );

    SwParaPortion *GetPara();

    inline SwTxtLine &GetTxtLine();

    virtual sal_Bool IsAvailable() const;
};


inline SwTxtLine &SwTxtLineAccess::GetTxtLine()
{
    return *((SwTxtLine*)Get());
}

#endif
