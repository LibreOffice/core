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

#include "proplist.hxx"

#include <rtl/ustrbuf.hxx>
#include <toolkit/dllapi.h>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <tools/debug.hxx>

#include "helper.hxx"

namespace layoutimpl
{

using namespace com::sun::star;
using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;

namespace prophlp
{

bool TOOLKIT_DLLPUBLIC
canHandleProps( const uno::Reference< uno::XInterface > &xPeer )
{
    uno::Reference< beans::XPropertySet > xPropSet( xPeer, uno::UNO_QUERY );
    if ( xPropSet.is() )
        return true;
    uno::Reference< beans::XPropertySetInfo > xInfo( xPeer, uno::UNO_QUERY );
    uno::Reference< awt::XVclWindowPeer> xVclPeer( xPeer, uno::UNO_QUERY );
    return xInfo.is() && xVclPeer.is();
}

uno::Reference< beans::XPropertySetInfo > TOOLKIT_DLLPUBLIC
queryPropertyInfo(
    const uno::Reference< uno::XInterface > &xPeer )
{
    uno::Reference< beans::XPropertySetInfo > xInfo( xPeer, uno::UNO_QUERY );
    if ( !xInfo.is() )
    {
        uno::Reference< beans::XPropertySet > xPropSet( xPeer, uno::UNO_QUERY );
        if ( xPropSet.is() )
            xInfo = xPropSet->getPropertySetInfo();
    }
    return xInfo;
}

void TOOLKIT_DLLPUBLIC
setProperty( const uno::Reference< uno::XInterface > &xPeer,
                  const OUString &rName, uno::Any aValue )
{
    uno::Reference< awt::XVclWindowPeer> xVclPeer( xPeer, uno::UNO_QUERY );
    if ( xVclPeer.is() )
        xVclPeer->setProperty( rName, aValue );
    else
    {
        uno::Reference< beans::XPropertySet > xPropSet( xPeer, uno::UNO_QUERY );
        xPropSet->setPropertyValue( rName, aValue );
    }
}

uno::Any TOOLKIT_DLLPUBLIC
getProperty( const uno::Reference< uno::XInterface > &xPeer,
                      const OUString &rName )
{
    uno::Reference< awt::XVclWindowPeer> xVclPeer( xPeer, uno::UNO_QUERY );
    if ( xVclPeer.is() )
        return xVclPeer->getProperty( rName );

    uno::Reference< beans::XPropertySet > xPropSet( xPeer, uno::UNO_QUERY );
    return xPropSet->getPropertyValue( rName );
}

} // namespace prophlp


/* Given a string and a type, it converts the string to the type, and returns
   it encapsulated in Any. */
uno::Any anyFromString( OUString const& value, uno::Type const& type )
{
    sal_Int16 radix = 10;
    OUString intval = value;
    if ( value.getLength() > 2 && value[0] == '0' && value[1] == 'x' )
        intval = value.copy( 2 ), radix = 16;
    else if ( value.getLength() > 1 && value[0] == '#' )
        intval = value.copy( 1 ), radix = 16;
    switch ( type.getTypeClass() )
    {
        case uno::TypeClass_CHAR:
            return uno::makeAny( value.toChar() );
        case uno::TypeClass_BOOLEAN:
            if ( value == OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) ) )
                return uno::makeAny( true );
            else if ( value == OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) ) )
                return uno::makeAny( false );
            break;  // ends switch, throws exception
        case uno::TypeClass_BYTE:
            return uno::makeAny( ( sal_uInt8 ) intval.toInt32( radix ) );
        case uno::TypeClass_SHORT:
            return uno::makeAny( ( sal_Int16 ) intval.toInt32( radix ) );
        case uno::TypeClass_UNSIGNED_SHORT:
            return uno::makeAny( ( sal_uInt16 ) intval.toInt32( radix ) );
        case uno::TypeClass_ENUM:
            return uno::makeAny( ( sal_Int16 ) intval.toInt32( radix ) );
        case uno::TypeClass_LONG:
            return uno::makeAny( ( sal_Int32 ) intval.toInt32( radix ) );
        case uno::TypeClass_UNSIGNED_LONG:
            return uno::makeAny( ( sal_uInt32 ) intval.toInt32( radix ) );
        case uno::TypeClass_HYPER:
            return uno::makeAny( ( sal_Int64 ) intval.toInt64( radix ) );
        case uno::TypeClass_UNSIGNED_HYPER:
            return uno::makeAny( ( sal_uInt16 ) intval.toInt64( radix ) );
        case uno::TypeClass_FLOAT:
            return uno::makeAny( value.toFloat() );
        case uno::TypeClass_DOUBLE:
            return uno::makeAny( value.toDouble() );
        case uno::TypeClass_STRING:
            return uno::makeAny( value );
        case uno::TypeClass_CONSTANT:
            return uno::makeAny( intval.toInt32( radix ) );
        case uno::TypeClass_INTERFACE:
            return uno::makeAny( loadGraphic( OUSTRING_CSTR( value ) ) );
        case uno::TypeClass_SEQUENCE:
        {
            sal_Int32 i = 0;
            bool escaped = false, first = true;
            OUString item, token;
            std::list< OUString > values;
            do
            {
                token = value.getToken( 0, ':', i );

                if ( !token.getLength() && !escaped )
                {
                    escaped = true;
                    item += OUString( ':' );
                }
                else if ( escaped )
                {
                    escaped = false;
                    item += token;
                }
                else
                {
                    if ( !first )
                        values.push_back( item );
                    item = token;
                }
                first = false;
            }
            while ( i >= 0 );
            if ( item.getLength() )
                values.push_back( item );

            uno::Sequence< OUString > seq( values.size() );
            i = 0;
            for ( std::list< OUString >::const_iterator it = values.begin();
                  it != values.end(); ++it, ++i )
                seq[ i ] = *it;

            return uno::makeAny( seq );
        }

        default:
            OSL_TRACE( "ERROR: unknown property type of value: `%s'\n", OUSTRING_CSTR( value ) );
            break;
    }
    throw uno::RuntimeException();
}

