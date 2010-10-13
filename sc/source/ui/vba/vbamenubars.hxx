#ifndef SC_VBA_MENUBARS_HXX
#define SC_VBA_MENUBARS_HXX

#include <ooo/vba/excel/XMenuBars.hpp>
#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBars.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XMenuBars > MenuBars_BASE;

class ScVbaMenuBars : public MenuBars_BASE
{
private:
    css::uno::Reference< ov::XCommandBars > m_xCommandBars;

public:
    ScVbaMenuBars( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< ov::XCommandBars >& xCommandBars ) throw (css::uno::RuntimeException);
    virtual ~ScVbaMenuBars();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& /*aIndex2*/ ) throw( css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SC_VBA_MENUBARS_HXX
