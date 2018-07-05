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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_FAPIHELPER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_FAPIHELPER_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <osl/diagnose.h>
#include <tools/color.hxx>
#include "ftools.hxx"
#include <scdllapi.h>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
} } }

namespace comphelper { class IDocPasswordVerifier; }

// Static helper functions ====================================================

class SfxMedium;
class SfxObjectShell;

/** Static API helper functions. */
class ScfApiHelper
{
public:
    /** Converts a non-empty vector into a UNO sequence containing elements of the same type. */
    template< typename Type >
    static css::uno::Sequence< Type >
                            VectorToSequence( const ::std::vector< Type >& rVector );

    /** Returns the service name provided via the XServiceName interface, or an empty string on error. */
    static OUString GetServiceName( const css::uno::Reference< css::uno::XInterface >& xInt );

    /** Returns the multi service factory from a document shell. */
    static css::uno::Reference< css::lang::XMultiServiceFactory > GetServiceFactory( const SfxObjectShell* pShell );

    /** Creates an instance from the passed service name, using the passed service factory. */
    static css::uno::Reference< css::uno::XInterface > CreateInstance(
                            const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                            const OUString& rServiceName );

    /** Creates an instance from the passed service name, using the service factory of the passed object. */
    static css::uno::Reference< css::uno::XInterface > CreateInstance(
                            const SfxObjectShell* pShell,
                            const OUString& rServiceName );

    /** Creates an instance from the passed service name, using the process service factory. */
    static css::uno::Reference< css::uno::XInterface > CreateInstance( const OUString& rServiceName );

    /** Opens a password dialog and returns the encryption data.
        @return  The encryption data or an empty sequence on 'Cancel' or any error. */
    static css::uno::Sequence< css::beans::NamedValue > QueryEncryptionDataForMedium( SfxMedium& rMedium,
                            ::comphelper::IDocPasswordVerifier& rVerifier,
                            const ::std::vector< OUString >* pDefaultPasswords );
};

template< typename Type >
css::uno::Sequence< Type > ScfApiHelper::VectorToSequence( const ::std::vector< Type >& rVector )
{
    OSL_ENSURE( !rVector.empty(), "ScfApiHelper::VectorToSequence - vector is empty" );
    return css::uno::Sequence<Type>(rVector.data(), static_cast< sal_Int32 >(rVector.size()));
}

// Property sets ==============================================================

/** A wrapper for a UNO property set.

    This class provides functions to silently get and set properties (without
    exceptions, without the need to check validity of the UNO property set).

    An instance is constructed with the reference to a UNO property set or any
    other interface (the constructor will query for the XPropertySet interface
    then). The reference to the property set will be kept as long as the
    instance of this class is alive.

    The functions GetProperties() and SetProperties() try to handle all passed
    values at once, using the XMultiPropertySet interface. If the
    implementation does not support the XMultiPropertySet interface, all
    properties are handled separately in a loop.
 */
class ScfPropertySet
{
public:
    explicit     ScfPropertySet() {}
    /** Constructs a property set wrapper with the passed UNO property set. */
    explicit     ScfPropertySet( const css::uno::Reference< css::beans::XPropertySet > & xPropSet ) { Set( xPropSet ); }
    /** Constructs a property set wrapper after querying the XPropertySet interface. */
    template< typename InterfaceType >
    explicit     ScfPropertySet( const css::uno::Reference< InterfaceType >& xInterface ) { Set( xInterface ); }

                        ~ScfPropertySet();
    //TOOD:
    ScfPropertySet(ScfPropertySet const &) = default;
    ScfPropertySet(ScfPropertySet &&) = default;
    ScfPropertySet & operator =(ScfPropertySet const &) = default;
    ScfPropertySet & operator =(ScfPropertySet &&) = default;

    /** Sets the passed UNO property set and releases the old UNO property set. */
    void                Set( css::uno::Reference< css::beans::XPropertySet > const & xPropSet );
    /** Queries the passed interface for an XPropertySet and releases the old UNO property set. */
    template< typename InterfaceType >
    void         Set( css::uno::Reference< InterfaceType > xInterface )
                            { Set( css::uno::Reference< css::beans::XPropertySet >( xInterface, css::uno::UNO_QUERY ) ); }

    /** Returns true, if the contained XPropertySet interface is valid. */
    bool         Is() const { return mxPropSet.is(); }

    /** Returns the contained XPropertySet interface. */
    const css::uno::Reference< css::beans::XPropertySet >& GetApiPropertySet() const { return mxPropSet; }

    /** Returns the service name provided via the XServiceName interface, or an empty string on error. */
    OUString     GetServiceName() const;

    // Get properties ---------------------------------------------------------

    /** Returns true, if the property set contains the specified property. */
    bool                HasProperty( const OUString& rPropName ) const;

    /** Gets the specified property from the property set.
        @return  true, if the Any could be filled with the property value. */
    bool                GetAnyProperty( css::uno::Any& rValue, const OUString& rPropName ) const;

    /** Gets the specified property from the property set.
        @return  true, if the passed variable could be filled with the property value. */
    template< typename Type >
    bool         GetProperty( Type& rValue, const OUString& rPropName ) const
                            { css::uno::Any aAny; return GetAnyProperty( aAny, rPropName ) && (aAny >>= rValue); }

    /** Gets the specified Boolean property from the property set.
        @return  true = property contains true; false = property contains false or error occurred. */
    bool                GetBoolProperty( const OUString& rPropName ) const;