/* Converts the XML naming scheme to UNO's, for legacy compatibility
   (so, ergo, "one-two-three-four" -> "OneTwoThreeFour"). */
static OUString toUnoNaming ( OUString const &string )
{
    OUStringBuffer buffer( string.getLength() );
    sal_Unicode *str = string.pData->buffer;
    bool capitalize = true;

    for ( int i = 0; i < string.getLength(); i++ )
    {
        if ( i == 0 && str[0] == '_' )
            /* Skip translate-me prefix.  */
            continue;
        if ( str[i] == '-' )
            capitalize = true;
        else
        {
            if ( capitalize && str[i] >= 'a' && str[i] <= 'z' )
                buffer.append( (sal_Unicode ) ( str[i] - 'a' + 'A' ) );
            else
                buffer.append( (sal_Unicode ) str[i] );
            capitalize = false;
        }
    }

    return buffer.makeStringAndClear();
}

/*
 * convert incoming XML style property names, to AWT style property names.
 * convert the values based on introspection information.
 * apply to either an XPropertySet or an XPropertySetInfo | XVclWindowPeer
 * aggregate.
 */
void
setProperties( uno::Reference< uno::XInterface > const& xPeer,
               PropList const& rProps )
{
    if ( !prophlp::canHandleProps( xPeer ) )
    {
        OSL_FAIL( "Error: setProperties - bad handle ignoring props:\n" );
        for ( PropList::const_iterator it = rProps.begin(); it != rProps.end();
              ++it )
        {
            OSL_TRACE( "%s=%s\n", OUSTRING_CSTR( it->first ), OUSTRING_CSTR( it->second ) );
        }
        return;
    }

    for ( PropList::const_iterator it = rProps.begin(); it != rProps.end();
          ++it )
        setProperty( xPeer, it->first, it->second );
}

void
setProperty( uno::Reference< uno::XInterface > const& xPeer,
             OUString const& attr, OUString const& value )
{
    OUString unoAttr = toUnoNaming( attr );

    OSL_TRACE( "setting %s=%s", OUSTRING_CSTR( attr ), OUSTRING_CSTR( value ) );
    // get a Property object
    beans::Property prop;
    try
    {
        uno::Reference< beans::XPropertySetInfo > xInfo
            = prophlp::queryPropertyInfo( xPeer );
        prop = xInfo->getPropertyByName( unoAttr );
    }
    catch( beans::UnknownPropertyException & )
    {
        OSL_TRACE( "Warning: unknown attribute: `%s'\n", OUSTRING_CSTR( unoAttr ) );
        return;
    }

    if ( prop.Name.getLength() <= 0 )
    {
        OSL_TRACE( "Warning: missing prop: `%s'\n", OUSTRING_CSTR( unoAttr ) );
        return;
    }

    // encapsulates value in an uno::Any
    uno::Any any;
    try
    {
        any = anyFromString( value, prop.Type );
    }
    catch( uno::RuntimeException & )
    {
        OSL_TRACE( "Warning: %s( %s )( %s ) attribute is of type %s( rejected: %s )\n", OUSTRING_CSTR( unoAttr ), OUSTRING_CSTR( value ), OUSTRING_CSTR( prop.Name ),  OUSTRING_CSTR( prop.Type.getTypeName() ), OUSTRING_CSTR( value ) );
        return;
    }

    // sets value on property
    try
    {
        prophlp::setProperty( xPeer, unoAttr, any );
    }
    catch( ... )
    {
        OSL_TRACE( "Warning: cannot set attribute %s to %s \n", OUSTRING_CSTR( unoAttr ), OUSTRING_CSTR( value ) );
    }
}




