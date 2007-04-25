/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaseriescollection.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:11:17 $
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
#ifndef SC_VBA_SERIESCOLLECTION_HXX
#define SC_VBA_SERIESCOLLECTION_HXX
#include <org/openoffice/excel/XSeriesCollection.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper2<oo::excel::XSeriesCollection,
css::container::XEnumerationAccess > SeriesCollectionImpl_BASE;

class ScVbaSeriesCollection : public SeriesCollectionImpl_BASE
{

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
public:
    ScVbaSeriesCollection( const css::uno::Reference< css::uno::XComponentContext >& xContext ) : m_xContext(xContext) {}
    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);
};

#endif //SC_VBA_WINDOW_HXX
