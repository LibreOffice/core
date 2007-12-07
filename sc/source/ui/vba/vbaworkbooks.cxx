/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaworkbooks.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:06:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <comphelper/processfactory.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>

#include "vbaglobals.hxx"
#include "vbaworkbook.hxx"
#include "vbaworkbooks.hxx"
#include "vbahelper.hxx"

#include <hash_map>
#include <osl/file.hxx>
using namespace ::org::openoffice;
using namespace ::com::sun::star;

const sal_Int16 CUSTOM_CHAR = 5;

typedef  std::hash_map< rtl::OUString,
sal_Int32, ::rtl::OUStringHash,
::std::equal_to< ::rtl::OUString > > NameIndexHash;

typedef std::vector < uno::Reference< sheet::XSpreadsheetDocument > > WorkBooks;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > SpreadSheetDocEnumImpl_BASE;

static uno::Any
getWorkbook( uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSpreadsheetDocument > &xDoc )
{
    // FIXME: fine as long as ScVbaWorkbook is stateless ...
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if( !xModel.is() )
        return uno::Any();

    ScVbaWorkbook *pWb = new ScVbaWorkbook(  uno::Reference< vba::XHelperInterface >( ScVbaGlobals::getGlobalsImpl( xContext )->getApplication(), uno::UNO_QUERY_THROW ), xContext, xModel );
    return uno::Any( uno::Reference< excel::XWorkbook > (pWb) );
}


// #FIXME clearly this is a candidate for some sort of helper base class as
// this is a copy of SelectedSheetsEnum ( vbawindow.cxx )

class SpreadSheetDocEnumImpl : public SpreadSheetDocEnumImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    WorkBooks m_books;
    WorkBooks::const_iterator m_it;

public:
    SpreadSheetDocEnumImpl( const uno::Reference< uno::XComponentContext >& xContext, const WorkBooks& books ) throw ( uno::RuntimeException ) :  m_xContext( xContext ), m_books( books )
    {
        m_it = m_books.begin();
    }
    SpreadSheetDocEnumImpl( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException ) :  m_xContext( xContext )
    {
        uno::Reference< lang::XMultiComponentFactory > xSMgr(
            m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );

        uno::Reference< frame::XDesktop > xDesktop
            (xSMgr->createInstanceWithContext(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"), m_xContext), uno::UNO_QUERY_THROW );
        uno::Reference< container::XEnumeration > mxComponents = xDesktop->getComponents()->createEnumeration();
        while( mxComponents->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( mxComponents->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
                m_books.push_back( xNext );
        }
        m_it = m_books.begin();
    }
    // XEnumeration
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return m_it != m_books.end();
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasMoreElements() )
        {
            throw container::NoSuchElementException();
        }
        return makeAny( *(m_it++) );
    }
};

class WorkBookEnumImpl : public EnumerationHelperImpl
{
public:
    WorkBookEnumImpl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ){}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< sheet::XSpreadsheetDocument > xDoc( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return getWorkbook( m_xContext, xDoc );
    }

};

// #FIXME clearly this is also a candidate for some sort of helper base class as
// a very similar one is used in vbawindow ( SelectedSheetsEnumAccess )
// Maybe a template base class that does all of the operations on the hashmap
// and vector only, and the sub-class does everything else
// => ctor, createEnumeration & factory method need be defined ( to be called
// by getByIndex, getByName )
typedef ::cppu::WeakImplHelper3< container::XEnumerationAccess
    , com::sun::star::container::XIndexAccess
    , com::sun::star::container::XNameAccess
    > WorkBooksAccessImpl_BASE;

class WorkBooksAccessImpl : public WorkBooksAccessImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    WorkBooks m_books;
    NameIndexHash namesToIndices;
public:
    WorkBooksAccessImpl( const uno::Reference< uno::XComponentContext >& xContext ):m_xContext( xContext )
    {
        uno::Reference< container::XEnumeration > xEnum = new SpreadSheetDocEnumImpl( m_xContext );
        sal_Int32 nIndex=0;
        while( xEnum->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( xEnum->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
            {
                m_books.push_back( xNext );
                uno::Reference< frame::XModel > xModel( xNext, uno::UNO_QUERY_THROW ); // that the spreadsheetdocument is a xmodel is a given
                INetURLObject aURL( xModel->getURL() );
                namesToIndices[ aURL.GetLastName() ] = nIndex++;
            }
        }

    }

    //XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new SpreadSheetDocEnumImpl( m_xContext, m_books );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return m_books.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0
            || static_cast< WorkBooks::size_type >(Index) >= m_books.size() )
            throw lang::IndexOutOfBoundsException();
        return makeAny( m_books[ Index ] ); // returns xspreadsheetdoc
    }

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return sheet::XSpreadsheetDocument::static_type(0);
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return (m_books.size() > 0);
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return makeAny( m_books[ it->second ] );

    }

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > names( namesToIndices.size() );
        ::rtl::OUString* pString = names.getArray();
        NameIndexHash::const_iterator it = namesToIndices.begin();
        NameIndexHash::const_iterator it_end = namesToIndices.end();
        for ( ; it != it_end; ++it, ++pString )
            *pString = it->first;
        return names;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        return (it != namesToIndices.end());
    }

};

