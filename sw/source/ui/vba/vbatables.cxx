#include "vbatables.hxx"
#include "vbatable.hxx"
#include "vbarange.hxx"
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <comphelper/componentcontext.hxx>

using namespace ::ooo::vba;
using namespace css;

uno::Reference< container::XIndexAccess > lcl_getTables( const uno::Reference< frame::XModel >& xDoc )
{
    uno::Reference< container::XIndexAccess > xTables;
    uno::Reference< text::XTextTablesSupplier > xSupp( xDoc, uno::UNO_QUERY );
    if ( xSupp.is() )
        xTables.set( xSupp->getTextTables(), uno::UNO_QUERY_THROW );
    return xTables;
}

uno::Any lcl_createTable( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< frame::XModel >& xDocument, const uno::Any& aSource )
{
    uno::Reference< text::XTextTable > xTextTable( aSource, uno::UNO_QUERY_THROW );
    uno::Reference< text::XTextDocument > xTextDocument( xDocument, uno::UNO_QUERY_THROW );
    uno::Reference< word::XTable > xTable( new SwVbaTable( xParent, xContext, xTextDocument, xTextTable ) );
    return uno::makeAny( xTable );
}

typedef ::cppu::WeakImplHelper1< css::container::XEnumeration > EnumBase;

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

SwVbaTables::SwVbaTables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< frame::XModel >& xDocument ) : SwVbaTables_BASE( xParent, xContext , lcl_getTables( xDocument ) ), mxDocument( xDocument )
{
}


uno::Reference< word::XTable > SAL_CALL
SwVbaTables::Add( const uno::Reference< word::XRange >& Range, const uno::Any& NumRows, const uno::Any& NumColumns, const uno::Any& DefaultTableBehavior, const uno::Any& AutoFitBehavior ) throw (script::BasicErrorException, uno::RuntimeException)
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
    xTable.set( xMsf->createInstance( rtl::OUString::createFromAscii("com.sun.star.text.TextTable") ), uno::UNO_QUERY_THROW );
/*
    comphelper::ComponentContext aCtx( xMsf );
    if ( !aCtx.createComponent( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextTable") ), xTable ) );
        throw uno::RuntimeException(); // #FIXME better exception??
*/
    xTable->initialize( nRows, nCols );
    uno::Reference< text::XText > xText = xTextRange->getText();
    uno::Reference< text::XTextContent > xContext( xTable, uno::UNO_QUERY_THROW );

    xText->insertTextContent( xTextRange, xContext, true );
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
rtl::OUString&
SwVbaTables::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaTables") );
    return sImplName;
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

