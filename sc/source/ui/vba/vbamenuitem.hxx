#ifndef SC_VBA_MENUITEM_HXX
#define SC_VBA_MENUITEM_HXX

#include <ooo/vba/excel/XMenuItem.hpp>
#include <ooo/vba/XCommandBarControl.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XMenuItem > MenuItem_BASE;

class ScVbaMenuItem : public MenuItem_BASE
{
private:
    css::uno::Reference< ov::XCommandBarControl > m_xCommandBarControl;

public:
    ScVbaMenuItem( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< ov::XCommandBarControl >& xCommandBarControl ) throw( css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getCaption() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getOnAction() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setOnAction( const ::rtl::OUString& _onaction ) throw (css::uno::RuntimeException);

    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif//SC_VBA_MENUITEM_HXX
