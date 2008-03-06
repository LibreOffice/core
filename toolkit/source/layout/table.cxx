#include <table.hxx>

#include <sal/macros.h>
#include <osl/mutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/awt/PosSize.hpp>
#include <tools/debug.hxx>

// fixed point precision for distributing error
#define FIXED_PT 16

namespace layoutimpl
{

using namespace com::sun::star;

class TableChildProps : public PropHelper
{
public:
    TableChildProps( Table::ChildData *pData )
    {
        addProp( RTL_CONSTASCII_USTRINGPARAM( "XExpand" ),
                 ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
                 &( pData->bExpand[ 0 ] ) );
        addProp( RTL_CONSTASCII_USTRINGPARAM( "YExpand" ),
                 ::getCppuType( static_cast< const sal_Bool* >( NULL ) ),
                 &( pData->bExpand[ 1 ] ) );
        addProp( RTL_CONSTASCII_USTRINGPARAM( "ColSpan" ),
                 ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
                 &( pData->nColSpan ) );
        addProp( RTL_CONSTASCII_USTRINGPARAM( "RowSpan" ),
                 ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
                 &( pData->nRowSpan ) );
    }
    PROPHELPER_SET_INFO
};

bool Table::ChildData::isVisible()
{
    if ( !xChild.is() || nColSpan < 1 || nRowSpan < 1 )
        return false;
    return xChild.is();
}

Table::Table()
    : Container()
{
    // another default value could be 0xffff for infinite columns( = 1 row )
    mnColsLen = 1;
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Columns" ),
             ::getCppuType( static_cast< const sal_Int32* >( NULL ) ),
             &mnColsLen );
}

void SAL_CALL
Table::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw( uno::RuntimeException, awt::MaxChildrenException )
{
    if ( xChild.is() )
    {
        ChildData *pData = new ChildData();
        pData->bExpand[ 0 ] = pData->bExpand[ 1 ] = true;
        pData->nColSpan = pData->nRowSpan = 1;
        pData->xChild = xChild;
        maChildren.push_back( pData );

        setChildParent( xChild );
        queueResize();

        // cause of flicker
        allocateChildAt( xChild, awt::Rectangle( 0,0,0,0 ) );
    }
}

void SAL_CALL
Table::removeChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw( uno::RuntimeException )
{
    for( std::list< ChildData * >::iterator it = maChildren.begin();
         it != maChildren.end(); it++ )
    {
        if ( ( *it )->xChild == xChild )
        {
            delete *it;
            maChildren.erase( it );

            unsetChildParent( xChild );
            queueResize();
            break;
        }
    }
    DBG_ERROR( "Table error: trying to remove child that doesn't exist" );
}

uno::Sequence< uno::Reference < awt::XLayoutConstrains > > SAL_CALL
Table::getChildren()
    throw( uno::RuntimeException )
{
    uno::Sequence< uno::Reference< awt::XLayoutConstrains > > children( maChildren.size() );
    unsigned int i = 0;
    for( std::list< ChildData * >::iterator it = maChildren.begin();
         it != maChildren.end(); it++, i++ )
        children[i] =( *it )->xChild;

    return children;
}

uno::Reference< beans::XPropertySet > SAL_CALL
Table::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw( uno::RuntimeException )
{
    std::list< ChildData * >::iterator iter;
    for( iter = maChildren.begin(); iter != maChildren.end(); iter++ )
    {
        if ( ( *iter )->xChild == xChild )
        {
            if ( !( *iter )->xProps.is() )
            {
                // FIXME: make me safe !
                PropHelper *pProps = new TableChildProps( *iter );
                pProps->setChangeListener( this );
( *iter )->xProps = pProps;
            }
            return( *iter )->xProps;
        }
    }
    return uno::Reference< beans::XPropertySet >();
}