    /** Gets the specified Boolean property from the property set. */
    OUString       GetStringProperty( const OUString& rPropName ) const;

    /** Gets the specified color property from the property set.
        @return  true, if the passed color variable could be filled with the property value. */
    bool                GetColorProperty( Color& rColor, const OUString& rPropName ) const;

    /** Gets the specified properties from the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                GetProperties( css::uno::Sequence< css::uno::Any >& rValues, const css::uno::Sequence< OUString >& rPropNames ) const;

    // Set properties ---------------------------------------------------------

    /** Puts the passed Any into the property set. */
    void                SetAnyProperty( const OUString& rPropName, const css::uno::Any& rValue );

    /** Puts the passed value into the property set. */
    template< typename Type >
    void         SetProperty( const OUString& rPropName, const Type& rValue )
                            { SetAnyProperty( rPropName, css::uno::makeAny( rValue ) ); }

    /** Puts the passed Boolean value into the property set. */
    void         SetBoolProperty( const OUString& rPropName, bool bValue )
                            { SetAnyProperty( rPropName, css::uno::Any( bValue ) ); }

    /** Puts the passed string into the property set. */
    void         SetStringProperty( const OUString& rPropName, const OUString& rValue )
                            { SetProperty( rPropName, rValue ); }

    /** Puts the passed color into the property set. */
    void         SetColorProperty( const OUString& rPropName, const Color& rColor )
                            { SetProperty( rPropName, sal_Int32( rColor ) ); }

    /** Puts the passed properties into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                SetProperties( const css::uno::Sequence< OUString > & rPropNames, const css::uno::Sequence< css::uno::Any >& rValues );

private:
    css::uno::Reference< css::beans::XPropertySet >       mxPropSet;          /// The mandatory property set interface.
    css::uno::Reference< css::beans::XMultiPropertySet >  mxMultiPropSet;     /// The optional multi property set interface.
};

/** Generic helper class for reading from and writing to property sets.

    Usage:
    1)  Call the constructor with a null-terminated array of ASCII strings.
    2a) Read properties from a property set: Call the ReadFromPropertySet()
        function, then get the properties with the ReadValue() functions or the
        operator>> stream operator. The properties are returned in order of the
        array of property names passed in the constructor.
    2b) Write properties to a property set: Call InitializeWrite() to start a
        new cycle. Set the values with the WriteValue() functions or the
        operator<< stream operator. The order of the properties is equal to the
        array of property names passed in the constructor. Finally, call the
        WriteToPropertySet() function.
 */
class ScfPropSetHelper
{
public:
    /** @param ppPropNames  A null-terminated array of ASCII property names. */
    explicit            ScfPropSetHelper( const sal_Char* const* ppcPropNames );

    // read properties --------------------------------------------------------

    /** Reads all values from the passed property set. */
    void                ReadFromPropertySet( const ScfPropertySet& rPropSet );

    /** Reads the next value from the value sequence. */
    template< typename Type >
    void                ReadValue( Type& rValue );
    /** Reads an Any from the value sequence. */
    void                ReadValue( css::uno::Any& rAny );
    /** Reads a color value from the value sequence. */
    void                ReadValue( Color& rColor );
    /** Reads a C++ boolean value from the value sequence. */
    void                ReadValue( bool& rbValue );

    // write properties -------------------------------------------------------

    /** Must be called before reading or storing property values in the helper. */
    void                InitializeWrite();

    /** Writes the next value to the value sequence. */
    template< typename Type >
    void                WriteValue( const Type& rValue );
    /** Writes an Any to the value sequence. */
    void                WriteValue( const css::uno::Any& rAny );
    /** Writes a color value to the value sequence. */
    void         WriteValue( const Color& rColor )
                            { WriteValue( sal_Int32( rColor ) ); }
    /** Writes a C++ boolean value to the value sequence. */
    void                WriteValue( bool rbValue );

    /** Writes all values to the passed property set. */
    void                WriteToPropertySet( ScfPropertySet& rPropSet ) const;

private:
    /** Returns a pointer to the next Any to be written to. */
    css::uno::Any*             GetNextAny();

private:
    css::uno::Sequence< OUString >       maNameSeq;          /// Sequence of property names.
    css::uno::Sequence< css::uno::Any >  maValueSeq;         /// Sequence of property values.
    ScfInt32Vec         maNameOrder;        /// Maps initial order to alphabetical order.
    size_t              mnNextIdx;          /// Counter for next Any to be processed.
};

template< typename Type >
void ScfPropSetHelper::ReadValue( Type& rValue )
{
    css::uno::Any* pAny = GetNextAny();
    if (pAny)
        *pAny >>= rValue;
}

template< typename Type >
void ScfPropSetHelper::WriteValue( const Type& rValue )
{
    css::uno::Any* pAny = GetNextAny();
    if( pAny )
        *pAny <<= rValue;
}

template< typename Type >
ScfPropSetHelper& operator>>( ScfPropSetHelper& rPropSetHelper, Type& rValue )
{
    rPropSetHelper.ReadValue( rValue );
    return rPropSetHelper;
}

template< typename Type >
ScfPropSetHelper& operator<<( ScfPropSetHelper& rPropSetHelper, const Type& rValue )
{
    rPropSetHelper.WriteValue( rValue );
    return rPropSetHelper;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
