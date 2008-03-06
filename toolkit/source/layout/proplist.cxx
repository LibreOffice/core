#include "proplist.hxx"

#include <rtl/ustrbuf.hxx>
#include <toolkit/dllapi.h>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <tools/debug.hxx>

#if TEST_LAYOUT && !defined( DBG_UTIL )
#include <stdio.h>
#undef DBG_ERROR
#define DBG_ERROR printf
#undef DBG_ERROR1
#define DBG_ERROR1 printf
#undef DBG_ERROR2
#define DBG_ERROR2 printf
#endif /* TEST_LAYOUT && !DBG_UTIL */

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
uno::Any anyFromString( const OUString &value, const uno::Type &type )
{
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
            return uno::makeAny( ( sal_uInt8 ) value.toInt32() );
        case uno::TypeClass_SHORT:
            return uno::makeAny( ( sal_Int16 ) value.toInt32() );
        case uno::TypeClass_UNSIGNED_SHORT:
            return uno::makeAny( ( sal_uInt16 ) value.toInt32() );
        case uno::TypeClass_ENUM:
            return uno::makeAny( ( sal_Int16 ) value.toInt32() );
        case uno::TypeClass_LONG:
            return uno::makeAny( ( sal_Int32 ) value.toInt32() );
        case uno::TypeClass_UNSIGNED_LONG:
            return uno::makeAny( ( sal_uInt32 ) value.toInt32() );
        case uno::TypeClass_HYPER:
            return uno::makeAny( ( sal_Int64 ) value.toInt64() );
        case uno::TypeClass_UNSIGNED_HYPER:
            return uno::makeAny( ( sal_uInt16 ) value.toInt64() );
        case uno::TypeClass_FLOAT:
            return uno::makeAny( value.toFloat() );
        case uno::TypeClass_DOUBLE:
            return uno::makeAny( value.toDouble() );
        case uno::TypeClass_STRING:
            return uno::makeAny( value );
        case uno::TypeClass_CONSTANT:
            if ( value.getLength() > 2 && value[0] == '0' && value[1] == 'x' )
                return uno::makeAny( value.copy( 2 ).toInt32( 16 ) );
            return uno::makeAny( value.toInt32() );
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
            for( std::list< OUString >::const_iterator it = values.begin();
                 it != values.end(); it++, i++ )
                seq[ i ] = *it;

            return uno::makeAny( seq );
        }
        break;

        default:
            DBG_ERROR1( "ERROR: unknown property type of value: `%s'", OUSTRING_CSTR( value ) );
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
        DBG_ERROR( "Error: setProperties - bad handle ignoring props:" );
        PropList::const_iterator cur;
        for( cur = rProps.begin(); cur != rProps.end(); cur++ )
        {
            OString attr = OUStringToOString( cur->first, RTL_TEXTENCODING_UTF8 );
            OString value = OUStringToOString( cur->second, RTL_TEXTENCODING_UTF8 );
        }
        return;
    }

    PropList::const_iterator cur;
    for( cur = rProps.begin(); cur != rProps.end(); cur++ )
        setProperty( xPeer, cur->first, cur->second );
}

void
setProperty( uno::Reference< uno::XInterface > const& xPeer,
             OUString const& attr, OUString const& value )
{
    OUString unoAttr = toUnoNaming( attr );

    // get a Property object
    beans::Property prop;
    try
    {
        uno::Reference< beans::XPropertySetInfo > xInfo
            = prophlp::queryPropertyInfo( xPeer );
        prop = xInfo->getPropertyByName( unoAttr );
    }
    catch( beans::UnknownPropertyException &ex )
    {
        DBG_ERROR1( "Warning: unknown attribute: `%s'", OUSTRING_CSTR( unoAttr ) );
        return;
    }

    if ( prop.Name.getLength() <= 0 )
    {
        DBG_ERROR1( "Warning: missing prop: `%s'", OUSTRING_CSTR( unoAttr ) );
        return;
    }

    // encapsulates value in an uno::Any
    uno::Any any;
    try
    {
        any = anyFromString( value, prop.Type );
    }
    catch( uno::RuntimeException &ex )
    {
        DBG_ERROR5( "Warning: %s( %s )( %s ) attribute is of type %s( rejected: %s )", OUSTRING_CSTR( unoAttr ), OUSTRING_CSTR( value ), OUSTRING_CSTR( prop.Name ),  OUSTRING_CSTR( prop.Type.getTypeName() ), OUSTRING_CSTR( value ) );
        return;
    }

    // sets value on property
    try
    {
        prophlp::setProperty( xPeer, unoAttr, any );
    }
    catch( ... )
    {
        DBG_ERROR2( "Warning: cannot set attribute %s to %s ", OUSTRING_CSTR( unoAttr ), OUSTRING_CSTR( value ) );
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
// Comment-out for <= m237
//    { "nolabel",      awt::VclWindowPeerAttribute::NOLABEL,      false },
    { "optimumsize",  awt::WindowAttribute::OPTIMUMSIZE,         false },
    { "readonly",     awt::VclWindowPeerAttribute::READONLY,     false },
    { "right",        awt::VclWindowPeerAttribute::RIGHT,        false },
    { "show",         awt::WindowAttribute::SHOW,                true },
    { "sizeable",     awt::WindowAttribute::SIZEABLE,            true },
    { "sort",         awt::VclWindowPeerAttribute::SORT,         false },
    { "spin",         awt::VclWindowPeerAttribute::SPIN,         false },
    { "vscroll",      awt::VclWindowPeerAttribute::VSCROLL,      false },

    // cutting on OK, YES_NO_CANCEL and related obscure attributes...
};
static const int attribsMapLen = sizeof( attribsMap ) / sizeof( AttributesMap );

#if 0
long getAttribute( const OUString &rName, bool bTopWindow )
{

    int min = 0, max = attribsMapLen - 1, mid, cmp;
    do
    {
        mid = min +( max - min )/2;
        cmp = rName.compareToAscii( attribsMap[ mid ].name );
        if ( cmp > 0 )
            min = mid+1;
        else if ( cmp < 0 )
            max = mid-1;
        else
        {
            if ( bTopWindow || attribsMap[ mid ].value )
                return attribsMap[ mid ].windowAttr;
            return 0;
        }
    }
    while ( min <= max );
    return 0;
}
#endif

void propsFromAttributes( const uno::Reference<xml::input::XAttributes> & xAttributes,
                          PropList &rProps, sal_Int32 nNamespace )
{


    sal_Int32 nAttrs = xAttributes->getLength();
    for( sal_Int32 i = 0; i < nAttrs; i++ )
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
    PropList::iterator cur;
    OUString aName = OUString::createFromAscii( pAttr );

    for( cur = rProps.begin(); cur != rProps.end(); cur++ )
    {
        if ( cur->first.equalsIgnoreAsciiCase( aName ) )
        {
            rValue = cur->second;
            rProps.erase( cur );
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

    for( int i = 0; i < attribsMapLen; i++ )
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

