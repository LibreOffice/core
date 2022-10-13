/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAWORDBASIC_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAWORDBASIC_HXX

class SwVbaApplication;

/**
 * This is a representation of the WordBasic statements and functions
 * that were available in Word version 6.0 and Word for Windows 95.
 *
 * It can be specified as "Application.WordBasic." or "WordBasic.".
 *
 * Starting with Word 2000, old macros were automatically converted
 * into Visual Basic modules, and the WordBasic prefix was added where
 * no VBA methods precisely corresponded.
 *
 * In other words, it is a compatibility shim.
 */
class SwWordBasic : public cppu::WeakImplHelper<ooo::vba::word::XWordBasic>
{
private:
    SwVbaApplication* mpApp;

public:
    SwWordBasic(SwVbaApplication* pApp);

    // XWordBasic
    virtual sal_Int32 SAL_CALL getMailMergeMainDocumentType() override;
    virtual void SAL_CALL
    setMailMergeMainDocumentType(sal_Int32 _mailmergemaindocumenttype) override;

    virtual void SAL_CALL FileOpen(const OUString& Name, const css::uno::Any& ConfirmConversions,
                                   const css::uno::Any& ReadOnly, const css::uno::Any& AddToMru,
                                   const css::uno::Any& PasswordDoc,
                                   const css::uno::Any& PasswordDot, const css::uno::Any& Revert,
                                   const css::uno::Any& WritePasswordDoc,
                                   const css::uno::Any& WritePasswordDot) override;
    virtual void SAL_CALL FileSave() override;
    virtual void SAL_CALL FileSaveAs(
        const css::uno::Any& Name, const css::uno::Any& Format, const css::uno::Any& LockAnnot,
        const css::uno::Any& Password, const css::uno::Any& AddToMru,
        const css::uno::Any& WritePassword, const css::uno::Any& RecommendReadOnly,
        const css::uno::Any& EmbedFonts, const css::uno::Any& NativePictureFormat,
        const css::uno::Any& FormsData, const css::uno::Any& SaveAsAOCELetter) override;
    virtual void SAL_CALL FileClose(const css::uno::Any& Save) override;
    virtual void SAL_CALL ToolsOptionsView(
        const css::uno::Any& DraftFont, const css::uno::Any& WrapToWindow,
        const css::uno::Any& PicturePlaceHolders, const css::uno::Any& FieldCodes,
        const css::uno::Any& BookMarks, const css::uno::Any& FieldShading,
        const css::uno::Any& StatusBar, const css::uno::Any& HScroll, const css::uno::Any& VScroll,
        const css::uno::Any& StyleAreaWidth, const css::uno::Any& Tabs, const css::uno::Any& Spaces,
        const css::uno::Any& Paras, const css::uno::Any& Hyphens, const css::uno::Any& Hidden,
        const css::uno::Any& ShowAll, const css::uno::Any& Drawings, const css::uno::Any& Anchors,
        const css::uno::Any& TextBoundaries, const css::uno::Any& VRuler,
        const css::uno::Any& Highlight) override;
    virtual css::uno::Any SAL_CALL WindowName(const css::uno::Any& Number) override;
    virtual css::uno::Any SAL_CALL ExistingBookmark(const OUString& Name) override;
    virtual void SAL_CALL MailMergeOpenDataSource(
        const OUString& Name, const css::uno::Any& Format, const css::uno::Any& ConfirmConversions,
        const css::uno::Any& ReadOnly, const css::uno::Any& LinkToSource,
        const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument,
        const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert,
        const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate,
        const css::uno::Any& Connection, const css::uno::Any& SQLStatement,
        const css::uno::Any& SQLStatement1, const css::uno::Any& OpenExclusive,
        const css::uno::Any& SubType) override;
    virtual css::uno::Any SAL_CALL AppMaximize(const css::uno::Any& WindowName,
                                               const css::uno::Any& State) override;
    virtual css::uno::Any SAL_CALL DocMaximize(const css::uno::Any& State) override;
    virtual void SAL_CALL AppShow(const css::uno::Any& WindowName) override;
    virtual css::uno::Any SAL_CALL AppCount() override;
    virtual void SAL_CALL MsgBox(const OUString& sPrompt) override;
    virtual void SAL_CALL ScreenUpdating(const css::uno::Any& On) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAWORDBASIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
