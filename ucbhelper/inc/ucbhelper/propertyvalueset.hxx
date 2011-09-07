/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#define _UCBHELPER_PROPERTYVALUESET_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <cppuhelper/weak.hxx>

#include "osl/mutex.hxx"
#include <ucbhelper/macros.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct PropertyValue;
    class XPropertySet;
} } } }

namespace ucbhelper {

class PropertyValues;

//=========================================================================

/**
  * This class implements the interface XRow. After construction of a valueset
  * the user can append properties ( incl. its values ) to the set. This class
  * is useful when implementing the command "getPropertyValues", because the
  * values to return can easyly appended to a valueset object. That object can
  * directly be returned by the implementation of the command.
  */
class UCBHELPER_DLLPUBLIC PropertyValueSet :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::sdbc::XRow,
                public com::sun::star::sdbc::XColumnLocate
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                     m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter >
                                     m_xTypeConverter;
    osl::Mutex      m_aMutex;
    PropertyValues* m_pValues;
    sal_Bool        m_bWasNull;
    sal_Bool        m_bTriedToGetTypeConverter;

private:
    UCBHELPER_DLLPRIVATE const com::sun::star::uno::Reference<
            com::sun::star::script::XTypeConverter >&
    getTypeConverter();

public:
    PropertyValueSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );
    virtual ~PropertyValueSet();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull()
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::rtl::OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const ::com::sun::star::uno::Reference<
                   ::com::sun::star::container::XNameAccess >& typeMap )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );

    // XColumnLocate
    virtual sal_Int32 SAL_CALL
    findColumn( const ::rtl::OUString& columnName )
        throw( ::com::sun::star::sdbc::SQLException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods
    //////////////////////////////////////////////////////////////////////

    void appendString( const ::rtl::OUString& rPropName, const ::rtl::OUString& rValue );
    void appendString( const sal_Char* pAsciiPropName, const ::rtl::OUString& rValue )
    {
        appendString( ::rtl::OUString::createFromAscii( pAsciiPropName ), rValue );
    }
    void appendString( const ::com::sun::star::beans::Property& rProp, const ::rtl::OUString& rValue )
    {
        appendString( rProp.Name, rValue );
    }

    void appendBoolean( const ::rtl::OUString& rPropName, sal_Bool bValue );
    void appendBoolean( const sal_Char* pAsciiPropName, sal_Bool bValue )
    {
        appendBoolean( ::rtl::OUString::createFromAscii( pAsciiPropName ), bValue );
    }
    void appendBoolean( const ::com::sun::star::beans::Property& rProp, sal_Bool bValue )
    {
        appendBoolean( rProp.Name, bValue );
    }

    void appendLong( const ::rtl::OUString& rPropName, sal_Int64 nValue );
    void appendLong( const sal_Char* pAsciiPropName, sal_Int64 nValue )
    {
        appendLong( ::rtl::OUString::createFromAscii( pAsciiPropName ), nValue );
    }
    void appendLong( const ::com::sun::star::beans::Property& rProp, sal_Int64 nValue )
    {
        appendLong( rProp.Name, nValue );
    }

    void appendTimestamp( const ::rtl::OUString& rPropName, const ::com::sun::star::util::DateTime& rValue );
    void appendTimestamp( const sal_Char* pAsciiPropName, const ::com::sun::star::util::DateTime& rValue )
    {
        appendTimestamp( ::rtl::OUString::createFromAscii( pAsciiPropName ), rValue );
    }
    void appendTimestamp( const ::com::sun::star::beans::Property& rProp, const ::com::sun::star::util::DateTime& rValue )
    {
        appendTimestamp( rProp.Name, rValue );
    }

    void appendObject( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rValue );
    void appendObject( const sal_Char* pAsciiPropName, const ::com::sun::star::uno::Any& rValue )
    {
        appendObject( ::rtl::OUString::createFromAscii( pAsciiPropName ), rValue );
    }
    void appendObject( const ::com::sun::star::beans::Property& rProp, const ::com::sun::star::uno::Any& rValue )
    {
        appendObject( rProp.Name, rValue );
    }

    void appendVoid( const ::rtl::OUString& rPropName );
    void appendVoid( const sal_Char* pAsciiPropName)
    {
        appendVoid( ::rtl::OUString::createFromAscii( pAsciiPropName ) );
    }
    void appendVoid( const ::com::sun::star::beans::Property& rProp )
    {
        appendVoid( rProp.Name );
    }

    /**
      * This method tries to append all property values contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
      */
    void appendPropertySet( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet >& rSet );

    /** This method tries to append a single property value contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
       *    @param  rProperty is the property for that the value shall be obtained
      *         from the given property set.
       *    @return False, if the property value cannot be obtained from the
      *         given property pet. True, otherwise.
       */
    sal_Bool appendPropertySetValue(
                        const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& rSet,
                        const ::com::sun::star::beans::Property& rProperty );
};

}

#endif /* !_UCBHELPER_PROPERTYVALUESET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
