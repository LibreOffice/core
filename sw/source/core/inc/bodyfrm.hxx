/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bodyfrm.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:53:57 $
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
#ifndef _BODYFRM_HXX
#define _BODYFRM_HXX

#include <tools/mempool.hxx>
#include "layfrm.hxx"

class SwBorderAttrs;

class SwBodyFrm: public SwLayoutFrm
{
protected:
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );

#if defined ( GCC) && defined ( C272 )
    ~SwBodyFrm();
#endif
public:
    SwBodyFrm( SwFrmFmt* );

    DECL_FIXEDMEMPOOL_NEWDEL(SwBodyFrm)
};

#endif  //_BODYFRM_HXX
