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
#ifndef _SVX_UNOMLSTR_HXX
#define _SVX_UNOMLSTR_HXX

#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#include <cppuhelper/implbase1.hxx> 
namespace binfilter {

class SdrObject;

class SvxUnoShapeModifyListener : public ::cppu::WeakAggImplHelper1< ::com::sun::star::util::XModifyListener >
{
    SdrObject*	mpObj;

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


}//end of namespace binfilter
#endif


