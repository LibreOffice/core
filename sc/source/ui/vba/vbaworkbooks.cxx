/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <comphelper/processfactory.hxx>

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

    uno::Reference< excel::XWorkbook > xWb( getVBADocument( xModel ), uno::UNO_QUERY );
    if ( xWb.is() )
    {
        OSL_TRACE(" *** Returning Module uno Object *** ");
        return uno::Any( xWb );
    }

    ScVbaWorkbook *pWb = new ScVbaWorkbook( xParent, xContext, xModel );
    return uno::Any( uno::Reference< excel::XWorkbook > (pWb) );
}

class WorkBookEnumImpl : public EnumerationHelperImpl
{
public:
    WorkBookEnumImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
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
    return cppu::UnoType<excel::XWorkbook>::get();
}
uno::Reference< container::XEnumeration >
ScVbaWorkbooks::createEnumeration() throw (uno::RuntimeException)
{
    // #FIXME its possible the WorkBookEnumImpl here doesn't reflect
    // the state of this object ( although it should ) would be
    // safer to create an enumeration based on this objects state
    // rather than one effectively based of the desktop component
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new WorkBookEnumImpl( mxParent, mxContext, xEnumerationAccess->createEnumeration() );
}

uno::Any
ScVbaWorkbooks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( aSource, uno::UNO_QUERY_THROW );
    return getWorkbook( mxContext, xDoc, mxParent );
}

uno::Any SAL_CALL
ScVbaWorkbooks::Add( const uno::Any& Template ) throw (uno::RuntimeException, std::exception)
{
    uno::Reference< sheet::XSpreadsheetDocument > xSpreadDoc;
    sal_Int32 nWorkbookType = 0;
    OUString aTemplateFileName;
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
ScVbaWorkbooks::Close() throw (uno::RuntimeException, std::exception)
{
}

bool
ScVbaWorkbooks::isTextFile( const OUString& sType )
{
    // will return true if the file is
    // a) a variant of a text file
    // b) a csv file
    // c) unknown
    // returning true basically means treat this like a csv file
    return sType == "generic_Text" || sType.isEmpty();
}

bool
ScVbaWorkbooks::isSpreadSheetFile( const OUString& sType )
{
    // include calc_QPro etc. ? ( not for the moment anyway )
    if ( sType.startsWith( "calc_MS" )
      || sType.startsWith( "calc8" )
      || sType.startsWith( "calc_StarOffice" ) )
        return true;
    return false;
}

OUString
ScVbaWorkbooks::getFileFilterType( const OUString& rFileName )
{
    uno::Reference< document::XTypeDetection > xTypeDetect( mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", mxContext), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aMediaDesc(1);
    aMediaDesc[ 0 ].Name = "URL";
    aMediaDesc[ 0 ].Value <<= rFileName;
    OUString sType = xTypeDetect->queryTypeByDescriptor( aMediaDesc, true );
    return sType;
}

// #TODO# #FIXME# can any of the unused params below be used?
uno::Any SAL_CALL
ScVbaWorkbooks::Open( const OUString& rFileName, const uno::Any& /*UpdateLinks*/, const uno::Any& ReadOnly, const uno::Any& Format, const uno::Any& /*Password*/, const uno::Any& /*WriteResPassword*/, const uno::Any& /*IgnoreReadOnlyRecommended*/, const uno::Any& /*Origin*/, const uno::Any& Delimiter, const uno::Any& /*Editable*/, const uno::Any& /*Notify*/, const uno::Any& /*Converter*/, const uno::Any& /*AddToMru*/ ) throw (uno::RuntimeException, std::exception)
{
    // we need to detect if this is a URL, if not then assume it's a file path
    OUString aURL;
    INetURLObject aObj;
    aObj.SetURL( rFileName );
    bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
    if ( bIsURL )
        aURL = rFileName;
    else
        osl::FileBase::getFileURLFromSystemPath( rFileName, aURL );

    uno::Sequence< beans::PropertyValue > sProps(0);

    OUString sType = getFileFilterType( aURL );
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
        sProps[ nIndex ].Name = "FilterOptions";
        sal_Int16 delims[] = { 0 /*default not used*/, 9/*tab*/, 44/*comma*/, 32/*space*/, 59/*semicolon*/ };
        static const char sRestOfFormat[] = ",34,0,1";

        OUString sFormat;
        sal_Int16 nFormat = 0; // default indicator

        if ( Format.hasValue() )
        {
            Format >>= nFormat; // val of nFormat overwritten if extracted
            // validate param
            if ( nFormat < 1 || nFormat > 6 )
                throw uno::RuntimeException("Illegal value for Format" );
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
                throw uno::RuntimeException("Expected value for Delimiter" );
            OUString sStr;
            Delimiter >>= sStr;
            if ( !sStr.isEmpty() )
                nDelim = sStr[0];
            else
                throw uno::RuntimeException("Incorrect value for Delimiter" );
        }

        getCurrentDelim() = nDelim; //set new current

        sFormat = OUString::number( nDelim ) + sRestOfFormat;
        sProps[ nIndex++ ].Value <<= sFormat;
        sProps[ nIndex ].Name = "FilterName";
        sProps[ nIndex++ ].Value <<= OUString( "Text - txt - csv (StarCalc)" );
        // Ensure WORKAROUND_CSV_TXT_BUG_i60158 gets called in typedetection.cxx so
        // csv is forced for deep detected 'writerxxx' types
        sProps[ nIndex ].Name = "DocumentService";
        sProps[ nIndex ].Value <<= OUString("com.sun.star.sheet.SpreadsheetDocument");
    }
    else if ( !isSpreadSheetFile( sType ) )
        throw uno::RuntimeException("Bad Format" );

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( openDocument( rFileName, ReadOnly, sProps ), uno::UNO_QUERY_THROW );
    uno::Any aRet = getWorkbook( mxContext, xSpreadDoc, mxParent );
    uno::Reference< excel::XWorkbook > xWBook( aRet, uno::UNO_QUERY );
    if ( xWBook.is() )
        xWBook->Activate();
    return aRet;
}

OUString
ScVbaWorkbooks::getServiceImplName()
{
    return OUString("ScVbaWorkbooks");
}

css::uno::Sequence<OUString>
ScVbaWorkbooks::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.excel.Workbooks";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
