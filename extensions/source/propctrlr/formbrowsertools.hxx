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



#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#define _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <rtl/ustring.hxx>

#include <functional>
#include <set>

//............................................................................
namespace pcr
{
//............................................................................

    ::rtl::OUString GetUIHeadlineName(sal_Int16 _nClassId, const ::com::sun::star::uno::Any& _rUnoObject);
    sal_Int16 classifyComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

    //========================================================================
    struct FindPropertyByHandle : public ::std::unary_function< ::com::sun::star::beans::Property, bool >
    {
    private:
        sal_Int32 m_nId;

    public:
        FindPropertyByHandle( sal_Int32 _nId ) : m_nId ( _nId ) { }
        bool operator()( const ::com::sun::star::beans::Property& _rProp ) const
        {
            return m_nId == _rProp.Handle;
        }
    };

    //========================================================================
    struct FindPropertyByName : public ::std::unary_function< ::com::sun::star::beans::Property, bool >
    {
    private:
        ::rtl::OUString m_sName;

    public:
        FindPropertyByName( const ::rtl::OUString& _rName ) : m_sName( _rName ) { }
        bool operator()( const ::com::sun::star::beans::Property& _rProp ) const
        {
            return m_sName == _rProp.Name;
        }
    };

    //========================================================================
    struct PropertyLessByName
                :public ::std::binary_function  <   ::com::sun::star::beans::Property,
                                                    ::com::sun::star::beans::Property,
                                                    bool
                                                >
    {
        bool operator() (::com::sun::star::beans::Property _rLhs, ::com::sun::star::beans::Property _rRhs) const
        {
            return _rLhs.Name < _rRhs.Name ? true : false;
        }
    };

    //========================================================================
    struct TypeLessByName
                :public ::std::binary_function  <   ::com::sun::star::uno::Type,
                                                    ::com::sun::star::uno::Type,
                                                    bool
                                                >
    {
        bool operator() (::com::sun::star::uno::Type _rLhs, ::com::sun::star::uno::Type _rRhs) const
        {
            return _rLhs.getTypeName() < _rRhs.getTypeName() ? true : false;
        }
    };

    //========================================================================
    typedef ::std::set< ::com::sun::star::beans::Property, PropertyLessByName > PropertyBag;

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_

