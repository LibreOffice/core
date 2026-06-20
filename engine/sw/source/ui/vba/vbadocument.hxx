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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX

#include <ooo/vba/XSink.hpp>
#include <ooo/vba/XSinkCaller.hpp>
#include <ooo/vba/word/XDocument.hpp>
#include <vbahelper/vbadocumentbase.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <sfx2/sfxbasemodel.hxx>

#include <vector>

class SwXTextDocument;

typedef cppu::ImplInheritanceHelper< VbaDocumentBase, ooo::vba::word::XDocument, ooo::vba::XSinkCaller > SwVbaDocument_BASE;

class SwVbaDocument : public SwVbaDocument_BASE
{
private:
    rtl::Reference< SwXTextDocument > mxTextDocument;

    std::vector<css::uno::Reference< ooo::vba::XSink >> mvSinks;

    void Initialize();
    cpo::uno::Any getControlShape( std::u16string_view sName );
    css::uno::Reference< css::container::XNameAccess > getFormControls() const;

protected:
    // this should be SwXTextDocument, but the inheritance hierarchy makes that impossible
    virtual SfxBaseModel* getModel() const override;

public:
    SwVbaDocument( const css::uno::Reference< ooo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& m_xContext, rtl::Reference< SwXTextDocument > const & xModel );
    SwVbaDocument(  css::uno::Sequence< cpo::uno::Any > const& aArgs, css::uno::Reference< css::uno::XComponentContext >const& xContext );
    virtual ~SwVbaDocument() override;

    sal_uInt32 AddSink( const css::uno::Reference< ooo::vba::XSink >& xSink );
    void RemoveSink( sal_uInt32 nNumber );