ScVbaWorkbooks::ScVbaWorkbooks( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaWorkbooks_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new WorkBooksAccessImpl( xContext ) ) )
{
}
// XEnumerationAccess
uno::Type
ScVbaWorkbooks::getElementType() throw (uno::RuntimeException)
{
    return excel::XWorkbook::static_type(0);
}
uno::Reference< container::XEnumeration >
ScVbaWorkbooks::createEnumeration() throw (uno::RuntimeException)
{
    // #FIXME its possible the WorkBookEnumImpl here doens't reflect
    // the state of this object ( although it should ) would be
    // safer to create an enumeration based on this objects state
    // rather than one effectively based of the desktop component
    return new WorkBookEnumImpl( mxContext, uno::Reference< container::XEnumeration >( new SpreadSheetDocEnumImpl(mxContext) ) );
}

uno::Any
ScVbaWorkbooks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( aSource, uno::UNO_QUERY );
    return getWorkbook( mxContext, xDoc );
}


uno::Any SAL_CALL
ScVbaWorkbooks::Add() throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiComponentFactory > xSMgr(
        mxContext->getServiceManager(), uno::UNO_QUERY_THROW );

     uno::Reference< frame::XComponentLoader > xLoader(
        xSMgr->createInstanceWithContext(
            ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"),
                mxContext), uno::UNO_QUERY_THROW );
    uno::Reference< lang::XComponent > xComponent = xLoader->loadComponentFromURL(
                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc") ),
                                       rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ), 0,
                                       uno::Sequence< beans::PropertyValue >(0) );
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xComponent, uno::UNO_QUERY_THROW );

    if( xSpreadDoc.is() )
        return getWorkbook( mxContext, xSpreadDoc );
    return uno::Any();
}

void
ScVbaWorkbooks::Close() throw (uno::RuntimeException)
{
    uno::Reference< lang::XMultiComponentFactory > xSMgr(
        mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CloseDoc"));
    dispatchRequests(xModel,url);
}

bool
ScVbaWorkbooks::isTextFile( const rtl::OUString& sType )
{
    // will return true if the file is
    // a) a variant of a text file
    // b) a csv file
    // c) unknown
    // returning true basically means treat this like a csv file
    const static rtl::OUString txtType( RTL_CONSTASCII_USTRINGPARAM("writer_Text" ) );
    const static rtl::OUString csvType( RTL_CONSTASCII_USTRINGPARAM("calc_Text_txt_csv_StarCalc" ) );
    const static rtl::OUString encodedTxtType( RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded" ) );
    return sType.equals( txtType ) || sType.equals( csvType ) || ( sType.getLength() == 0 ) || sType.equals( encodedTxtType );
}

bool
ScVbaWorkbooks::isSpreadSheetFile( const rtl::OUString& sType )
{
    // include calc_QPro etc. ? ( not for the moment anyway )
    if ( sType.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("calc_MS"))) == 0
    || sType.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("calc8"))) == 0
    || sType.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice"))) == 0 )
        return true;
    return false;
}

