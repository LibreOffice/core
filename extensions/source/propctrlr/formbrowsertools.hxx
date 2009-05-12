/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formbrowsertools.hxx,v $
 * $Revision: 1.6 $
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

