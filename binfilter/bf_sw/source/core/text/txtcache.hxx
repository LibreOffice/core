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
#ifndef _TXTCACHE_HXX
#define _TXTCACHE_HXX

#include <sal/types.h>

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#include "swcache.hxx"
namespace binfilter {

class SwParaPortion;
class SwTxtFrm;

class SwTxtLine : public SwCacheObj
{
    SwParaPortion *pLine;

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtLine)

    SwTxtLine( SwTxtFrm *pFrm, SwParaPortion *pNew = 0 );
    virtual ~SwTxtLine();

    inline 		 SwParaPortion *GetPara()		{ return pLine; }
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

} //namespace binfilter
#endif
