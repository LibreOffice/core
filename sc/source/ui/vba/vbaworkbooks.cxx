/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>

#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>

#include "vbaglobals.hxx"
#include "vbaworkbook.hxx"
#include "vbaworkbooks.hxx"
#include <vbahelper/vbahelper.hxx>

#include <boost/unordered_map.hpp>
#include <vector>
#include <osl/file.hxx>
using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int16 CUSTOM_CHAR = 5;

static uno::Any
getWorkbook( uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSpreadsheetDocument > &xDoc, const uno::Reference< XHelperInterface >& xParent )
{
    // FIXME: fine as long as ScVbaWorkbook is stateless ...
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if( !xModel.is() )
        return uno::Any();

    ScDocShell* pShell = excel::getDocShell( xModel );
    if ( pShell )
    {
        String sCodeName = pShell->GetDocument()->GetCodeName();
        uno::Reference< XHelperInterface > xIf = getUnoDocModule( sCodeName, pShell );
                if ( xIf.is() )
                {
                    OSL_TRACE(" *** Returning Module uno Object *** ");
                    return  uno::makeAny( xIf );
                }
    }

    ScVbaWorkbook *pWb = new ScVbaWorkbook(  xParent, xContext, xModel );
    return uno::Any( uno::Reference< excel::XWorkbook > (pWb) );
}

class WorkBookEnumImpl : public EnumerationHelperImpl
{
    uno::Any m_aApplication;
public:
    WorkBookEnumImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, const uno::Any& aApplication ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_aApplication( aApplication ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< sheet::XSpreadsheetDocument > xDoc( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return getWorkbook( m_xContext, xDoc, m_xParent );
    }

};

ScVbaWorkbooks::ScVbaWorkbooks( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaWorkbooks_BASE( xParent, xContext, VbaDocumentsBase::EXCEL_DOCUMENT )
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
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new WorkBookEnumImpl( mxParent, mxContext, xEnumerationAccess->createEnumeration(), Application() );
}

uno::Any
ScVbaWorkbooks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( aSource, uno::UNO_QUERY_THROW );
    return getWorkbook( mxContext, xDoc, mxParent );
}


uno::Any SAL_CALL
ScVbaWorkbooks::Add( const uno::Any& Template ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSpreadsheetDocument > xSpreadDoc;
    sal_Int32 nWorkbookType = 0;
    ::rtl::OUString aTemplateFileName;
    if( Template >>= nWorkbookType )
    {
        // nWorkbookType is a constant from XlWBATemplate (added in Excel 2007)
        // TODO: create chart-sheet if supported by Calc

        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
        // create a document with one sheet only
        uno::Reference< sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), uno::UNO_SET_THROW );
        uno::Reference< container::XIndexAccess > xSheetsIA( xSheets, uno::UNO_QUERY_THROW );
        while( xSheetsIA->getCount() > 1 )
        {
            uno::Reference< container::XNamed > xSheetName( xSheetsIA->getByIndex( xSheetsIA->getCount() - 1 ), uno::UNO_QUERY_THROW );
            xSheets->removeByName( xSheetName->getName() );
        }
    }
    else if( Template >>= aTemplateFileName )
    {
        // TODO: create document from template
        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
    }
    else if( !Template.hasValue() )
    {
        // regular spreadsheet document with configured number of sheets
        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
    }
    else
    {
        // illegal argument
        throw uno::RuntimeException();
    }

    // need to set up the document modules ( and vba mode ) here
    excel::setUpDocumentModules( xSpreadDoc );
    if( xSpreadDoc.is() )
        return getWorkbook( mxContext, xSpreadDoc, mxParent );
    return uno::Any();
}

void SAL_CALL
ScVbaWorkbooks::Close() throw (uno::RuntimeException)
{
    closeDocuments();
}

bool
ScVbaWorkbooks::isTextFile( const rtl::OUString& sType )
{
    // will return true if the file is
    // a) a variant of a text file
    // b) a csv file
    // c) unknown
    // returning true basically means treat this like a csv file
    const static rtl::OUString txtType("generic_Text");
    return sType.equals( txtType ) || sType.isEmpty();
}

bool
ScVbaWorkbooks::isSpreadSheetFile( const rtl::OUString& sType )
{
    // include calc_QPro etc. ? ( not for the moment anyway )
    if ( sType.indexOf( "calc_MS" ) == 0
    || sType.indexOf( "calc8" ) == 0
    || sType.indexOf( "calc_StarOffice" ) == 0 )
        return true;
    return false;
}

rtl::OUString
ScVbaWorkbooks::getFileFilterType( const rtl::OUString& rFileName )
{
    uno::Reference< document::XTypeDetection > xTypeDetect( mxContext->getServiceManager()->createInstanceWithContext(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.TypeDetection")), mxContext), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aMediaDesc(1);
    aMediaDesc[ 0 ].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ("URL" ) );
    aMediaDesc[ 0 ].Value <<= rFileName;
    rtl::OUString sType = xTypeDetect->queryTypeByDescriptor( aMediaDesc, sal_True );
    return sType;
}

// #TODO# #FIXME# can any of the unused params below be used?
uno::Any SAL_CALL
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

    uno::Sequence< beans::PropertyValue > sProps(0);

    rtl::OUString sType = getFileFilterType( aURL );
    // A text file means it needs to be processed as a csv file
    if ( isTextFile( sType ) )
    {
        sal_Int32 nIndex = 0;
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

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( openDocument( rFileName, ReadOnly, sProps ), uno::UNO_QUERY_THROW );
    uno::Any aRet = getWorkbook( mxContext, xSpreadDoc, mxParent );
    uno::Reference< excel::XWorkbook > xWBook( aRet, uno::UNO_QUERY );
    if ( xWBook.is() )
        xWBook->Activate();
    return aRet;
}

rtl::OUString
ScVbaWorkbooks::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaWorkbooks"));
}

css::uno::Sequence<rtl::OUString>
ScVbaWorkbooks::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Workbooks") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
