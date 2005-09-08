/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomlstr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:27:24 $
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
#ifndef _SVX_UNOMLSTR_HXX
#define _SVX_UNOMLSTR_HXX

#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#include <cppuhelper/implbase1.hxx>

class SdrObject;

class SvxUnoShapeModifyListener : public ::cppu::WeakAggImplHelper1< ::com::sun::star::util::XModifyListener >
{
    SdrObject*  mpObj;

public:
    SvxUnoShapeModifyListener( SdrObject* pObj ) throw();
    virtual ~SvxUnoShapeModifyListener() throw();

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& aEvent) throw(  ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(  ::com::sun::star::uno::RuntimeException);

    // internal
    void invalidate() throw();
};


#endif


