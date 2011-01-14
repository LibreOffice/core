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

#ifndef PROPERTYSTORAGE_HXX
#define PROPERTYSTORAGE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Any.hxx>
/** === end UNO includes === **/

#include <tools/solar.h>

#include <boost/shared_ptr.hpp>
#include <map>

class SfxItemSet;

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= PropertyStorage
    //====================================================================
    class SAL_NO_VTABLE PropertyStorage
    {
    public:
        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const = 0;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue ) = 0;

        virtual ~PropertyStorage();
    };

    typedef ::boost::shared_ptr< PropertyStorage >      PPropertyStorage;
    typedef ::std::map< sal_Int32, PPropertyStorage >   PropertyValues;

    //====================================================================
    //= SetItemPropertyStorage
    //====================================================================
    typedef sal_uInt16  ItemId;

    /** a PropertyStorage implementation which stores the value in an item set
    */
    class SetItemPropertyStorage : public PropertyStorage
    {
    public:
        SetItemPropertyStorage( SfxItemSet& _rItemSet, const ItemId _nItemID )
            :m_rItemSet( _rItemSet )
            ,m_nItemID( _nItemID )
        {
        }

        virtual ~SetItemPropertyStorage()
        {
        }

        virtual void getPropertyValue( ::com::sun::star::uno::Any& _out_rValue ) const;
        virtual void setPropertyValue( const ::com::sun::star::uno::Any& _rValue );

    private:
        SfxItemSet&     m_rItemSet;
        const ItemId    m_nItemID;
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // PROPERTYSTORAGE_HXX
