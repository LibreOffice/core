/*************************************************************************
 *
 *  $RCSfile: fapihelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 13:43:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

// Get properties =============================================================

/** Puts the value of a property into an Any. The XPropertySet must be valid.
    @return  sal_True, if the Any could be filled. */
bool getPropAny(
        ::com::sun::star::uno::Any& rAny,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxProp,
        const ::rtl::OUString& rName );

/** Template for getting data from a property. The XPropertySet must be valid.
    @return  true, if the struct could be filled. */
template< typename Type >
inline bool getPropValue(
        Type& rValue,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxProp,
        const ::rtl::OUString& rName )
{
    ::com::sun::star::uno::Any aAny;
    return ::getPropAny( aAny, rxProp, rName ) && (aAny >>= rValue);
}

/** Gets a boolean value from an XPropertySet. The XPropertySet must be valid.
    @return  true, if the boolean could be filled. */
inline bool getPropBool(
        bool& rbValue,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxProp,
        const ::rtl::OUString& rName )
{
    ::com::sun::star::uno::Any aAny;
    bool bRet = ::getPropAny( aAny, rxProp, rName ) && (aAny.getValueTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN);
    if( bRet )
        rbValue = (*static_cast< const sal_Bool* >( aAny.getValue() ) == sal_True);
    return bRet;
}

/** Gets a boolean value from an XPropertySet.
    @descr  The XPropertySet must be valid. This is the short version, it returns
    false on a property with value sal_False and on error.
    @return  true = if the property contains sal_True;  false = sal_False or error. */
inline bool getPropBool(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxProp,
        const ::rtl::OUString& rName )
{
    bool bRet;
    return ::getPropBool( bRet, rxProp, rName ) && bRet;
}

// Static helper functions ====================================================

class SfxMedium;

/** Static API helper functions. */
class ScfApiHelper
{
public:
    /** Opens a password dialog and returns the entered password.
        @return  The entered password or an empty string on 'Cancel' or any error. */
    static String       QueryPasswordForMedium( SfxMedium& rMedium );
};

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
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       XPropertySetRef;
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >  XMultiPropSetRef;
    typedef ::com::sun::star::uno::Any                                                      UnoAny;
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                   UnoAnySequence;
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >                              UnoStringSequence;

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
    void                GetProperties( UnoAnySequence& rValues, const UnoStringSequence& rPropNames ) const;

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
                            { SetProperty( rPropName, static_cast< sal_Int32 >( rColor.GetColor() ) ); }

    /** Puts the passed properties into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                SetProperties( const UnoStringSequence& rPropNames, const UnoAnySequence& rValues );

    // ------------------------------------------------------------------------
private:
    XPropertySetRef     mxPropSet;          /// The mandatory property set interface.
    XMultiPropSetRef    mxMultiPropSet;     /// The optional multi property set interface.
};

// ----------------------------------------------------------------------------

/** Generic helper class for reading from and writing to property sets.

    Usage:
    1)  Call the constructor with an array of ASCII strings ordered
        alphabetically (required by the XMultiPropertySet interface).
    2a) Get properties from a property set: Call the ReadFromPropertySet()
        function, then get the read properties with the GetAny() function. The
        indexes are the same as used for the property names passed to the
        constructor.
    2b) Set properties to a property set: Set the values with the GetAny()
        function. The index is the same as used for the property names passed
        to the constructor. All values must be set properly. Finally, call the
        WriteToPropertySet() function.
 */
class ScfPropSetHelper
{
public:
    /** @param ppPropNames  An array of ASCII property names.
        @param nPropCount  Count of property names contained in ppPropNames. */
    explicit            ScfPropSetHelper( const sal_Char** ppcPropNames, sal_Int32 nPropCount );

    /** Returns the property name. */
    inline const ::rtl::OUString& GetName( sal_Int32 nIdx ) const;
    /** Returns a reference to the Any containing a property value. */
    inline ::com::sun::star::uno::Any& GetAny( sal_Int32 nIdx );

    /** Reads all values from the passed property set. */
    void                ReadFromPropertySet( const ScfPropertySet& rPropSet );
    /** Writes all values to the passed property set. */
    void                WriteToPropertySet( ScfPropertySet& rPropSet ) const;

private:
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >              UnoStringSequence;
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   UnoAnySequence;

    UnoStringSequence   maNameSeq;          /// Sequence of property names.
    UnoAnySequence      maValueSeq;         /// Sequence of property values.

};

inline const ::rtl::OUString& ScfPropSetHelper::GetName( sal_Int32 nIdx ) const
{
    DBG_ASSERT( (0 <= nIdx) && (nIdx < maNameSeq.getLength()), "ScfPropSetHelper::GetName - invalid index" );
    return maNameSeq[ nIdx ];
}

inline ::com::sun::star::uno::Any& ScfPropSetHelper::GetAny( sal_Int32 nIdx )
{
    DBG_ASSERT( (0 <= nIdx) && (nIdx < maValueSeq.getLength()), "ScfPropSetHelper::GetAny - invalid index" );
    return maValueSeq[ nIdx ];
}

// ============================================================================

#endif

