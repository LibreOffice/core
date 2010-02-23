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
#include <cppuhelper/weak.hxx>

#include "osl/mutex.hxx"
#include <ucbhelper/macros.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct Property;
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
    PropertyValueSet(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyValue >& rValues );
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

    /**
      * This method returns the number of elements of the value set.
      *
      * @return the number of elements of the value set.
      */
    sal_Int32 getLength() const;

    /**
      * This method appends a string to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendString   ( const ::com::sun::star::beans::Property& rProp,
                          const ::rtl::OUString& rValue );

    /**
      * This method appends a boolean to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBoolean  ( const ::com::sun::star::beans::Property& rProp,
                          sal_Bool bValue );

    /**
      * This method appends a byte to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendByte     ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int8 nValue );

    /**
      * This method appends a short to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendShort    ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int16 nValue );

    /**
      * This method appends an int to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendInt      ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int32 nValue );

    /**
      * This method appends a long to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendLong     ( const ::com::sun::star::beans::Property& rProp,
                          sal_Int64 nValue );

    /**
      * This method appends a float to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendFloat    ( const ::com::sun::star::beans::Property& rProp,
                          float nValue );

    /**
      * This method appends a double to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendDouble   ( const ::com::sun::star::beans::Property& rProp,
                          double nValue );

    /**
      * This method appends a byte sequence to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBytes    ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Sequence<
                              sal_Int8 >& rValue );

    /**
      * This method appends a date to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendDate     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::Date& rValue );

    /**
      * This method appends a time to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendTime     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::Time& rValue );

    /**
      * This method appends a timestamp to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendTimestamp( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::util::DateTime& rValue );

    /**
      * This method appends a binary stream to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBinaryStream   ( const ::com::sun::star::beans::Property& rProp,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::io::XInputStream >&
                                        rValue );

    /**
      * This method appends a character stream to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendCharacterStream( const ::com::sun::star::beans::Property& rProp,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::io::XInputStream >&
                                        rValue );

    /**
      * This method appends an object ( any ) to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendObject   ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Any& rValue );

    /**
      * This method appends a ref to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendRef      ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XRef >& rValue );

    /**
      * This method appends a blob to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendBlob     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XBlob >& rValue );

    /**
      * This method appends a clob to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendClob     ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XClob >& rValue );

    /**
      * This method appends an array to the value set.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendArray    ( const ::com::sun::star::beans::Property& rProp,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::sdbc::XArray >& rValue );

    /**
      * This method appends a void value ( a "hole" ) to the value set. This
      * is useful, since void values indicate errors, like non-existing
      * property (-values) etc.
      *
      * @param rProp is the property the value belongs to.
      */
    void appendVoid     ( const ::com::sun::star::beans::Property& rProp );

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