rtl::OUString
ScVbaWorkbooks::getFileFilterType( const rtl::OUString& rFileName )
{
    uno::Reference< document::XTypeDetection > xTypeDetect( mxContext->getServiceManager()->createInstanceWithContext(::rtl::OUString::createFromAscii("com.sun.star.document.TypeDetection"), mxContext), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aMediaDesc(1);
    aMediaDesc[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("URL" ) );
    aMediaDesc[ 0 ].Value <<= rFileName;
    rtl::OUString sType = xTypeDetect->queryTypeByDescriptor( aMediaDesc, sal_True );
    return sType;
}

// #TODO# #FIXME# can any of the unused params below be used?
uno::Any
ScVbaWorkbooks::Open( const rtl::OUString& rFileName, const uno::Any& /*UpdateLinks*/, const uno::Any& ReadOnly, const uno::Any& Format, const uno::Any& /*Password*/, const uno::Any& /*WriteResPassword*/, const uno::Any& /*IgnoreReadOnlyRecommended*/, const uno::Any& /*Origin*/, const uno::Any& Delimiter, const uno::Any& /*Editable*/, const uno::Any& /*Notify*/, const uno::Any& /*Converter*/, const uno::Any& /*AddToMru*/ ) throw (uno::RuntimeException)
{
    // we need to detect if this is a URL, if not then assume its a file path
        rtl::OUString aURL;
        INetURLObject aObj;
    aObj.SetURL( rFileName );
    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
    if ( bIsURL )
        aURL = rFileName;
    else
        osl::FileBase::getFileURLFromSystemPath( rFileName, aURL );
    uno::Reference< lang::XMultiComponentFactory > xSMgr(
        mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XDesktop > xDesktop
        (xSMgr->createInstanceWithContext(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")                    , mxContext),
        uno::UNO_QUERY_THROW );
    uno::Reference< frame::XComponentLoader > xLoader(
        xSMgr->createInstanceWithContext(
        ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop"),
        mxContext),
        uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > sProps(0);
    sal_Int32 nIndex = 0;

    rtl::OUString sType = getFileFilterType( aURL );
    // A text file means it needs to be processed as a csv file
    if ( isTextFile( sType ) )
    {
        // Values for format
        // 1 Tabs
        // 2 Commas
        // 3 Spaces
        // 4 Semicolons
        // 5 Nothing
        // 6 Custom character (see the Delimiter argument
        // no format means use the current delimiter
        sProps.realloc( 3 );
        sProps[ nIndex ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FilterOptions" ) );
        sal_Int16 delims[] = { 0 /*default not used*/, 9/*tab*/, 44/*comma*/, 32/*space*/, 59/*semicolon*/ };
        static rtl::OUString sRestOfFormat( RTL_CONSTASCII_USTRINGPARAM(",34,0,1" ) );

        rtl::OUString sFormat;
        sal_Int16 nFormat = 0; // default indicator


        if ( Format.hasValue() )
        {
            Format >>= nFormat; // val of nFormat overwritten if extracted
            // validate param
            if ( nFormat < 1 || nFormat > 6 )
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Illegal value for Format" ) ), uno::Reference< uno::XInterface >() );
        }

        sal_Int16 nDelim = getCurrentDelim();

        if (  nFormat > 0 && nFormat < CUSTOM_CHAR )
        {
            nDelim =  delims[ nFormat ];
        }
        else if ( nFormat > CUSTOM_CHAR )
        {
            // Need to check Delimiter param
            if ( !Delimiter.hasValue() )
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Expected value for Delimiter" ) ), uno::Reference< uno::XInterface >() );
            rtl::OUString sStr;
            Delimiter >>= sStr;
            String aUniStr( sStr );
            if ( aUniStr.Len() )
                nDelim = aUniStr.GetChar(0);
            else
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Incorrect value for Delimiter" ) ), uno::Reference< uno::XInterface >() );
        }

        getCurrentDelim() = nDelim; //set new current

        sFormat = rtl::OUString::valueOf( (sal_Int32)nDelim ) + sRestOfFormat;
        sProps[ nIndex++ ].Value <<= sFormat;
        sProps[ nIndex ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FilterName") );
        sProps[ nIndex++ ].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Text - txt - csv (StarCalc)") );
        // Ensure WORKAROUND_CSV_TXT_BUG_i60158 gets called in typedetection.cxx so
        // csv is forced for deep detected 'writerxxx' types
        sProps[ nIndex ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DocumentService") );
        sProps[ nIndex ].Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument") );
    }
    else if ( !isSpreadSheetFile( sType ) )
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Bad Format")), uno::Reference< uno::XInterface >() );

    if ( ReadOnly.hasValue()  )
    {
        sal_Bool bIsReadOnly = sal_False; ReadOnly >>= bIsReadOnly;
        if ( bIsReadOnly )
        {
            static const rtl::OUString sReadOnly( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
            sProps.realloc( sProps.getLength() + 1 );
            sProps[ nIndex ].Name = sReadOnly;
            sProps[ nIndex++ ].Value = uno::makeAny( (sal_Bool)sal_True );
        }
    }

    uno::Reference< lang::XComponent > xComponent = xLoader->loadComponentFromURL( aURL,
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_default") ),
        frame::FrameSearchFlag::CREATE,
        sProps);
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xComponent, uno::UNO_QUERY_THROW );
    uno::Any aRet = getWorkbook( mxContext, xSpreadDoc );
    uno::Reference< excel::XWorkbook > xWBook( aRet, uno::UNO_QUERY );
    if ( xWBook.is() )
        xWBook->Activate();
    return aRet;
}

rtl::OUString&
ScVbaWorkbooks::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaWorkbooks") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaWorkbooks::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Workbooks") );
    }
    return sNames;
}