struct AttributesMap
{
    const char *name;
    long value;
    bool windowAttr;
};
static const AttributesMap attribsMap[] =
{
    { "autohscroll",  awt::VclWindowPeerAttribute::AUTOHSCROLL,  false },
    { "autovscroll",  awt::VclWindowPeerAttribute::AUTOVSCROLL,  false },
    { "center",       awt::VclWindowPeerAttribute::CENTER,       false },
    { "clipchildren", awt::VclWindowPeerAttribute::CLIPCHILDREN, false },
    { "closeable",    awt::WindowAttribute::CLOSEABLE,           true },
    { "defbutton",    awt::VclWindowPeerAttribute::DEFBUTTON,    false },
    { "dropdown",     awt::VclWindowPeerAttribute::DROPDOWN,     false },
    { "fullsize",     awt::WindowAttribute::FULLSIZE,            true  }, //FIXME?
    { "group",        awt::VclWindowPeerAttribute::GROUP,        false },
    { "has_border",   awt::WindowAttribute::BORDER,              true },
    { "hscroll",      awt::VclWindowPeerAttribute::HSCROLL,      false },
    { "left",         awt::VclWindowPeerAttribute::LEFT,         false },
    { "moveable",     awt::WindowAttribute::MOVEABLE,            true },
    { "noborder",     awt::VclWindowPeerAttribute::NOBORDER,     false },
    { "nolabel",      awt::VclWindowPeerAttribute::NOLABEL,      false },
    { "optimumsize",  awt::WindowAttribute::OPTIMUMSIZE,         false },
    { "readonly",     awt::VclWindowPeerAttribute::READONLY,     false },
    { "right",        awt::VclWindowPeerAttribute::RIGHT,        false },
    { "show",         awt::WindowAttribute::SHOW,                true },
    { "sizeable",     awt::WindowAttribute::SIZEABLE,            true },
    { "sort",         awt::VclWindowPeerAttribute::SORT,         false },
    { "spin",         awt::VclWindowPeerAttribute::SPIN,         false },
    { "vscroll",      awt::VclWindowPeerAttribute::VSCROLL,      false },

    // cutting on OK, YES_NO_CANCEL and related obsite attributes...
};
static const int attribsMapLen = sizeof( attribsMap ) / sizeof( AttributesMap );

void propsFromAttributes( const uno::Reference<xml::input::XAttributes> & xAttributes,
                          PropList &rProps, sal_Int32 nNamespace )
{
    sal_Int32 nAttrs = xAttributes->getLength();
    for ( sal_Int32 i = 0; i < nAttrs; i++ )
    {
        if ( nNamespace != xAttributes->getUidByIndex( i ) )
            continue;

        std::pair< OUString, OUString > aElem
            ( xAttributes->getLocalNameByIndex( i ),
              xAttributes->getValueByIndex( i ) );

        if ( aElem.first.getLength() > 0 ) // namespace bits ..
            rProps.push_back( aElem );
    }
}

bool
findAndRemove( const char *pAttr, PropList &rProps, OUString &rValue )
{
    PropList::iterator it;
    OUString aName = OUString::createFromAscii( pAttr );

    for ( it = rProps.begin(); it != rProps.end(); ++it )
    {
        if ( it->first.equalsIgnoreAsciiCase( aName )
             || it->first.equalsIgnoreAsciiCase( OUString(RTL_CONSTASCII_USTRINGPARAM ("_")) + aName ) )
        {
            rValue = it->second;
            rProps.erase( it );
            return true;
        }
    }
    rValue = OUString();
    return false;
}

long
getAttributeProps( PropList &rProps )
{
    long nAttrs = 0;
    OUString aValue;

    OUString trueStr( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

    if ( findAndRemove( "show", rProps, aValue ) &&
         aValue.equalsIgnoreAsciiCase(
             OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) ) ) )
        ;
    else
        nAttrs |= awt::WindowAttribute::SHOW;

    for ( int i = 0; i < attribsMapLen; i++ )
    {
        if ( findAndRemove( attribsMap[i].name, rProps, aValue ) )
        {
            if ( aValue.equalsIgnoreAsciiCase( trueStr ) )
                nAttrs |= attribsMap[i].value;
        }
    }

    if ( findAndRemove( "align", rProps, aValue ) )
    {
        sal_Int32 nVal = aValue.toInt32();

        if ( nVal == 0 /* PROPERTY_ALIGN_LEFT */ )
            nAttrs |= awt::VclWindowPeerAttribute::LEFT;
        else if ( nVal == 1 /* PROPERTY_ALIGN_CENTER */ )
            nAttrs |= awt::VclWindowPeerAttribute::CENTER;
        else if ( nVal == 2 )
            nAttrs |= awt::VclWindowPeerAttribute::RIGHT;
    }

    return nAttrs;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
