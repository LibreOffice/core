#include "box.hxx"

#include <tools/debug.hxx>
#include <sal/macros.h>

// fixed point precision for distributing error
#define FIXED_PT 16

namespace layoutimpl
{

using namespace css;

class BoxChildProps : public PropHelper
{
public:
    BoxChildProps( Box::ChildData *pData )
    {
        addProp( RTL_CONSTASCII_USTRINGPARAM( "Expand" ),
                 ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
                 &(pData->bExpand) );
        addProp( RTL_CONSTASCII_USTRINGPARAM( "Fill" ),
                 ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
                 &(pData->bFill) );
        addProp( RTL_CONSTASCII_USTRINGPARAM( "Padding" ),
                 ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
                 &(pData->nPadding) );
    }
    PROPHELPER_SET_INFO
};

bool Box::ChildData::isVisible()
{
    // FIXME: call the 'isVisible' method on it ?
    if ( !xChild.is() )
    {
        DBG_ERROR( "FIXME: invalid child !" );
    }
    return xChild.is();
}

Box::Box( bool horizontal )
    : Container()
    , mnSpacing( 0 )
    , mbHomogeneous( false )
    , mbHorizontal( horizontal )
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Homogeneous" ),
             ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
             &mbHomogeneous );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Spacing" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnSpacing );
    mbHasFlowChildren = false;
}

Box::ChildData *
Box::createChild( uno::Reference< awt::XLayoutConstrains > const& xChild )
{
    ChildData *p = new ChildData();

    p->nPadding = 0;
    p->bExpand = true;
    p->bFill = true;
    p->xChild = xChild;
    return p;
}

void SAL_CALL
Box::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException, awt::MaxChildrenException)
{
    if ( xChild.is() )
    {
        ChildData *pData = createChild( xChild );
        maChildren.push_back( pData );
        setChildParent( xChild );
        queueResize();
    }
}

Box::ChildData *
Box::removeChildData( std::list< ChildData *> lst, css::uno::Reference< css::awt::XLayoutConstrains > const& xChild )
{
    for( std::list< ChildData * >::iterator it = lst.begin();
         it != lst.end(); it++ )
    {
        if ( (*it)->xChild == xChild )
        {
            lst.erase( it );
            return *it;
        }
    }
    return 0;
}

void SAL_CALL
Box::removeChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    if ( ChildData *p = removeChildData( maChildren, xChild ) )
    {
        // CHECK: BoxChildProps leaks?
        delete p;
        unsetChildParent( xChild );
        queueResize();
    }
    else
    {
        DBG_ERROR( "Box: removeChild: no such child" );
    }
}

uno::Sequence< uno::Reference < awt::XLayoutConstrains > > SAL_CALL
Box::getChildren()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > children( maChildren.size() );
    unsigned int i = 0;
    for( std::list< ChildData * >::iterator it = maChildren.begin();
         it != maChildren.end(); it++, i++ )
        children[i] = (*it)->xChild;

    return children;
}

uno::Reference< beans::XPropertySet > SAL_CALL
Box::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    std::list< ChildData * >::iterator iter;
    for( iter = maChildren.begin(); iter != maChildren.end(); iter++)
    {
        if ( (*iter)->xChild == xChild )
        {
            if ( !(*iter)->xProps.is() )
            {
                // FIXME: make me safe !
                PropHelper *pProps = new BoxChildProps( *iter );
                pProps->setChangeListener( this );
                (*iter)->xProps = pProps;
            }
            return (*iter)->xProps;
        }
    }
    return uno::Reference< beans::XPropertySet >();
}

awt::Size
Box::calculateSize( long nWidth )
{
    int nVisibleChildren = 0;
    // primary vs secundary axis (instead of a X and Y)
    int nPrimSize = 0, nSecSize = 0;
    int nFlowMinWidth = 0;  // in case the box only has flow children

    mbHasFlowChildren = false;

    std::list<ChildData *>::const_iterator it;
    for( it = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = *it;
        if ( !child->isVisible() )
            continue;

        uno::Reference< awt::XLayoutContainer > xChildCont( child->xChild, uno::UNO_QUERY );
        bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();

        awt::Size aChildSize = child->aRequisition = child->xChild->getMinimumSize();

        if ( !mbHorizontal /*vertical*/ && bFlow )
        {
            if ( nFlowMinWidth == 0 || nFlowMinWidth > aChildSize.Width )
                nFlowMinWidth = aChildSize.Width;
            mbHasFlowChildren = true;
        }
        else
        {
            int size = primDim( aChildSize ) + child->nPadding * 2;
            if ( mbHomogeneous )
                nPrimSize = SAL_MAX( nPrimSize, size );
            else
                nPrimSize += size;

            nSecSize = SAL_MAX( nSecSize, secDim( aChildSize ) );
        }
        nVisibleChildren++;
    }

    if ( nVisibleChildren )
    {
        if ( mbHomogeneous )
            nPrimSize *= nVisibleChildren;
        nPrimSize += (nVisibleChildren - 1) * mnSpacing;
    }

    if ( mbHasFlowChildren )
    {
        if ( nWidth == 0 )
            nWidth = nSecSize ? nSecSize : nFlowMinWidth;
        for( it = maChildren.begin(); it != maChildren.end(); it++ )
        {
            ChildData *child = *it;
            if ( !child->isVisible() )
                continue;

            uno::Reference< awt::XLayoutContainer > xChildCont( child->xChild, uno::UNO_QUERY );
            bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();

            if ( bFlow )
                nPrimSize += xChildCont->getHeightForWidth( nWidth );
        }
    }

    nPrimSize += mnBorderWidth * 2;
    nSecSize += mnBorderWidth * 2;
    return awt::Size( mbHorizontal ? nPrimSize : nSecSize,
                      mbHorizontal ? nSecSize : nPrimSize );
}

