#ifndef _LAYOUT_CORE_HXX_
#define _LAYOUT_CORE_HXX_

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XLayoutContainer.hpp>
#include <com/sun/star/awt/XLayoutConstrains.hpp>

#include <layout/layout.hxx>

namespace layoutimpl
{

namespace css = layout::css;

css::uno::Reference< css::awt::XLayoutContainer >
createContainer( const rtl::OUString &rName );

css::uno::Reference< css::awt::XLayoutConstrains >
createWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
              css::uno::Reference< css::uno::XInterface > xParent,
              const rtl::OUString &rName, long nProps );

css::uno::Reference< css::awt::XLayoutConstrains >
createInternalWidget( css::uno::Reference< css::awt::XToolkit > xToolkit,
                      css::uno::Reference< css::uno::XInterface > xParent,
                      const rtl::OUString &rName, long nProps );

css::uno::Reference< css::graphic::XGraphic > loadGraphic( const char *pName );

// Helpers - unfortunately VCLXWindows don't implement XPropertySet
// but containers do - these helpers help us to hide this
namespace prophlp
{
// can we set properties on this handle ?
bool canHandleProps( const css::uno::Reference< css::uno::XInterface > &xRef );
// if so which properties ?
css::uno::Reference< css::beans::XPropertySetInfo > queryPropertyInfo(
    const css::uno::Reference< css::uno::XInterface > &xRef );
// set / get ...
void setProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                  const rtl::OUString &rName,
                  css::uno::Any aValue );
css::uno::Any getProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                           const rtl::OUString &rName );
} // namespace prophlp
} // namespace layoutimpl

#endif // _LAYOUT_CORE_HXX_
