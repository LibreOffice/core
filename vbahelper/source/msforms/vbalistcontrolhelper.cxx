/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <vbalistcontrolhelper.hxx>
#include <vector>
#include <vbahelper/vbapropvalue.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

const static rtl::OUString ITEMS( RTL_CONSTASCII_USTRINGPARAM("StringItemList") );

class ListPropListener : public PropListener
{
private:
    uno::Reference< beans::XPropertySet > m_xProps;
    uno::Any m_pvargIndex;
    uno::Any m_pvarColumn;

public:
    ListPropListener( const uno::Reference< beans::XPropertySet >& xProps, const uno::Any& pvargIndex, const uno::Any& pvarColumn );
    virtual void setValueEvent( const css::uno::Any& value );
    virtual css::uno::Any getValueEvent();
};

ListPropListener::ListPropListener( const uno::Reference< beans::XPropertySet >& xProps, const uno::Any& pvargIndex, const uno::Any& pvarColumn ) : m_xProps( xProps ), m_pvargIndex( pvargIndex ), m_pvarColumn( pvarColumn )
{
}

void ListPropListener::setValueEvent( const uno::Any& value )
{
    if( m_pvargIndex.hasValue() || m_pvarColumn.hasValue() )
        throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "Bad argument" )), uno::Reference< uno::XInterface >() );

    m_xProps->setPropertyValue( ITEMS, value );
}

uno::Any ListPropListener::getValueEvent()
{
    uno::Sequence< rtl::OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
    sal_Int16 nLength = static_cast< sal_Int16 >( sList.getLength() );
    uno::Any aRet;
    if ( m_pvargIndex.hasValue() )
    {
        sal_Int16 nIndex = -1;
        m_pvargIndex >>= nIndex;
        if( nIndex < 0 || nIndex >= nLength )
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Bad row Index" )), uno::Reference< uno::XInterface >() );
        aRet <<= sList[ nIndex ];
    }
    else if ( m_pvarColumn.hasValue() ) // pvarColumn on its own would be bad
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Bad column Index" )), uno::Reference< uno::XInterface >() );
    else // List() ( e.g. no args )
    {
        uno::Sequence< uno::Sequence< rtl::OUString > > sReturnArray( nLength );
        for ( sal_Int32 i = 0; i < nLength; ++i )
        {
            sReturnArray[ i ].realloc( 10 );
            sReturnArray[ i ][ 0 ] = sList[ i ];
        }
        aRet = uno::makeAny( sReturnArray );
    }
    return aRet;
}

void SAL_CALL
ListControlHelper::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex ) throw (uno::RuntimeException)
{
    if ( pvargItem.hasValue()  )
    {
        uno::Sequence< rtl::OUString > sList;
        m_xProps->getPropertyValue( ITEMS ) >>= sList;

        sal_Int32 nIndex = sList.getLength();

        if ( pvargIndex.hasValue() )
            pvargIndex >>= nIndex;

        rtl::OUString sString = getAnyAsString( pvargItem );

        // if no index specified or item is to be appended to end of
        // list just realloc the array and set the last item
        if ( nIndex  == sList.getLength() )
        {
            sal_Int32 nOldSize = sList.getLength();
            sList.realloc( nOldSize + 1 );
            sList[ nOldSize ] = sString;
        }
        else
        {
            // just copy those elements above the one to be inserted
            std::vector< rtl::OUString > sVec;
            // reserve just the amount we need to copy
            sVec.reserve( sList.getLength() - nIndex );

            // point at first element to copy
            rtl::OUString* pString = sList.getArray() + nIndex;
            const rtl::OUString* pEndString = sList.getArray() + sList.getLength();
            // insert the new element
            sVec.push_back( sString );
            // copy elements
            for ( ; pString != pEndString; ++pString )
                sVec.push_back( *pString );

            sList.realloc(  sList.getLength() + 1 );

            // point at first element to be overwritten
            pString = sList.getArray() + nIndex;
            pEndString = sList.getArray() + sList.getLength();
            std::vector< rtl::OUString >::iterator it = sVec.begin();
            for ( ; pString != pEndString; ++pString, ++it)
                *pString = *it;
            //
        }

        m_xProps->setPropertyValue( ITEMS, uno::makeAny( sList ) );

    }
}

void SAL_CALL
ListControlHelper::removeItem( const uno::Any& index ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    // for int index
    if ( index >>= nIndex  )
    {
        uno::Sequence< rtl::OUString > sList;
        m_xProps->getPropertyValue( ITEMS ) >>= sList;
        if( nIndex < 0 || nIndex > ( sList.getLength() - 1 ) )
            throw uno::RuntimeException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid index")), uno::Reference< uno::XInterface > () );
        if( sList.hasElements() )
        {
            if( sList.getLength() == 1 )
            {
                Clear();
                return;
            }
            for( sal_Int32 i = nIndex; i < ( sList.getLength()-1 ); i++ )
            {
                sList[i] = sList[i+1];
            }
            sList.realloc(  sList.getLength() - 1 );
        }

        m_xProps->setPropertyValue( ITEMS, uno::makeAny( sList ) );
    }
}

void SAL_CALL
ListControlHelper::Clear(  ) throw (uno::RuntimeException)
{
    // urk, setValue doesn't seem to work !!
    //setValue( uno::makeAny( sal_Int16() ) );
    m_xProps->setPropertyValue( ITEMS, uno::makeAny( uno::Sequence< rtl::OUString >() ) );
}

void SAL_CALL
ListControlHelper::setRowSource( const rtl::OUString& _rowsource ) throw (uno::RuntimeException)
{
    if ( _rowsource.getLength() == 0 )
        Clear();
}

sal_Int32 SAL_CALL
ListControlHelper::getListCount() throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
    return sList.getLength();
}

uno::Any SAL_CALL
ListControlHelper::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn ) throw (uno::RuntimeException)
{
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( new ListPropListener( m_xProps, pvargIndex, pvarColumn ) ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
