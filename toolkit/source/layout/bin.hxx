/* A few simple binary containers */

#ifndef CORE_BIN_HXX
#define CORE_BIN_HXX

#include "container.hxx"

namespace layoutimpl
{

class Bin : public Container
{
protected:
    // Child
    css::awt::Size maChildRequisition;
    css::uno::Reference< css::awt::XLayoutConstrains > mxChild;

public:
    Bin();
    virtual ~Bin() {}

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException);
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference
                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException);

    // css::awt::XLayoutConstrains
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);

    PROPHELPER_SET_INFO
};

// Align gives control over child position on the allocated space.
class Align : public Bin
{
    friend class AlignChildProps;
protected:
    // properties
    float fHorAlign, fVerAlign;
    float fHorFill, fVerFill;

public:
    Align();

    // css::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);

    PROPHELPER_SET_INFO
};

// Makes child request its or a specified size, whatever is larger.
class MinSize : public Bin
{
protected:
    // properties
    long mnMinWidth, mnMinHeight;

public:
    MinSize();

    // css::awt::XLayoutContainer
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);

    PROPHELPER_SET_INFO
};

} //  namespace layoutimpl

#endif /*CORE_BIN_HXX*/
