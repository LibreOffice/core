/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_UCBHELPER_PROPERTYVALUESET_HXX
#define INCLUDED_UCBHELPER_PROPERTYVALUESET_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <cppuhelper/weak.hxx>

#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace com { namespace sun { namespace star { namespace script {
    class XTypeConverter;
} } } }

namespace com { namespace sun { namespace star { namespace beans {
    struct PropertyValue;
    class XPropertySet;
} } } }

namespace ucbhelper {

class PropertyValues;



/**
  * This class implements the interface XRow. After construction of a valueset
  * the user can append properties ( incl. its values ) to the set. This class
  * is useful when implementing the command "getPropertyValues", because the
  * values to return can easily appended to a valueset object. That object can
  * directly be returned by the implementation of the command.
  */
class UCBHELPER_DLLPUBLIC PropertyValueSet :
                public cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::sdbc::XRow,
                public css::sdbc::XColumnLocate
{
    css::uno::Reference< css::uno::XComponentContext >   m_xContext;
    css::uno::Reference< css::script::XTypeConverter >   m_xTypeConverter;
    osl::Mutex      m_aMutex;
    PropertyValues* m_pValues;
    bool        m_bWasNull;
    bool        m_bTriedToGetTypeConverter;

private:
    UCBHELPER_DLLPRIVATE const css::uno::Reference< css::script::XTypeConverter >&
    getTypeConverter();

public:
    PropertyValueSet(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~PropertyValueSet();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XRow
    virtual sal_Bool SAL_CALL
    wasNull()
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL
    getString( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
    getBoolean( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int8 SAL_CALL
    getByte( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL
    getShort( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL
    getInt( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int64 SAL_CALL
    getLong( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual float SAL_CALL
    getFloat( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual double SAL_CALL
    getDouble( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getBytes( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::util::Date SAL_CALL
    getDate( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::util::Time SAL_CALL
    getTime( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::util::DateTime SAL_CALL
    getTimestamp( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getBinaryStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::io::XInputStream > SAL_CALL
    getCharacterStream( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Any SAL_CALL
    getObject( sal_Int32 columnIndex,
               const css::uno::Reference<
                   css::container::XNameAccess >& typeMap )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XRef > SAL_CALL
    getRef( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XBlob > SAL_CALL
    getBlob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XClob > SAL_CALL
    getClob( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference<
                css::sdbc::XArray > SAL_CALL
    getArray( sal_Int32 columnIndex )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;

    // XColumnLocate
    virtual sal_Int32 SAL_CALL
    findColumn( const OUString& columnName )
        throw( css::sdbc::SQLException,
               css::uno::RuntimeException, std::exception ) override;


    // Non-interface methods

    void appendString( const OUString& rPropName, const OUString& rValue );
    void appendString( const css::beans::Property& rProp, const OUString& rValue )
    {
        appendString( rProp.Name, rValue );
    }

    void appendBoolean( const OUString& rPropName, bool bValue );
    void appendBoolean( const css::beans::Property& rProp, bool bValue )
    {
        appendBoolean( rProp.Name, bValue );
    }

    void appendLong( const OUString& rPropName, sal_Int64 nValue );
    void appendLong( const css::beans::Property& rProp, sal_Int64 nValue )
    {
        appendLong( rProp.Name, nValue );
    }

    void appendTimestamp( const OUString& rPropName, const css::util::DateTime& rValue );
    void appendTimestamp( const css::beans::Property& rProp, const css::util::DateTime& rValue )
    {
        appendTimestamp( rProp.Name, rValue );
    }

    void appendObject( const OUString& rPropName, const css::uno::Any& rValue );
    void appendObject( const css::beans::Property& rProp, const css::uno::Any& rValue )
    {
        appendObject( rProp.Name, rValue );
    }

    void appendVoid( const OUString& rPropName );
    void appendVoid( const css::beans::Property& rProp )
    {
        appendVoid( rProp.Name );
    }

    /**
      * This method tries to append all property values contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
      */
    void appendPropertySet( const css::uno::Reference< css::beans::XPropertySet >& rSet );

    /** This method tries to append a single property value contained in a
      * property set to the value set.
      *
       *    @param  rSet is a property set containing the property values.
       *    @param  rProperty is the property for that the value shall be obtained
      *         from the given property set.
       *    @return False, if the property value cannot be obtained from the
      *         given property pet. True, otherwise.
       */
    bool appendPropertySetValue(
                        const css::uno::Reference< css::beans::XPropertySet >& rSet,
                        const css::beans::Property& rProperty );
};

}

#endif /* ! INCLUDED_UCBHELPER_PROPERTYVALUESET_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
