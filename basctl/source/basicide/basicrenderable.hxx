/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basidesh.hxx,v $
 * $Revision: 1.10 $
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
#ifndef _BASICRENDERABLE_HXX
#define _BASICRENDERABLE_HXX

#include "com/sun/star/view/XRenderable.hpp"
#include "cppuhelper/compbase1.hxx"

class IDEBaseWindow;
class Printer;

namespace basicide
{
class BasicRenderable :
        public cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >
{
    IDEBaseWindow*      mpWindow;
    osl::Mutex          maMutex;

    Printer* getPrinter( const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue >& xOptions );
public:
    BasicRenderable( IDEBaseWindow* pWin )
    : cppu::WeakComponentImplHelper1< com::sun::star::view::XRenderable >( maMutex )
    , mpWindow( pWin )
    {
    }

    virtual ~BasicRenderable();

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount (
        const com::sun::star::uno::Any& aSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue >& xOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> SAL_CALL getRenderer (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL render (
        sal_Int32 nRenderer,
        const com::sun::star::uno::Any& rSelection,
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rxOptions)
        throw (com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);

};

} // namespace

#endif