awt::Size SAL_CALL
Table::getMinimumSize() throw( uno::RuntimeException )
{
    int nRowsLen = 0;

    // 1. layout the table -- adjust to cope with row-spans...
    {
        // temporary 1D representation of the table
        std::vector< ChildData *> aTable;

        int col = 0, row = 0;
        for( std::list<ChildData *>::iterator it = maChildren.begin();
             it != maChildren.end(); it++ )
        {
            ChildData *child = *it;
            if ( !child->isVisible() )
                continue;

            while ( col + SAL_MIN( child->nColSpan, mnColsLen ) > mnColsLen )
            {
                col = 0;
                row++;

                unsigned int i = col +( row*mnColsLen );
                while ( aTable.size() > i && !aTable[ i ] )
                    i++;

                col = i % mnColsLen;
                row = i / mnColsLen;
            }

            child->nLeftCol = col;
            child->nRightCol = SAL_MIN( col + child->nColSpan, mnColsLen );
            child->nTopRow = row;
            child->nBottomRow = row + child->nRowSpan;

            col += child->nColSpan;

            unsigned int start = child->nLeftCol +( child->nTopRow*mnColsLen );
            unsigned int end =( child->nRightCol-1 ) +( ( child->nBottomRow-1 )*mnColsLen );
            if ( aTable.size() < end+1 )
                aTable.resize( end+1, NULL );
            for( unsigned int i = start; i < end; i++ )
                aTable[ i ] = child;

            nRowsLen = SAL_MAX( nRowsLen, child->nBottomRow );
        }
    }

    // 2. calculate columns/rows sizes
    for( int g = 0; g < 2; g++ )
    {
        std::vector< GroupData > &aGroup = g == 0 ? maCols : maRows;

        aGroup.clear();
        aGroup.resize( g == 0 ? mnColsLen : nRowsLen );

        // 2.1 base sizes on one-column/row children
        for( std::list<ChildData *>::iterator it = maChildren.begin();
             it != maChildren.end(); it++ )
        {
            ChildData *child = *it;
            if ( !child->isVisible() )
                continue;
            const int nFirstAttach = g == 0 ? child->nLeftCol : child->nTopRow;
            const int nLastAttach  = g == 0 ? child->nRightCol : child->nBottomRow;

            if ( nFirstAttach == nLastAttach-1 )
            {
                child->aRequisition = child->xChild->getMinimumSize();
                int attach = nFirstAttach;
                int child_size = g == 0 ? child->aRequisition.Width
                    : child->aRequisition.Height;
                aGroup[ attach ].nSize = SAL_MAX( aGroup[ attach ].nSize, child_size );
                if ( child->bExpand[ g ] )
                    aGroup[ attach ].bExpand = true;
            }
        }

        // 2.2 make sure multiple-columns/rows children fit
        for( std::list<ChildData *>::iterator it = maChildren.begin();
             it != maChildren.end(); it++ )
        {
            ChildData *child = *it;
            if ( !child->isVisible() )
                continue;
            const int nFirstAttach = g == 0 ? child->nLeftCol : child->nTopRow;
            const int nLastAttach  = g == 0 ? child->nRightCol : child->nBottomRow;

            if ( nFirstAttach != nLastAttach-1 )
            {
                child->aRequisition = child->xChild->getMinimumSize();
                int size = 0;
                int expandables = 0;
                for( int i = nFirstAttach; i < nLastAttach; i++ )
                {
                    size += aGroup[ i ].nSize;
                    if ( aGroup[ i ].bExpand )
                        expandables++;
                }

                int child_size = g == 0 ? child->aRequisition.Width
                    : child->aRequisition.Height;
                int extra = child_size - size;
                if ( extra > 0 )
                {
                    if ( expandables )
                        extra /= expandables;
                    else
                        extra /= nLastAttach - nFirstAttach;

                    for( int i = nFirstAttach; i < nLastAttach; i++ )
                        if ( expandables == 0 || aGroup[ i ].bExpand )
                            aGroup[ i ].nSize += extra;
                }
            }
        }
    }

    // 3. Sum everything up
    mnColExpandables =( mnRowExpandables = 0 );
    maRequisition.Width =( maRequisition.Height = 0 );
    for( std::vector<GroupData>::iterator it = maCols.begin();
         it != maCols.end(); it++ )
    {
        maRequisition.Width += it->nSize;
        if ( it->bExpand )
            mnColExpandables++;
    }
    for( std::vector<GroupData>::iterator it = maRows.begin();
         it != maRows.end(); it++ )
    {
        maRequisition.Height += it->nSize;
        if ( it->bExpand )
            mnRowExpandables++;
    }

    return maRequisition;
}

void SAL_CALL
Table::allocateArea( const awt::Rectangle &rArea )
    throw( uno::RuntimeException )
{
    maAllocation = rArea;
    if ( maCols.size() == 0 || maRows.size() == 0 )
        return;

    int nExtraSize[ 2 ] = { SAL_MAX( rArea.Width - maRequisition.Width, 0 ),
                            SAL_MAX( rArea.Height - maRequisition.Height, 0 ) };
    // split it
    nExtraSize[ 0 ] /= mnColExpandables ? mnColExpandables : mnColsLen;
    nExtraSize[ 1 ] /= mnRowExpandables ? mnRowExpandables : maRows.size();

    for( std::list<ChildData *>::const_iterator it = maChildren.begin();
         it != maChildren.end(); it++ )
    {
        ChildData *child = *it;
        if ( !child->isVisible() )
            continue;

        awt::Rectangle rChildArea( rArea.X, rArea.Y, 0, 0 );

        for( int g = 0; g < 2; g++ )
        {
            std::vector< GroupData > &aGroup = g == 0 ? maCols : maRows;
            const int nFirstAttach = g == 0 ? child->nLeftCol : child->nTopRow;
            const int nLastAttach  = g == 0 ? child->nRightCol : child->nBottomRow;

            for( int i = 0; i < nFirstAttach; i++ )
            {
                int gSize = aGroup[ i ].nSize;
                if ( aGroup[ i ].bExpand )
                    gSize += nExtraSize[ g ];
                if ( g == 0 )
                    rChildArea.X += gSize;
                else
                    rChildArea.Y += gSize;
            }
            for( int i = nFirstAttach; i < nLastAttach; i++ )
            {
                int gSize = aGroup[ i ].nSize;
                if ( aGroup[ i ].bExpand )
                    gSize += nExtraSize[ g ];
                if ( g == 0 )
                    rChildArea.Width  += gSize;
                else
                    rChildArea.Height += gSize;
            }
        }

        allocateChildAt( child->xChild, rChildArea );
    }
}

} // namespace layoutimpl
