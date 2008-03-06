#ifndef CORE_BOX_HXX
#define CORE_BOX_HXX

#include "container.hxx"

#include <com/sun/star/awt/Point.hpp>

#include <list>

namespace layoutimpl
{

class Box : public Container
{
    friend class BoxChildProps;
protected:
    // Box properties (i.e. affect all children)
    sal_Int32 mnSpacing;
    sal_Bool mbHomogeneous;
    sal_Bool mbHorizontal;  // false for Vertical

    // Children properties
    struct ChildData
    {
      sal_Int32 nPadding;
      sal_Bool  bExpand, bFill;
      css::awt::Size aRequisition;
      css::uno::Reference< css::awt::XLayoutConstrains > xChild;
      css::uno::Reference< css::beans::XPropertySet > xProps;
      bool isVisible();
    };
    std::list< ChildData * > maChildren;
    bool mbHasFlowChildren;

public:
    Box( bool horizontal );

    static ChildData *createChild( css::uno::Reference< css::awt::XLayoutConstrains > const& xChild );

    ChildData *removeChildData( std::list< ChildData *>, css::uno::Reference< css::awt::XLayoutConstrains > const& Child );

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

    // helper: mix of getMinimumSize() and getHeightForWidth()
    css::awt::Size calculateSize( long nWidth = 0 );

    PROPHELPER_SET_INFO

private:
    /* Helpers to deal with the joint Box directions. */
    inline int primDim (const css::awt::Size &size)
    { if (mbHorizontal) return size.Width; else return size.Height; }
    inline int secDim (const css::awt::Size &size)
    { if (mbHorizontal) return size.Height; else return size.Width; }
    inline int primDim (const css::awt::Point &point)
    { if (mbHorizontal) return point.X; else return point.Y; }
    inline int secDim (const css::awt::Point &point)
    { if (mbHorizontal) return point.Y; else return point.X; }
};

struct VBox : public Box
{ VBox() : Box (false) {} };

struct HBox : public Box
{ HBox() : Box (true) {} };

} //  namespace layoutimpl

#endif /*CORE_BOX_HXX*/
