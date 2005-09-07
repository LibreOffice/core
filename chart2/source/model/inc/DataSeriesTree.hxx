/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesTree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:51:35 $
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
#ifndef CHART_DATASERIESTREE_HXX
#define CHART_DATASERIESTREE_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIESTREEPARENT_HPP_
#include <com/sun/star/chart2/XDataSeriesTreeParent.hpp>
#endif

#include <vector>

namespace chart
{

class DataSeriesTree : public
    ::cppu::WeakImplHelper2<
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XDataSeriesTreeParent >
{
public:
    DataSeriesTree( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSeriesTree();

    /// declare XServiceInfo methods
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSeriesTree )

protected:

    // ____ XDataSeriesTreeParent ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode > >
        SAL_CALL getChildren()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode >& aNode )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChild(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeriesTreeNode >& aNode )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XDataSeriesTreeNode ____

private:
    typedef ::std::vector< ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeriesTreeNode > >
        m_tChildType;

    m_tChildType   m_aChildren;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
};

} //  namespace chart

// CHART_DATASERIESTREE_HXX
#endif
