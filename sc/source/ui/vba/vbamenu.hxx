#ifndef SC_VBA_MENU_HXX
#define SC_VBA_MENU_HXX

#include <ooo/vba/excel/XMenu.hpp>
#include <ooo/vba/XCommandBarControl.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XMenu > Menu_BASE;

class ScVbaMenu : public Menu_BASE
{
private:
    css::uno::Reference< ov::XCommandBarControl > m_xCommandBarControl;

public:
    ScVbaMenu( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< ov::XCommandBarControl >& xCommandBarControl ) throw( css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException);

    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL MenuItems( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif//SC_VBA_MENU_HXX