awt::Size SAL_CALL
Box::getMinimumSize() throw(uno::RuntimeException)
{
    maRequisition = calculateSize();
    return maRequisition;
}

sal_Bool SAL_CALL
Box::hasHeightForWidth()
    throw(uno::RuntimeException)
{
    return mbHasFlowChildren;
}

sal_Int32 SAL_CALL
Box::getHeightForWidth( sal_Int32 nWidth )
    throw(uno::RuntimeException)
{
    if ( hasHeightForWidth() )
        return calculateSize( nWidth ).Height;
    return maRequisition.Height;
}

void SAL_CALL
Box::allocateArea( const awt::Rectangle &newArea )
    throw (uno::RuntimeException)
{
    maAllocation = newArea;
    int nVisibleChildren = 0, nExpandChildren = 0;

    std::list<ChildData *>::const_iterator it;
    for( it = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = *it;
        if ( child->isVisible() )
        {
            nVisibleChildren++;
            if ( child->bExpand )
                nExpandChildren++;
        }
    }
    if ( !nVisibleChildren )
        return;

    // split rectangle for dimension helpers
    awt::Point newPoint( newArea.X, newArea.Y );
    awt::Size newSize( newArea.Width, newArea.Height );

    int nExtraSpace;
    if ( mbHomogeneous )
        nExtraSpace = ( ( primDim( newSize ) - mnBorderWidth * 2 -
                          ( nVisibleChildren - 1 ) * mnSpacing )) / nVisibleChildren;
    else if ( nExpandChildren )
    {
        int reqSize = primDim( maRequisition );
        if ( !mbHorizontal && hasHeightForWidth() )
            reqSize = getHeightForWidth( newArea.Width );
        nExtraSpace = ( primDim( newSize ) - reqSize ) / nExpandChildren;
    }
    else
        nExtraSpace = 0;

    int nChildPrimPoint, nChildSecPoint, nChildPrimSize, nChildSecSize;

    int nStartPoint = primDim( newPoint ) + mnBorderWidth;
    int nBoxSecSize = SAL_MAX( 1, secDim( newSize ) - mnBorderWidth * 2 );

    for( it = maChildren.begin(); it != maChildren.end(); it++ )
    {
        ChildData *child = *it;
        if ( !child->isVisible() )
            continue;

        awt::Point aChildPos;
        int nBoxPrimSize;  // of the available box space

        if ( mbHomogeneous )
            nBoxPrimSize = nExtraSpace;
        else
        {
            uno::Reference< awt::XLayoutContainer > xChildCont( child->xChild, uno::UNO_QUERY );
            bool bFlow = xChildCont.is() && xChildCont->hasHeightForWidth();
            if ( !mbHorizontal && bFlow )
                nBoxPrimSize = xChildCont->getHeightForWidth( newArea.Width );
            else
                nBoxPrimSize = primDim( child->aRequisition );
            nBoxPrimSize += child->nPadding;
            if ( child->bExpand )
                nBoxPrimSize += nExtraSpace;
        }

        nChildPrimPoint = nStartPoint + child->nPadding;
        nChildSecPoint = secDim( newPoint ) + mnBorderWidth;

        nChildSecSize = nBoxSecSize;
        if ( child->bFill )
            nChildPrimSize = SAL_MAX( 1, nBoxPrimSize - child->nPadding);
        else
        {
            nChildPrimSize = primDim( child->aRequisition );
            nChildPrimPoint += (nBoxPrimSize - nChildPrimSize) / 2;

            nChildSecPoint += (nBoxSecSize - nChildSecSize) / 2;
        }

        awt::Rectangle area;
        area.X = mbHorizontal ? nChildPrimPoint : nChildSecPoint;
        area.Y = mbHorizontal ? nChildSecPoint : nChildPrimPoint;
        area.Width = mbHorizontal ? nChildPrimSize : nChildSecSize;
        area.Height = mbHorizontal ? nChildSecSize : nChildPrimSize;

        allocateChildAt( child->xChild, area );

        nStartPoint += nBoxPrimSize + mnSpacing + child->nPadding;
    }
}

} // namespace layoutimpl
