/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SC_VBA_CHARTOBJECTS_HXX
#define SC_VBA_CHARTOBJECTS_HXX
#include <ooo/vba/excel/XChartObjects.hpp>
#include <ooo/vba/excel/XChartObject.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include "excelvbahelper.hxx"
#include <hash_map>

typedef CollTestImplHelper< ov::excel::XChartObjects > ChartObjects_BASE;
/* #TODO see if this hash table is 'really' necessary
typedef ::std::hash_map< ::rtl::OUString,
css::uno::Reference< ov::excel::XChartObject >,
    ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString > > aHashTable;
*/

class ScVbaChartObjects : public ChartObjects_BASE
{

    css::uno::Reference< css::table::XTableCharts > xTableCharts;
    css::uno::Reference< css::drawing::XDrawPageSupplier > xDrawPageSupplier;
    // method associated with populating the hashmap ( I'm not convinced this is necessary )
    //css::uno::Reference< ov::excel::XChartObject > putByPersistName( const rtl:::OUString& _sPersistChartName );
public:
    ScVbaChartObjects( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableCharts >& _xTableCharts, const css::uno::Reference< css::drawing::XDrawPageSupplier >&  _xDrawPageSupplier );

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
