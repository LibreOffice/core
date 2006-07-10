/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fapihelper.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:51:44 $
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

#ifndef SC_FAPIHELPER_HXX
#define SC_FAPIHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

#ifndef _COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
} } }

// Static helper functions ====================================================

class SfxMedium;
class SfxObjectShell;

/** Static API helper functions. */
class ScfApiHelper
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               XInterfaceRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    XServiceFactoryRef;
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                       UnoAnySequence;

public:
    /** Converts a tools color to a UNO color value. */
    inline static sal_Int32 ConvertToApiColor( const Color& rColor )
                            { return static_cast< sal_Int32 >( rColor.GetColor() ); }
    /** Converts a UNO color value to a tools color. */
    inline static Color ConvertFromApiColor( sal_Int32 nApiColor )
                            { return Color( static_cast< ColorData >( nApiColor ) ); }

    /** Converts a non-empty vector into a UNO sequence containing elements of the same type. */
    template< typename Type >
    static ::com::sun::star::uno::Sequence< Type >
                            VectorToSequence( const ::std::vector< Type >& rVector );

    /** Returns the multi service factory from a document shell. */
    static XServiceFactoryRef GetServiceFactory( SfxObjectShell* pShell );

    /** Creates an instance from the passed service name, using the passed service factory. */
    static XInterfaceRef CreateInstance(
                            XServiceFactoryRef xFactory,
                            const ::rtl::OUString& rServiceName );

    /** Creates an instance from the passed service name, using the service factory of the passed object. */
    static XInterfaceRef CreateInstance(
                            SfxObjectShell* pShell,
                            const ::rtl::OUString& rServiceName );

    /** Creates an instance from the passed service name, using the process service factory. */
    static XInterfaceRef CreateInstance( const ::rtl::OUString& rServiceName );

    /** Creates an instance from the passed service name, using the passed service factory. */
    static XInterfaceRef CreateInstanceWithArgs(
                            XServiceFactoryRef xFactory,
                            const ::rtl::OUString& rServiceName,
                            const UnoAnySequence& rArgs );

    /** Creates an instance from the passed service name, using the service factory of the passed object. */
    static XInterfaceRef CreateInstanceWithArgs(
                            SfxObjectShell* pShell,
                            const ::rtl::OUString& rServiceName,
                            const UnoAnySequence& rArgs );

    /** Creates an instance from the passed service name, using the process service factory. */
    static XInterfaceRef CreateInstanceWithArgs(
                            const ::rtl::OUString& rServiceName,
                            const UnoAnySequence& rArgs );

    /** Opens a password dialog and returns the entered password.
        @return  The entered password or an empty string on 'Cancel' or any error. */
    static String       QueryPasswordForMedium( SfxMedium& rMedium );
};

template< typename Type >
::com::sun::star::uno::Sequence< Type > ScfApiHelper::VectorToSequence( const ::std::vector< Type >& rVector )
{
    DBG_ASSERT( !rVector.empty(), "ScfApiHelper::VectorToSequence - vector is empty" );
    return ::com::sun::star::uno::Sequence< Type >( &rVector.front(), static_cast< sal_Int32 >( rVector.size() ) );
}

// Property sets ==============================================================

