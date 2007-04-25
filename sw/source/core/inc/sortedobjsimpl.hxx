/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sortedobjsimpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 09:05:53 $
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
#ifndef _SORTEDOBJSIMPL_HXX
#define _SORTEDOBJSIMPL_HXX

#include <vector>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif

class SwAnchoredObject;
class SwTxtFrm;

class SwSortedObjsImpl
{
    private:
        std::vector< SwAnchoredObject* > maSortedObjLst;

    public:
        SwSortedObjsImpl();
        ~SwSortedObjsImpl();

        sal_uInt32 Count() const;

        SwAnchoredObject* operator[]( sal_uInt32 _nIndex );

        bool Insert( SwAnchoredObject& _rAnchoredObj );

        bool Remove( SwAnchoredObject& _rAnchoredObj );

        bool Contains( const SwAnchoredObject& _rAnchoredObj ) const;

        bool Update( SwAnchoredObject& _rAnchoredObj );

        sal_uInt32 ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const;
};

#endif
