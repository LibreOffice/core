#ifndef CORE_PROPLIST_HXX
#define CORE_PROPLIST_HXX

#include <list>
#include <com/sun/star/xml/input/XAttributes.hpp>

#include <rtl/ustring.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;

typedef std::list< std::pair< rtl::OUString, rtl::OUString > > PropList;

void propsFromAttributes( const css::uno::Reference<css::xml::input::XAttributes> & xAttributes,
                          PropList &rProps, sal_Int32 nNamespace );

void setProperties( css::uno::Reference< css::uno::XInterface > const& xPeer,
                    PropList const& rProps);

void setProperty( css::uno::Reference< css::uno::XInterface > const& xPeer,
                  rtl::OUString const& attr, rtl::OUString const& value );

long getAttributeProps( PropList &rProps );
bool findAndRemove( const char *pAttr, PropList &rProps, rtl::OUString &rValue);

}

// Convert a rtl::OUString to a byte string.
#define OUSTRING_CSTR( str ) \
    rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US ).getStr()

#endif // CORE_PROPLIST_HXX
