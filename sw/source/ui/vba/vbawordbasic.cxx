/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
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

#include "vbaapplication.hxx"
#include "vbafilterpropsfromformat.hxx"
#include "vbamailmerge.hxx"
#include "vbawordbasic.hxx"

#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <ooo/vba/word/XBookmarks.hpp>
#include <ooo/vba/word/XDocuments.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwWordBasic::SwWordBasic(SwVbaApplication* pApp)
    : mpApp(pApp)
{
}

sal_Int32 SAL_CALL SwWordBasic::getMailMergeMainDocumentType()
{
    return SwVbaMailMerge::get(mpApp->getParent(), mpApp->getContext())->getMainDocumentType();
}

void SAL_CALL SwWordBasic::setMailMergeMainDocumentType(sal_Int32 _mailmergemaindocumenttype)
{
    SwVbaMailMerge::get(mpApp->getParent(), mpApp->getContext())
        ->setMainDocumentType(_mailmergemaindocumenttype);
}

void SAL_CALL SwWordBasic::FileOpen(const OUString& Name, const uno::Any& ConfirmConversions,
                                    const uno::Any& ReadOnly, const uno::Any& AddToMru,
                                    const uno::Any& PasswordDoc, const uno::Any& PasswordDot,
                                    const uno::Any& Revert, const uno::Any& WritePasswordDoc,
                                    const uno::Any& WritePasswordDot)
{
    uno::Any aDocuments = mpApp->Documents(uno::Any());

    uno::Reference<word::XDocuments> rDocuments;

    if (aDocuments >>= rDocuments)
        rDocuments->Open(Name, ConfirmConversions, ReadOnly, AddToMru, PasswordDoc, PasswordDot,
                         Revert, WritePasswordDoc, WritePasswordDot, uno::Any(), uno::Any(),
                         uno::Any(), uno::Any(), uno::Any(), uno::Any(), uno::Any());
}

void SAL_CALL SwWordBasic::FileSave()
{
    uno::Reference<frame::XModel> xModel(mpApp->getCurrentDocument(), uno::UNO_SET_THROW);
    dispatchRequests(xModel, u".uno:Save"_ustr);
}

void SAL_CALL SwWordBasic::FileSaveAs(
    const css::uno::Any& Name, const css::uno::Any& Format, const css::uno::Any& /*LockAnnot*/,
    const css::uno::Any& /*Password*/, const css::uno::Any& /*AddToMru*/,
    const css::uno::Any& /*WritePassword*/, const css::uno::Any& /*RecommendReadOnly*/,
    const css::uno::Any& /*EmbedFonts*/, const css::uno::Any& /*NativePictureFormat*/,
    const css::uno::Any& /*FormsData*/, const css::uno::Any& /*SaveAsAOCELetter*/)
{
    SAL_INFO("sw.vba", "WordBasic.FileSaveAs(Name:=" << Name << ",Format:=" << Format << ")");

    uno::Reference<frame::XModel> xModel(mpApp->getCurrentDocument(), uno::UNO_SET_THROW);

    // Based on SwVbaDocument::SaveAs2000.

    OUString sFileName;
    Name >>= sFileName;

    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath(sFileName, sURL);

    // Detect if there is no path then we need to use the current folder.
    INetURLObject aURL(sURL);
    sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
    if (sURL.isEmpty())
    {
        // Need to add cur dir ( of this document ) or else the 'Work' dir
        sURL = xModel->getURL();

        if (sURL.isEmpty())
        {
            // Not path available from 'this' document. Need to add the 'document'/work directory then.
            // Based on SwVbaOptions::getValueEvent()
            uno::Reference<util::XPathSettings> xPathSettings
                = util::thePathSettings::get(comphelper::getProcessComponentContext());
            OUString sPathUrl;
            xPathSettings->getPropertyValue(u"Work"_ustr) >>= sPathUrl;
            // Path could be a multipath, Microsoft doesn't support this feature in Word currently.
            // Only the last path is from interest.
            // No idea if this crack is relevant for WordBasic or not.
            sal_Int32 nIndex = sPathUrl.lastIndexOf(';');
            if (nIndex != -1)
            {
                sPathUrl = sPathUrl.copy(nIndex + 1);
            }

            aURL.SetURL(sPathUrl);
        }
        else
        {
            aURL.SetURL(sURL);
            aURL.Append(sFileName);
        }
        sURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
    }
    sal_Int32 nFileFormat = word::WdSaveFormat::wdFormatDocument;
    Format >>= nFileFormat;

    uno::Sequence aProps{ comphelper::makePropertyValue(u"FilterName"_ustr, css::uno::Any()),
                          comphelper::makePropertyValue(u"FileName"_ustr, sURL) };

    setFilterPropsFromFormat(nFileFormat, aProps);

    dispatchRequests(xModel, u".uno:SaveAs"_ustr, aProps);
}

void SAL_CALL SwWordBasic::FileClose(const css::uno::Any& Save)
{
    uno::Reference<frame::XModel> xModel(mpApp->getCurrentDocument(), uno::UNO_SET_THROW);

    sal_Int16 nSave = 0;
    if (Save.hasValue() && (Save >>= nSave) && (nSave == 0 || nSave == 1))
        FileSave();

    // FIXME: Here I would much prefer to call VbaDocumentBase::Close() but not sure how to get at
    // the VbaDocumentBase of the current document. (Probably it is easy and I haven't looked hard
    // enough.)
    //
    // FIXME: Error handling. If there is no current document, return some kind of error? But for
    // now, just ignore errors. This code is written to work for a very specific customer use case
    // anyway, not for an arbitrary sequence of COM calls to the "VBA" API.
    dispatchRequests(xModel, u".uno:CloseDoc"_ustr);
}

