#ifndef TABLE_HXX
#define TABLE_HXX

#include "container.hxx"

#include <list>

namespace layoutimpl
{

class Table : public Container
{
    friend class TableChildProps;
protected:
    // Table properties
    sal_Int32 mnColsLen;

    // Children properties
    struct ChildData
    {
        sal_Bool  bExpand[ 2 ];
        sal_Int32 nColSpan, nRowSpan;
        css::awt::Size aRequisition;
        css::uno::Reference< css::awt::XLayoutConstrains > xChild;
        css::uno::Reference< css::beans::XPropertySet >    xProps;
        bool isVisible();

        // automatically calculated
        int nLeftCol, nRightCol, nTopRow, nBottomRow;
    };
    std::list< ChildData * > maChildren;

    // a group of children; either a column or a row
    struct GroupData
    {
        sal_Bool bExpand;
        int nSize;  // request size (width or height)
        GroupData() : bExpand( false ), nSize( 0 ) {}
    };
    std::vector< GroupData > maCols, maRows;
    int mnColExpandables, mnRowExpandables;

public:
    Table();

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

    // unimplemented:
    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException)
    { return false; }
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 /*nWidth*/ )
    throw(css::uno::RuntimeException)
    { return maRequisition.Height; }

    PROPHELPER_SET_INFO
};

} //  namespace layoutimpl

#endif /*TABLE_HXX*/
