/*************************************************************************
 *
 *  $RCSfile: fapihelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:04:43 $
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

// ============================================================================

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

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

class SfxMedium;

// ============================================================================

#define CSS                     ::com::sun::star
#define PROPNAME( constascii )  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constascii ) )

// Set properties =============================================================

/** Sets the value of an Any to an XPropertySet. The XPropertySet must be valid. */
void setPropAny(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp,
        const ::rtl::OUString& rName,
        const ::com::sun::star::uno::Any& rAny );

/** Template for setting values to an XPropertySet. The XPropertySet must be valid. */
template< typename Type >
inline void setPropValue(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp,
        const ::rtl::OUString& rName,
        const Type& rValue )
{
    ::setPropAny( xProp, rName, ::com::sun::star::uno::makeAny( rValue ) );
}

/** Sets a Boolean value to an XPropertySet. The XPropertySet must be valid. */
inline void setPropBool(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp,
        const ::rtl::OUString& rName,
        sal_Bool bValue )
{
    ::setPropAny( xProp, rName, ::comphelper::makeBoolAny( bValue ) );
}

/** Sets a string to an XPropertySet. The XPropertySet must be valid. */
inline void setPropString(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProp,
        const ::rtl::OUString& rName,
        const String& rText )
{
    ::setPropValue( xProp, rName, ::rtl::OUString( rText ) );
}

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

/** Static API helper functions. */
class ScfApiHelper : ScfNoInstance
{
public:
    /** Opens a password dialog and returns the entered password.
        @return  The entered password or an empty string on 'Cancel' or any error. */
    static String       QueryPasswordForMedium( SfxMedium& rMedium );
};

// MultiPropertySets ==========================================================

/** Generic helper class for reading from and writing to XMultiPropertySets.
    @descr  Derived classes have to call the constructor with an array of ASCII
    strings ordered alphabetically. Note: The correct order is required by the
    XMultiPropertySet. It will not be checked here!
*/
class ScfMultiPSHelper
{
private:
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString > OUStringVec;
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > AnyVec;
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > XPropertySetRef;
    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XMultiPropertySet > XMultiPropertySetRef;

    OUStringVec                 maNameSeq;      /// Sequence of property names.
    AnyVec                      maValueSeq;     /// Sequence of property values.

public:
    /** @param ppPropNames  An array of ASCII property names.
        @param nPropCount  Count of property names contained in ppPropNames. */
                                ScfMultiPSHelper( const sal_Char** ppPropNames, sal_Int32 nPropCount );

    /** Returns the property name. */
    inline const ::rtl::OUString& getName( sal_Int32 nIndex ) const;
    /** Returns a reference to the Any containing a property value. */
    inline ::com::sun::star::uno::Any& getAny( sal_Int32 nIndex );

    /** Reads all values from the XMultiPropertySet.
        @descr  Tries to read all properties from the XPropertySet, if the XMultiPropertySet is not valid.
        @return  true on success. */
    bool                        getPropertyValues(
                                    const XMultiPropertySetRef& xMultiPS,
                                    const XPropertySetRef& xPropSet );

    /** Applies all values to the XMultiPropertySet.
        @descr  Tries to set all properties to the XPropertySet, if the XMultiPropertySet is not valid.
        @return  true on success. */
    bool                        setPropertyValues(
                                    const XMultiPropertySetRef& xMultiPS,
                                    const XPropertySetRef& xPropSet );
};

// ----------------------------------------------------------------------------

inline const ::rtl::OUString& ScfMultiPSHelper::getName( sal_Int32 nIndex ) const
{
    DBG_ASSERT( (0 <= nIndex) && (nIndex < maNameSeq.getLength()), "ScfMultiPSHelper::getName - invalid index" );
    return maNameSeq[ nIndex ];
}

inline ::com::sun::star::uno::Any& ScfMultiPSHelper::getAny( sal_Int32 nIndex )
{
    DBG_ASSERT( (0 <= nIndex) && (nIndex < maValueSeq.getLength()), "ScfMultiPSHelper::getValue - invalid index" );
    return maValueSeq[ nIndex ];
}

// ============================================================================

#endif

