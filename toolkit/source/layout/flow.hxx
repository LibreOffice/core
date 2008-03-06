#ifndef CORE_FLOW_HXX
#define CORE_FLOW_HXX

#include "container.hxx"

#include <list>

namespace layoutimpl
{

class Flow : public Container
{
protected:
    // Box properties (i.e. affect all children)
    sal_Int32 mnSpacing;
    sal_Bool mbHomogeneous;

    // Children properties
    struct ChildData
    {
        css::awt::Size aRequisition;
        css::uno::Reference< css::awt::XLayoutConstrains > xChild;
        css::uno::Reference< css::beans::XPropertySet >    xProps;
        bool isVisible();
    };
    std::list< ChildData * > maChildren;
    long mnEachWidth;  // on homogeneous, the width of every child

public:
    Flow();

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference
                                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException);

PROPHELPER_SET_INFO

private:
    // shared between getMinimumSize() and getHeightForWidth()
    css::awt::Size calculateSize( long nMaxWidth );
};

} //  namespace layoutimpl

#endif /*FLOW_CORE_HXX*/
