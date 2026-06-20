/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <tools/urlobj.hxx>
#include <rtl/ref.hxx>

#include "vbadocument.hxx"
#include "vbadocuments.hxx"

#include <osl/file.hxx>
#include <utility>
#include <unotxdoc.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static cpo::uno::Any
getDocument( uno::Reference< uno::XComponentContext > const & xContext, const rtl::Reference< SwXTextDocument > &xDoc, const cpo::uno::Any& aApplication )
{
    // FIXME: fine as long as SwVbaDocument is stateless ...
    if( !xDoc.is() )
        return cpo::uno::Any();

    rtl::Reference<SwVbaDocument> pWb = new SwVbaDocument(  uno::Reference< XHelperInterface >( aApplication, uno::UNO_QUERY_THROW ), xContext, xDoc );
    return cpo::uno::Any( uno::Reference< word::XDocument > (pWb) );
}

namespace {

class DocumentEnumImpl : public EnumerationHelperImpl
{
    cpo::uno::Any m_aApplication;
public:
    /// @throws uno::RuntimeException
    DocumentEnumImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, cpo::uno::Any  aApplication ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_aApplication(std::move( aApplication )) {}

    virtual cpo::uno::Any SAL_CALL nextElement(  ) override
    {
        uno::Reference< text::XTextDocument > xDoc( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return getDocument( m_xContext, dynamic_cast<SwXTextDocument*>(xDoc.get()), m_aApplication );
    }
};

}

SwVbaDocuments::SwVbaDocuments( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext ) : SwVbaDocuments_BASE( xParent, xContext, VbaDocumentsBase::WORD_DOCUMENT )
{
}
// XEnumerationAccess
uno::Type
SwVbaDocuments::getElementType()
{
    return cppu::UnoType<word::XDocument>::get();
}
uno::Reference< container::XEnumeration >
SwVbaDocuments::createEnumeration()
{
    // #FIXME it's possible the DocumentEnumImpl here doesn't reflect
    // the state of this object (although it should) would be
    // safer to create an enumeration based on this objects state
    // rather than one effectively based of the desktop component
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new DocumentEnumImpl( mxParent, mxContext, xEnumerationAccess->createEnumeration(), Application() );
}

cpo::uno::Any
SwVbaDocuments::createCollectionObject( const cpo::uno::Any& aSource )
{
    uno::Reference< text::XTextDocument > xDoc( aSource, uno::UNO_QUERY_THROW );
    return getDocument( mxContext, dynamic_cast<SwXTextDocument*>(xDoc.get()), Application() );
}

cpo::uno::Any SAL_CALL
SwVbaDocuments::Add( const cpo::uno::Any& Template, const cpo::uno::Any& /*NewTemplate*/, const cpo::uno::Any& /*DocumentType*/, const cpo::uno::Any& /*Visible*/ )
{
    OUString sFileName;
    if( Template.hasValue() && ( Template >>= sFileName ) )
    {
        return  Open( sFileName, cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any());
    }
    uno::Reference <text::XTextDocument> xTextDoc( createDocument() , uno::UNO_QUERY_THROW );
    return getDocument( mxContext, dynamic_cast<SwXTextDocument*>(xTextDoc.get()), Application() );
}

// #TODO# #FIXME# can any of the unused params below be used?
// #TODO# #FIXME# surely we should actually close the document here
void SAL_CALL
SwVbaDocuments::Close( const cpo::uno::Any& /*SaveChanges*/, const cpo::uno::Any& /*OriginalFormat*/, const cpo::uno::Any& /*RouteDocument*/ )
{
}

// #TODO# #FIXME# can any of the unused params below be used?
cpo::uno::Any SAL_CALL
SwVbaDocuments::Open( const OUString& Filename, const cpo::uno::Any& /*ConfirmConversions*/, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& /*AddToRecentFiles*/, const cpo::uno::Any& /*PasswordDocument*/, const cpo::uno::Any& /*PasswordTemplate*/, const cpo::uno::Any& /*Revert*/, const cpo::uno::Any& /*WritePasswordDocument*/, const cpo::uno::Any& /*WritePasswordTemplate*/, const cpo::uno::Any& /*Format*/, const cpo::uno::Any& /*Encoding*/, const cpo::uno::Any& /*Visible*/, const cpo::uno::Any& /*OpenAndRepair*/, const cpo::uno::Any& /*DocumentDirection*/, const cpo::uno::Any& /*NoEncodingDialog*/, const cpo::uno::Any& /*XMLTransform*/ )
{
    SAL_INFO("sw.vba", "Documents.Open(Filename:=" << Filename << ",ReadOnly:=" << ReadOnly << ")");

    // we need to detect if this is a URL, if not then assume it's a file path
    OUString aURL;
    INetURLObject aObj;
    aObj.SetURL( Filename );
    bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
    if ( bIsURL )
        aURL = Filename;
    else
        osl::FileBase::getFileURLFromSystemPath( Filename, aURL );

    uno::Reference <text::XTextDocument> xSpreadDoc( openDocument( Filename, ReadOnly, {}), uno::UNO_QUERY_THROW );
    cpo::uno::Any aRet = getDocument( mxContext, dynamic_cast<SwXTextDocument*>(xSpreadDoc.get()), Application() );
    uno::Reference< word::XDocument > xDocument( aRet, uno::UNO_QUERY );
    if ( xDocument.is() )
        xDocument->Activate();
    return aRet;
}

cpo::uno::Any SAL_CALL
SwVbaDocuments::OpenNoRepairDialog( const OUString& Filename, const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Format, const cpo::uno::Any& Encoding, const cpo::uno::Any& Visible, const cpo::uno::Any& OpenAndRepair, const cpo::uno::Any& DocumentDirection, const cpo::uno::Any& NoEncodingDialog, const cpo::uno::Any& XMLTransform )
{
    return Open( Filename, ConfirmConversions, ReadOnly, AddToRecentFiles, PasswordDocument, PasswordTemplate, Revert, WritePasswordDocument, WritePasswordTemplate, Format, Encoding, Visible, OpenAndRepair, DocumentDirection, NoEncodingDialog, XMLTransform );
}

cpo::uno::Any SAL_CALL
SwVbaDocuments::OpenOld( const OUString& FileName, const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Format )
{
    return Open( FileName, ConfirmConversions, ReadOnly, AddToRecentFiles, PasswordDocument, PasswordTemplate, Revert, WritePasswordDocument, WritePasswordTemplate, Format, cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any(), cpo::uno::Any() );
}

OUString
SwVbaDocuments::getServiceImplName()
{
    return u"SwVbaDocuments"_ustr;
}

uno::Sequence<OUString>
SwVbaDocuments::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Documents"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