/** A wrapper for an UNO property set.

    This class provides functions to silently get and set properties (without
    exceptions, without checking for validity of the UNO property set).

    An instance is constructed with the reference to an UNO property set or any
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
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >         XPropertySetRef;
    typedef ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XMultiPropertySet >    XMultiPropSetRef;
    typedef ::com::sun::star::uno::Any                          UnoAny;
    typedef ::com::sun::star::uno::Sequence< UnoAny >           UnoAnySequence;
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >  OUStringSequence;

public:
    inline explicit     ScfPropertySet() {}
    /** Constructs a property set wrapper with the passed UNO property set. */
    inline explicit     ScfPropertySet( XPropertySetRef xPropSet ) { Set( xPropSet ); }
    /** Constructs a property set wrapper after querying the XPropertySet interface. */
    template< typename InterfaceType >
    inline explicit     ScfPropertySet( ::com::sun::star::uno::Reference< InterfaceType > xInterface ) { Set( xInterface ); }

    /** Sets the passed UNO property set and releases the old UNO property set. */
    void                Set( XPropertySetRef xPropSet );
    /** Queries the passed interface for an XPropertySet and releases the old UNO property set. */
    template< typename InterfaceType >
    inline void         Set( ::com::sun::star::uno::Reference< InterfaceType > xInterface )
                            { Set( XPropertySetRef( xInterface, ::com::sun::star::uno::UNO_QUERY ) ); }

    /** Returns true, if the contained XPropertySet interface is valid. */
    inline bool         Is() const { return mxPropSet.is(); }

    // Get properties ---------------------------------------------------------

    /** Gets the specified property from the property set.
        @return  true, if the Any could be filled with the property value. */
    bool                GetAnyProperty( UnoAny& rValue, const ::rtl::OUString& rPropName ) const;

    /** Gets the specified property from the property set.
        @return  true, if the passed variable could be filled with the property value. */
    template< typename Type >
    inline bool         GetProperty( Type& rValue, const ::rtl::OUString& rPropName ) const
                            { UnoAny aAny; return GetAnyProperty( aAny, rPropName ) && (aAny >>= rValue); }

    /** Gets the specified Boolean property from the property set.
        @return  true, if the passed Boolean variable could be filled with the property value. */
    bool                GetBoolProperty( bool& rbValue, const ::rtl::OUString& rPropName ) const;

    /** Gets the specified Boolean property from the property set.
        @descr  This is the short version, returning false for a false value and on error.
        @return  true = property contains true; false = property contains false or error occured. */
    inline bool         GetBoolProperty( const ::rtl::OUString& rPropName ) const
                            { bool bValue; return GetBoolProperty( bValue, rPropName ) && bValue; }

    /** Gets the specified Boolean property from the property set.
        @return  true, if the passed Boolean variable could be filled with the property value. */
    bool                GetStringProperty( String& rValue, const ::rtl::OUString& rPropName ) const;

    /** Gets the specified color property from the property set.
        @return  true, if the passed color variable could be filled with the property value. */
    bool                GetColorProperty( Color& rColor, const ::rtl::OUString& rPropName ) const;

    /** Gets the specified properties from the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                GetProperties( UnoAnySequence& rValues, const OUStringSequence& rPropNames ) const;

    // Set properties ---------------------------------------------------------

    /** Puts the passed Any into the property set. */
    void                SetAnyProperty( const ::rtl::OUString& rPropName, const UnoAny& rValue );

    /** Puts the passed value into the property set. */
    template< typename Type >
    inline void         SetProperty( const ::rtl::OUString& rPropName, const Type& rValue )
                            { SetAnyProperty( rPropName, ::com::sun::star::uno::makeAny( rValue ) ); }

    /** Puts the passed Boolean value into the property set. */
    inline void         SetBoolProperty( const ::rtl::OUString& rPropName, bool bValue )
                            { SetAnyProperty( rPropName, ::comphelper::makeBoolAny( bValue ) ); }

    /** Puts the passed string into the property set. */
    inline void         SetStringProperty( const ::rtl::OUString& rPropName, const String& rValue )
                            { SetProperty( rPropName, ::rtl::OUString( rValue ) ); }

    /** Puts the passed color into the property set. */
    inline void         SetColorProperty( const ::rtl::OUString& rPropName, const Color& rColor )
                            { SetProperty( rPropName, ScfApiHelper::ConvertToApiColor( rColor ) ); }

    /** Puts the passed properties into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                SetProperties( const OUStringSequence& rPropNames, const UnoAnySequence& rValues );

    // ------------------------------------------------------------------------
private:
    XPropertySetRef     mxPropSet;          /// The mandatory property set interface.
    XMultiPropSetRef    mxMultiPropSet;     /// The optional multi property set interface.
};

// ----------------------------------------------------------------------------

/** Generic helper class for reading from and writing to property sets.

    Usage:
    1)  Call the constructor with a null-terminated array of ASCII strings.
    2a) Read properties from a property set: Call the ReadFromPropertySet()
        function, then get the properties with the ReadValue() function or the
        >> stream operator. The properties are returned in order of the array
        of property names passed in the constructor.
    2b) Write properties to a property set: Set the values with the
        WriteValue() function or the << stream operator. The order of the
        properties is equal to the array of property names passed in the
        constructor. Finally, call the WriteToPropertySet() function.
 */
class ScfPropSetHelper
{
public:
    typedef ::com::sun::star::uno::Any UnoAny;

public:
    /** @param ppPropNames  A null-terminated array of ASCII property names. */
    explicit            ScfPropSetHelper( const sal_Char* const* ppcPropNames );

    // read properties --------------------------------------------------------

    /** Reads all values from the passed property set. */
    void                ReadFromPropertySet( const ScfPropertySet& rPropSet );

    // write properties -------------------------------------------------------

    /** Must be called before reading or storing property values in the helper. */
    void                InitializeWrite( bool bClearAllAnys = false );

    /** Writes the next value to the value sequence. */
    template< typename Type >
    void                WriteValue( const Type& rValue );
    /** Writes an Any to the value sequence. */
    void                WriteValue( const UnoAny& rAny );
    /** Writes a tools string to the value sequence. */
    inline void         WriteValue( const String& rString )
                            { WriteValue( ::rtl::OUString( rString ) ); }
    /** Writes a color value to the value sequence. */
    inline void         WriteValue( const Color& rColor )
                            { WriteValue( ScfApiHelper::ConvertToApiColor( rColor ) ); }
    /** Writes a C++ boolean value to the value sequence. */
    void                WriteValue( const bool& rbValue );

    /** Writes all values to the passed property set. */
    void                WriteToPropertySet( ScfPropertySet& rPropSet ) const;

    // ------------------------------------------------------------------------
private:
    /** Returns a pointer to the next Any to be written to. */
    UnoAny*             GetNextAny();

private:
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >  OUStringSequence;
    typedef ::com::sun::star::uno::Sequence< UnoAny >           UnoAnySequence;

    OUStringSequence    maNameSeq;          /// Sequence of property names.
    UnoAnySequence      maValueSeq;         /// Sequence of property values.
    ScfInt32Vec         maNameOrder;        /// Maps initial order to alphabetical order.
    size_t              mnNextIdx;          /// Counter for next Any to be processed.
};

template< typename Type >
void ScfPropSetHelper::WriteValue( const Type& rValue )
{
    if( UnoAny* pAny = GetNextAny() )
        *pAny <<= rValue;
}

template< typename Type >
ScfPropSetHelper& operator<<( ScfPropSetHelper& rPropSetHelper, const Type& rValue )
{
    rPropSetHelper.WriteValue( rValue );
    return rPropSetHelper;
}

// ============================================================================

#endif

