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
#ifndef _CELLFRM_HXX
#define _CELLFRM_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif

#include "layfrm.hxx"
namespace binfilter {

class SwTableBox;
struct SwCrsrMoveState;
class SwBorderAttrs;

class SwCellFrm: public SwLayoutFrm
{
    const SwTableBox *pTabBox;

protected:
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );

public:
    SwCellFrm( const SwTableBox & );
    ~SwCellFrm();

    virtual	void   Modify( SfxPoolItem*, SfxPoolItem* );

    const SwTableBox *GetTabBox() const { return pTabBox; }
    DECL_FIXEDMEMPOOL_NEWDEL(SwCellFrm)
};

} //namespace binfilter
#endif
