/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formbrowsertools.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:22:55 $
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

#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#define _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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

