/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

    virtual void SAL_CALL FileOpen(const OUString& Name, const cpo::uno::Any& ConfirmConversions,
                                   const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToMru,
                                   const cpo::uno::Any& PasswordDoc,
                                   const cpo::uno::Any& PasswordDot, const cpo::uno::Any& Revert,
                                   const cpo::uno::Any& WritePasswordDoc,
                                   const cpo::uno::Any& WritePasswordDot) override;
    virtual void SAL_CALL FileSave() override;
    virtual void SAL_CALL FileSaveAs(
        const cpo::uno::Any& Name, const cpo::uno::Any& Format, const cpo::uno::Any& LockAnnot,
        const cpo::uno::Any& Password, const cpo::uno::Any& AddToMru,
        const cpo::uno::Any& WritePassword, const cpo::uno::Any& RecommendReadOnly,
        const cpo::uno::Any& EmbedFonts, const cpo::uno::Any& NativePictureFormat,
        const cpo::uno::Any& FormsData, const cpo::uno::Any& SaveAsAOCELetter) override;
    virtual void SAL_CALL FileClose(const cpo::uno::Any& Save) override;
    virtual void SAL_CALL ToolsOptionsView(
        const cpo::uno::Any& DraftFont, const cpo::uno::Any& WrapToWindow,
        const cpo::uno::Any& PicturePlaceHolders, const cpo::uno::Any& FieldCodes,
        const cpo::uno::Any& BookMarks, const cpo::uno::Any& FieldShading,
        const cpo::uno::Any& StatusBar, const cpo::uno::Any& HScroll, const cpo::uno::Any& VScroll,
        const cpo::uno::Any& StyleAreaWidth, const cpo::uno::Any& Tabs, const cpo::uno::Any& Spaces,
        const cpo::uno::Any& Paras, const cpo::uno::Any& Hyphens, const cpo::uno::Any& Hidden,
        const cpo::uno::Any& ShowAll, const cpo::uno::Any& Drawings, const cpo::uno::Any& Anchors,
        const cpo::uno::Any& TextBoundaries, const cpo::uno::Any& VRuler,
        const cpo::uno::Any& Highlight) override;
    virtual cpo::uno::Any SAL_CALL WindowName(const cpo::uno::Any& Number) override;
    virtual cpo::uno::Any SAL_CALL ExistingBookmark(const OUString& Name) override;
    virtual void SAL_CALL MailMergeOpenDataSource(
        const OUString& Name, const cpo::uno::Any& Format, const cpo::uno::Any& ConfirmConversions,
        const cpo::uno::Any& ReadOnly, const cpo::uno::Any& LinkToSource,
        const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument,
        const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert,
        const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate,
        const cpo::uno::Any& Connection, const cpo::uno::Any& SQLStatement,
        const cpo::uno::Any& SQLStatement1, const cpo::uno::Any& OpenExclusive,
        const cpo::uno::Any& SubType) override;
    virtual cpo::uno::Any SAL_CALL AppMaximize(const cpo::uno::Any& WindowName,
                                               const cpo::uno::Any& State) override;
    virtual cpo::uno::Any SAL_CALL DocMaximize(const cpo::uno::Any& State) override;
    virtual void SAL_CALL AppShow(const cpo::uno::Any& WindowName) override;
    virtual cpo::uno::Any SAL_CALL AppCount() override;
    virtual void SAL_CALL MsgBox(const OUString& sPrompt) override;
    virtual void SAL_CALL ScreenUpdating(const cpo::uno::Any& On) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAWORDBASIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