    // XDocument
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL getContent() override;
    virtual css::uno::Reference< ooo::vba::word::XRange > SAL_CALL Range( const cpo::uno::Any& rStart, const cpo::uno::Any& rEnd ) override;
    virtual cpo::uno::Any SAL_CALL BuiltInDocumentProperties( const cpo::uno::Any& index ) override;
    virtual cpo::uno::Any SAL_CALL CustomDocumentProperties( const cpo::uno::Any& index ) override;
    virtual cpo::uno::Any SAL_CALL Bookmarks( const cpo::uno::Any& rIndex ) override;
    cpo::uno::Any SAL_CALL ContentControls(const cpo::uno::Any& index) override;
    cpo::uno::Any SAL_CALL SelectContentControlsByTag(const cpo::uno::Any& index) override;
    cpo::uno::Any SAL_CALL SelectContentControlsByTitle(const cpo::uno::Any& index) override;
    css::uno::Reference<ov::word::XWindow> SAL_CALL getActiveWindow() override;
    virtual cpo::uno::Any SAL_CALL Variables( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any SAL_CALL getAttachedTemplate() override;
    virtual void SAL_CALL setAttachedTemplate( const cpo::uno::Any& _attachedtemplate ) override;
    virtual cpo::uno::Any SAL_CALL Paragraphs( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any SAL_CALL Styles( const cpo::uno::Any& rIndex ) override;
    virtual cpo::uno::Any SAL_CALL Tables( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Fields( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Shapes( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL Select() override;
    virtual cpo::uno::Any SAL_CALL Sections( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL Activate() override;
    virtual cpo::uno::Any SAL_CALL PageSetup() override;
    virtual cpo::uno::Any SAL_CALL TablesOfContents( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL FormFields( const cpo::uno::Any& aIndex ) override;
    virtual ::sal_Int32 SAL_CALL getProtectionType() override;
    virtual void SAL_CALL setProtectionType( ::sal_Int32 _protectiontype ) override;
    virtual bool SAL_CALL getUpdateStylesOnOpen() override;
    virtual void SAL_CALL setUpdateStylesOnOpen( bool _updatestylesonopen ) override;
    virtual bool SAL_CALL getAutoHyphenation() override;
    virtual void SAL_CALL setAutoHyphenation( bool _autohyphenation ) override;
    virtual ::sal_Int32 SAL_CALL getHyphenationZone() override;
    virtual void SAL_CALL setHyphenationZone( ::sal_Int32 _hyphenationzone ) override;
    virtual ::sal_Int32 SAL_CALL getConsecutiveHyphensLimit() override;
    virtual void SAL_CALL setConsecutiveHyphensLimit( ::sal_Int32 _consecutivehyphenslimit ) override;
    virtual css::uno::Reference< ooo::vba::word::XMailMerge > SAL_CALL getMailMerge() override;

    using VbaDocumentBase::Protect;
    virtual void SAL_CALL Protect( ::sal_Int32 Type, const cpo::uno::Any& NOReset, const cpo::uno::Any& Password, const cpo::uno::Any& UseIRM, const cpo::uno::Any& EnforceStyleLock ) override;
    virtual void SAL_CALL PrintOut( const cpo::uno::Any& Background, const cpo::uno::Any& Append, const cpo::uno::Any& Range, const cpo::uno::Any& OutputFileName, const cpo::uno::Any& From, const cpo::uno::Any& To, const cpo::uno::Any& Item, const cpo::uno::Any& Copies, const cpo::uno::Any& Pages, const cpo::uno::Any& PageType, const cpo::uno::Any& PrintToFile, const cpo::uno::Any& Collate, const cpo::uno::Any& FileName, const cpo::uno::Any& ActivePrinterMacGX, const cpo::uno::Any& ManualDuplexPrint, const cpo::uno::Any& PrintZoomColumn, const cpo::uno::Any& PrintZoomRow, const cpo::uno::Any& PrintZoomPaperWidth, const cpo::uno::Any& PrintZoomPaperHeight ) override;
    virtual void SAL_CALL PrintPreview(  ) override;
    virtual void SAL_CALL ClosePrintPreview(  ) override;
    virtual cpo::uno::Any SAL_CALL Revisions( const cpo::uno::Any& aIndex ) override;
    virtual cpo::uno::Any SAL_CALL Frames( const cpo::uno::Any& aIndex ) override;
    virtual void SAL_CALL SaveAs2000( const cpo::uno::Any& FileName, const cpo::uno::Any& FileFormat, const cpo::uno::Any& LockComments, const cpo::uno::Any& Password, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& WritePassword, const cpo::uno::Any& ReadOnlyRecommended, const cpo::uno::Any& EmbedTrueTypeFonts, const cpo::uno::Any& SaveNativePictureFormat, const cpo::uno::Any& SaveFormsData, const cpo::uno::Any& SaveAsAOCELetter ) override;
    virtual void SAL_CALL SaveAs( const cpo::uno::Any& FileName, const cpo::uno::Any& FileFormat, const cpo::uno::Any& LockComments, const cpo::uno::Any& Password, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& WritePassword, const cpo::uno::Any& ReadOnlyRecommended, const cpo::uno::Any& EmbedTrueTypeFonts, const cpo::uno::Any& SaveNativePictureFormat, const cpo::uno::Any& SaveFormsData, const cpo::uno::Any& SaveAsAOCELetter, const cpo::uno::Any& Encoding, const cpo::uno::Any& InsertLineBreaks, const cpo::uno::Any& AllowSubstitutions, const cpo::uno::Any& LineEnding, const cpo::uno::Any& AddBiDiMarks ) override;
    virtual void SAL_CALL Close( const cpo::uno::Any& SaveChanges, const cpo::uno::Any& OriginalFormat, const cpo::uno::Any& RouteDocument ) override;
    virtual void SAL_CALL SavePreviewPngAs( const cpo::uno::Any& FileName ) override;

    // XInvocation
    virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;
    virtual cpo::uno::Any SAL_CALL invoke( const OUString& aFunctionName, const css::uno::Sequence< cpo::uno::Any >& aParams, css::uno::Sequence< ::sal_Int16 >& aOutParamIndex, css::uno::Sequence< cpo::uno::Any >& aOutParam ) override;
    virtual void SAL_CALL setValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
    virtual bool SAL_CALL hasMethod( const OUString& aName ) override;
    virtual bool SAL_CALL hasProperty( const OUString& aName ) override;

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
    virtual void SAL_CALL CallSinks( const OUString& Method, css::uno::Sequence< cpo::uno::Any >& Arguments ) override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
