/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachartobjects.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:46:49 $
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
#ifndef SC_VBA_CHARTOBJECTS_HXX
#define SC_VBA_CHARTOBJECTS_HXX
#include <org/openoffice/excel/XChartObjects.hpp>
#include <org/openoffice/excel/XChartObject.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include"vbacollectionimpl.hxx"
#include "vbahelper.hxx"
#include <hash_map>

typedef CollTestImplHelper< oo::excel::XChartObjects > ChartObjects_BASE;
/* #TODO see if this hash table is 'really' necessary
typedef ::std::hash_map< ::rtl::OUString,
css::uno::Reference< oo::excel::XChartObject >,
    ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > aHashTable;
*/

class ScVbaChartObjects : public ChartObjects_BASE
{

    css::uno::Reference< css::table::XTableCharts > xTableCharts;
    css::uno::Reference< css::drawing::XDrawPageSupplier > xDrawPageSupplier;
    // method associated with populating the hashmap ( I'm not convinced this is necessary )
    //css::uno::Reference< oo::excel::XChartObject > putByPersistName( const rtl:::OUString& _sPersistChartName );
public:
    ScVbaChartObjects( const css::uno::Reference< oo::vba::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableCharts >& _xTableCharts, const css::uno::Reference< css::drawing::XDrawPageSupplier >&  _xDrawPageSupplier );

    css::uno::Sequence< rtl::OUString > getChartObjectNames() throw( css::script::BasicErrorException );
    void removeByName(const rtl::OUString& _sChartName);

    // XChartObjects
    virtual ::com::sun::star::uno::Any SAL_CALL Add( double Left, double Top, double Width, double Height ) throw (::com::sun::star::script::BasicErrorException);
    virtual void SAL_CALL Delete(  ) throw (::com::sun::star::script::BasicErrorException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    // ChartObjects_BASE
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_WINDOW_HXX
