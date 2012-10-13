/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "vbatables.hxx"
#include "vbatable.hxx"
#include "vbarange.hxx"
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <comphelper/componentcontext.hxx>

using namespace ::ooo::vba;
using namespace css;

static uno::Reference< container::XIndexAccess > lcl_getTables( const uno::Reference< frame::XModel >& xDoc )
{
    uno::Reference< container::XIndexAccess > xTables;
    uno::Reference< text::XTextTablesSupplier > xSupp( xDoc, uno::UNO_QUERY );
    if ( xSupp.is() )
        xTables.set( xSupp->getTextTables(), uno::UNO_QUERY_THROW );
    return xTables;
}

static uno::Any lcl_createTable( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xDocument, const uno::Any& aSource )
{
    uno::Reference< text::XTextTable > xTextTable( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( xDocument, uno::UNO_QUERY_THROW );
    uno::Reference< word::XTable > xTable( new SwVbaTable( xParent, xContext, xTextDocument, xTextTable ) );
    return uno::makeAny( xTable );
}

static sal_Bool lcl_isInHeaderFooter( const uno::Reference< text::XTextTable >& xTable )
{
    uno::Reference< text::XTextContent > xTextContent( xTable, uno::UNO_QUERY_THROW );
    uno::Reference< text::XText > xText = xTextContent->getAnchor()->getText();
    uno::Reference< lang::XServiceInfo > xServiceInfo( xText, uno::UNO_QUERY_THROW );
    rtl::OUString aImplName = xServiceInfo->getImplementationName();
    if ( aImplName == "SwXHeadFootText" )
        return sal_True;
    return sal_False;
}

typedef ::cppu::WeakImplHelper1< css::container::XEnumeration > EnumBase;
typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XNameAccess > TableCollectionHelper_Base;
typedef std::vector< uno::Reference< text::XTextTable > > XTextTableVec;

class TableCollectionHelper : public TableCollectionHelper_Base
{
    XTextTableVec mxTables;
    XTextTableVec::iterator cachePos;

public:
    TableCollectionHelper( const uno::Reference< frame::XModel >& xDocument )
    {
        // only count the tables in the body text, not in the header/footer
        uno::Reference< container::XIndexAccess > xTables = lcl_getTables( xDocument );
        sal_Int32 nCount = xTables->getCount();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            uno::Reference< text::XTextTable > xTable( xTables->getByIndex( i ) , uno::UNO_QUERY_THROW );
            if( !lcl_isInHeaderFooter( xTable ) )
                mxTables.push_back( xTable );
        }
        cachePos = mxTables.begin();
    }
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return mxTables.size();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        uno::Reference< text::XTextTable > xTable( mxTables[ Index ], uno::UNO_QUERY_THROW );
        return uno::makeAny( xTable );
    }
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  text::XTextTable::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException) { return getCount() > 0 ; }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        uno::Reference< text::XTextTable > xTable( *cachePos, uno::UNO_QUERY_THROW );
        return uno::makeAny( xTable );
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > sNames( mxTables.size() );
        rtl::OUString* pString = sNames.getArray();
        XTextTableVec::iterator it = mxTables.begin();
        XTextTableVec::iterator it_end = mxTables.end();
        for ( ; it != it_end; ++it, ++pString )
        {
            uno::Reference< container::XNamed > xName( *it, uno::UNO_QUERY_THROW );
            *pString = xName->getName();
        }
        return sNames;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        cachePos = mxTables.begin();
        XTextTableVec::iterator it_end = mxTables.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            uno::Reference< container::XNamed > xName( *cachePos, uno::UNO_QUERY_THROW );
            if ( aName.equalsIgnoreAsciiCase( xName->getName() ) )
                break;
        }
        return ( cachePos != it_end );
    }
};

class TableEnumerationImpl : public EnumBase
{
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< frame::XModel > mxDocument;
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 mnCurIndex;
public:
    TableEnumerationImpl(  const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xDocument, const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxParent( xParent ), mxContext( xContext ), mxDocument( xDocument ), mxIndexAccess( xIndexAccess ), mnCurIndex(0)
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( mnCurIndex < mxIndexAccess->getCount() );
    }
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
            throw container::NoSuchElementException();
        return lcl_createTable( mxParent, mxContext, mxDocument, mxIndexAccess->getByIndex( mnCurIndex++ ) );
    }

};

SwVbaTables::SwVbaTables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xDocument ) : SwVbaTables_BASE( xParent, xContext , uno::Reference< container::XIndexAccess >( new TableCollectionHelper( xDocument ) ) ), mxDocument( xDocument )
{
}


uno::Reference< word::XTable > SAL_CALL
SwVbaTables::Add( const uno::Reference< word::XRange >& Range, const uno::Any& NumRows, const uno::Any& NumColumns, const uno::Any& /*DefaultTableBehavior*/, const uno::Any& /*AutoFitBehavior*/ ) throw (script::BasicErrorException, uno::RuntimeException)
{
    sal_Int32 nCols = 0;
    sal_Int32 nRows = 0;
    SwVbaRange* pVbaRange = dynamic_cast< SwVbaRange* >( Range.get() );
    // Preconditions
    if ( !( pVbaRange && ( NumRows >>= nRows ) && ( NumColumns >>= nCols ) ) )
        throw uno::RuntimeException(); // #FIXME better exception??
    if ( nCols <= 0 || nRows <= 0 )
        throw uno::RuntimeException(); // #FIXME better exception??

    uno::Reference< frame::XModel > xModel( pVbaRange->getDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< lang::XMultiServiceFactory > xMsf( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextRange > xTextRange = pVbaRange->getXTextRange();

    uno::Reference< text::XTextTable > xTable;
    xTable.set( xMsf->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextTable")) ), uno::UNO_QUERY_THROW );

    xTable->initialize( nRows, nCols );
    uno::Reference< text::XText > xText = xTextRange->getText();
    uno::Reference< text::XTextContent > xContext( xTable, uno::UNO_QUERY_THROW );

    xText->insertTextContent( xTextRange, xContext, true );

    // move the current cursor to the first table cell
    uno::Reference< table::XCellRange > xCellRange( xTable, uno::UNO_QUERY_THROW );
    uno::Reference< text::XText> xFirstCellText( xCellRange->getCellByPosition(0, 0), uno::UNO_QUERY_THROW );
    word::getXTextViewCursor( mxDocument )->gotoRange( xFirstCellText->getStart(), sal_False );

    uno::Reference< word::XTable > xVBATable( new SwVbaTable( mxParent, mxContext,  pVbaRange->getDocument(), xTable ) );
    return xVBATable;
}

uno::Reference< container::XEnumeration > SAL_CALL
SwVbaTables::createEnumeration() throw (uno::RuntimeException)
{
    return new TableEnumerationImpl( mxParent, mxContext, mxDocument, m_xIndexAccess );
}

// ScVbaCollectionBaseImpl
uno::Any
SwVbaTables::createCollectionObject( const uno::Any& aSource )
{
    return lcl_createTable( mxParent, mxContext, mxDocument, aSource );
}

// XHelperInterface
rtl::OUString
SwVbaTables::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaTables"));
}

// XEnumerationAccess
uno::Type SAL_CALL
SwVbaTables::getElementType() throw (uno::RuntimeException)
{
    return  word::XTable::static_type(0);
}

uno::Sequence<rtl::OUString>
SwVbaTables::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Tables" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
