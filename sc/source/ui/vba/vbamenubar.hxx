#ifndef SC_VBA_MENUBAR_HXX
#define SC_VBA_MENUBAR_HXX

#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBar.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XMenuBar > MenuBar_BASE;

class ScVbaMenuBar : public MenuBar_BASE
{
private:
    css::uno::Reference< ov::XCommandBar > m_xCommandBar;

public:
    ScVbaMenuBar( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< ov::XCommandBar >& xCommandBar ) throw( css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL Menus( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif//SC_VBA_MENUBAR_HXX
