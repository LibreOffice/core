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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX

#include <ooo/vba/XSink.hpp>
#include <ooo/vba/XSinkCaller.hpp>
#include <ooo/vba/word/XDocument.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadocumentbase.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

typedef cppu::ImplInheritanceHelper< VbaDocumentBase, ooo::vba::word::XDocument, ooo::vba::XSinkCaller > SwVbaDocument_BASE;

class SwVbaDocument : public SwVbaDocument_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;

    std::vector<css::uno::Reference< ooo::vba::XSink >> mvSinks;

    void Initialize();
    css::uno::Any getControlShape( const OUString& sName );
    css::uno::Reference< css::container::XNameAccess > getFormControls();

public:
    SwVbaDocument( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& m_xContext, css::uno::Reference< css::frame::XModel > const & xModel );
    SwVbaDocument(  css::uno::Sequence< css::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~SwVbaDocument() override;

    sal_uInt32 AddSink( const css::uno::Reference< ooo::vba::XSink >& xSink );
    void RemoveSink( sal_uInt32 nNumber );

    // XDocument
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getContent() override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Range( const css::uno::Any& rStart, const css::uno::Any& rEnd ) override;
    virtual css::uno::Any SAL_CALL BuiltInDocumentProperties( const css::uno::Any& index ) override;
    virtual css::uno::Any SAL_CALL CustomDocumentProperties( const css::uno::Any& index ) override;
    virtual css::uno::Any SAL_CALL Bookmarks( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL Variables( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL getAttachedTemplate() override;
    virtual void SAL_CALL setAttachedTemplate( const css::uno::Any& _attachedtemplate ) override;
    virtual css::uno::Any SAL_CALL Paragraphs( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL Styles( const css::uno::Any& rIndex ) override;
    virtual css::uno::Any SAL_CALL Tables( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Fields( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Shapes( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Sections( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL Activate() override;
    virtual css::uno::Any SAL_CALL PageSetup() override;
    virtual css::uno::Any SAL_CALL TablesOfContents( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL FormFields( const css::uno::Any& aIndex ) override;
    virtual ::sal_Int32 SAL_CALL getProtectionType() override;
    virtual void SAL_CALL setProtectionType( ::sal_Int32 _protectiontype ) override;
    virtual sal_Bool SAL_CALL getUpdateStylesOnOpen() override;
    virtual void SAL_CALL setUpdateStylesOnOpen( sal_Bool _updatestylesonopen ) override;
    virtual sal_Bool SAL_CALL getAutoHyphenation() override;
    virtual void SAL_CALL setAutoHyphenation( sal_Bool _autohyphenation ) override;
    virtual ::sal_Int32 SAL_CALL getHyphenationZone() override;
    virtual void SAL_CALL setHyphenationZone( ::sal_Int32 _hyphenationzone ) override;
    virtual ::sal_Int32 SAL_CALL getConsecutiveHyphensLimit() override;
    virtual void SAL_CALL setConsecutiveHyphensLimit( ::sal_Int32 _consecutivehyphenslimit ) override;
    virtual css::uno::Reference< ooo::vba::word::XMailMerge > SAL_CALL getMailMerge() override;

    using VbaDocumentBase::Protect;
    virtual void SAL_CALL Protect( ::sal_Int32 Type, const css::uno::Any& NOReset, const css::uno::Any& Password, const css::uno::Any& UseIRM, const css::uno::Any& EnforceStyleLock ) override;
    virtual void SAL_CALL PrintOut( const css::uno::Any& Background, const css::uno::Any& Append, const css::uno::Any& Range, const css::uno::Any& OutputFileName, const css::uno::Any& From, const css::uno::Any& To, const css::uno::Any& Item, const css::uno::Any& Copies, const css::uno::Any& Pages, const css::uno::Any& PageType, const css::uno::Any& PrintToFile, const css::uno::Any& Collate, const css::uno::Any& FileName, const css::uno::Any& ActivePrinterMacGX, const css::uno::Any& ManualDuplexPrint, const css::uno::Any& PrintZoomColumn, const css::uno::Any& PrintZoomRow, const css::uno::Any& PrintZoomPaperWidth, const css::uno::Any& PrintZoomPaperHeight ) override;
    virtual void SAL_CALL PrintPreview(  ) override;
    virtual void SAL_CALL ClosePrintPreview(  ) override;
    virtual css::uno::Any SAL_CALL Revisions( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL Frames( const css::uno::Any& aIndex ) override;
    virtual void SAL_CALL SaveAs2000( const css::uno::Any& FileName, const css::uno::Any& FileFormat, const css::uno::Any& LockComments, const css::uno::Any& Password, const css::uno::Any& AddToRecentFiles, const css::uno::Any& WritePassword, const css::uno::Any& ReadOnlyRecommended, const css::uno::Any& EmbedTrueTypeFonts, const css::uno::Any& SaveNativePictureFormat, const css::uno::Any& SaveFormsData, const css::uno::Any& SaveAsAOCELetter ) override;
    virtual void SAL_CALL SaveAs( const css::uno::Any& FileName, const css::uno::Any& FileFormat, const css::uno::Any& LockComments, const css::uno::Any& Password, const css::uno::Any& AddToRecentFiles, const css::uno::Any& WritePassword, const css::uno::Any& ReadOnlyRecommended, const css::uno::Any& EmbedTrueTypeFonts, const css::uno::Any& SaveNativePictureFormat, const css::uno::Any& SaveFormsData, const css::uno::Any& SaveAsAOCELetter, const css::uno::Any& Encoding, const css::uno::Any& InsertLineBreaks, const css::uno::Any& AllowSubstitutions, const css::uno::Any& LineEnding, const css::uno::Any& AddBiDiMarks ) override;
    virtual void SAL_CALL SavePreviewPngAs( const css::uno::Any& FileName ) override;

    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;
    virtual css::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< css::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< css::uno::Any >& aOutParam ) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;

    // XInterfaceWithIID
    virtual OUString SAL_CALL getIID() override;

    // XConnectable
    virtual OUString SAL_CALL GetIIDForClassItselfNotCoclass() override;
    virtual ov::TypeAndIID SAL_CALL GetConnectionPoint() override;
    virtual css::uno::Reference<ov::XConnectionPoint> SAL_CALL FindConnectionPoint() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XSinkCaller
    virtual void SAL_CALL CallSinks( const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments ) override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