void SAL_CALL SwWordBasic::ToolsOptionsView(
    const css::uno::Any& DraftFont, const css::uno::Any& WrapToWindow,
    const css::uno::Any& PicturePlaceHolders, const css::uno::Any& FieldCodes,
    const css::uno::Any& BookMarks, const css::uno::Any& FieldShading,
    const css::uno::Any& StatusBar, const css::uno::Any& HScroll, const css::uno::Any& VScroll,
    const css::uno::Any& StyleAreaWidth, const css::uno::Any& Tabs, const css::uno::Any& Spaces,
    const css::uno::Any& Paras, const css::uno::Any& Hyphens, const css::uno::Any& Hidden,
    const css::uno::Any& ShowAll, const css::uno::Any& Drawings, const css::uno::Any& Anchors,
    const css::uno::Any& TextBoundaries, const css::uno::Any& VRuler,
    const css::uno::Any& Highlight)
{
    SAL_INFO("sw.vba", "WordBasic.ToolsOptionsView("
                       "DraftFont:="
                           << DraftFont << ", WrapToWindow:=" << WrapToWindow
                           << ", PicturePlaceHolders:=" << PicturePlaceHolders
                           << ", FieldCodes:=" << FieldCodes << ", BookMarks:=" << BookMarks
                           << ", FieldShading:=" << FieldShading << ", StatusBar:=" << StatusBar
                           << ", HScroll:=" << HScroll << ", VScroll:=" << VScroll
                           << ", StyleAreaWidth:=" << StyleAreaWidth << ", Tabs:=" << Tabs
                           << ", Spaces:=" << Spaces << ", Paras:=" << Paras
                           << ", Hyphens:=" << Hyphens << ", Hidden:=" << Hidden
                           << ", ShowAll:=" << ShowAll << ", Drawings:=" << Drawings
                           << ", Anchors:=" << Anchors << ", TextBoundaries:=" << TextBoundaries
                           << ", VRuler:=" << VRuler << ", Highlight:=" << Highlight << ")");
}

css::uno::Any SAL_CALL SwWordBasic::WindowName(const css::uno::Any& /*Number*/)
{
    return css::uno::Any(mpApp->getActiveSwVbaWindow()->getCaption());
}

css::uno::Any SAL_CALL SwWordBasic::ExistingBookmark(const OUString& Name)
{
    uno::Reference<word::XBookmarks> xBookmarks(mpApp->getActiveDocument()->Bookmarks(uno::Any()),
                                                uno::UNO_QUERY);
    return css::uno::Any(xBookmarks.is() && xBookmarks->Exists(Name));
}

void SAL_CALL SwWordBasic::MailMergeOpenDataSource(
    const OUString& Name, const css::uno::Any& Format, const css::uno::Any& ConfirmConversions,
    const css::uno::Any& ReadOnly, const css::uno::Any& LinkToSource,
    const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument,
    const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert,
    const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate,
    const css::uno::Any& Connection, const css::uno::Any& SQLStatement,
    const css::uno::Any& SQLStatement1, const css::uno::Any& OpenExclusive,
    const css::uno::Any& SubType)
{
    mpApp->getActiveDocument()->getMailMerge()->OpenDataSource(
        Name, Format, ConfirmConversions, ReadOnly, LinkToSource, AddToRecentFiles,
        PasswordDocument, PasswordTemplate, Revert, WritePasswordDocument, WritePasswordTemplate,
        Connection, SQLStatement, SQLStatement1, OpenExclusive, SubType);
}

css::uno::Any SAL_CALL SwWordBasic::AppMaximize(const css::uno::Any& WindowName,
                                                const css::uno::Any& State)
{
    SAL_INFO("sw.vba", "WordBasic.AppMaximize( WindowName:=" << WindowName << ", State:=" << State);

    // FIXME: Implement if necessary
    return css::uno::Any(sal_Int32(0));
}

css::uno::Any SAL_CALL SwWordBasic::DocMaximize(const css::uno::Any& State)
{
    SAL_INFO("sw.vba", "WordBasic.DocMaximize(State:=" << State << ")");

    // FIXME: Implement if necessary
    return css::uno::Any(sal_Int32(0));
}

void SAL_CALL SwWordBasic::AppShow(const css::uno::Any& WindowName)
{
    SAL_INFO("sw.vba", "WordBasic.AppShow(WindowName:=" << WindowName << ")");

    // FIXME: Implement if necessary
}

css::uno::Any SAL_CALL SwWordBasic::AppCount()
{
    SAL_INFO("sw.vba", "WordBasic.AppCount()");

    // FIXME: Implement if necessary. Return a random number for now.
    return css::uno::Any(sal_Int32(2));
}

void SAL_CALL SwWordBasic::MsgBox(const OUString& sPrompt)
{
    SbxArrayRef pArgs = new SbxArray;
    SbxVariable* pVar = new SbxVariable();
    pVar->PutString(sPrompt);
    pArgs->Put(pVar, 1);

    if (!executeRunTimeLibrary(u"MsgBox", pArgs.get()))
        SAL_WARN("sw.vba", "failed to execute runtime library function MsgBox (" << sPrompt << ")");
}

void SAL_CALL SwWordBasic::ScreenUpdating(const uno::Any& On)
{
    sal_Int32 nOn;
    if (On >>= nOn)
        mpApp->setScreenUpdating(nOn != 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
