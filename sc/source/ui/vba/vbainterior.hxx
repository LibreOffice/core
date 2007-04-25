/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbainterior.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:07:55 $
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
#ifndef SC_VBA_INTERIOR_HXX
#define SC_VBA_INTERIOR_HXX

#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/excel/XInterior.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "vbarange.hxx"

#include <com/sun/star/script/XInvocation.hpp>

class ScDocument;

typedef ::cppu::WeakImplHelper1< oo::excel::XInterior > ScVbaInterior_BASE;

class ScVbaInterior :  public ScVbaInterior_BASE
{
    css::uno::Reference< css::beans::XPropertySet > m_xProps;
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
    ScDocument* m_pScDoc;

        css::uno::Reference< css::container::XIndexAccess > getPalette();
public:
        ScVbaInterior( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                 const css::uno::Reference< css::beans::XPropertySet >& xProps, ScDocument* pScDoc ) throw ( css::lang::IllegalArgumentException);

        virtual ~ScVbaInterior(){}

    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException) ;

    virtual css::uno::Any SAL_CALL getColorIndex() throw ( css::uno::RuntimeException);
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw ( css::uno::RuntimeException );
};
#endif

