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

#include <strings.hrc>
#include <basidesh.hxx>
#include <IDEComboBox.hxx>
#include <iderdll.hxx>
#include <localizationmgr.hxx>
#include <managelang.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svtools/langtab.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

namespace basctl
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

/*! Macro for implementation two methods for LibBoxControl Class
 *
 * @code
 * SfxToolBoxControl* LibBoxControl::CreateImpl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
 * {
 *      return new LibBoxControl(nSlotId, nId, rTbx);
 * }
 *
 * void LibBoxControl::RegisterControl(sal_uInt16 nSlotId, SfxModule* pMod)
 * {
 *      SfxToolBoxControl::RegisterToolBoxControl(
 *          pMod, SfxTbxCtrlFactory(* LibBoxControl::CreateImpl, typeid(nItemClass), nSlotId));
 * }
 * @endcode
 * @see Macro SFX_DECL_TOOLBOX_CONTROL
 */
SFX_IMPL_TOOLBOX_CONTROL(LibBoxControl, SfxStringItem);

LibBoxControl::LibBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

void LibBoxControl::StateChanged(sal_uInt16, SfxItemState eState, const SfxPoolItem* pState)
{
    LibBox* pBox = static_cast<LibBox*>(GetToolBox().GetItemWindow(GetId()));

    DBG_ASSERT(pBox, "Box not found");
    if (!pBox)
        return;

    if (eState != SfxItemState::DEFAULT)
        pBox->Disable();
    else
    {
        pBox->Enable();
        pBox->Update(dynamic_cast<SfxStringItem const*>(pState));
    }
}

VclPtr<vcl::Window> LibBoxControl::CreateItemWindow(vcl::Window* pParent)
{
    return VclPtr<LibBox>::Create(pParent);
}

DocListenerBox::DocListenerBox(vcl::Window* pParent)
    : ListBox(pParent, WinBits(WB_BORDER | WB_DROPDOWN))
    , maNotifier(*this)
{
}

DocListenerBox::~DocListenerBox() { disposeOnce(); }

void DocListenerBox::dispose()
{
    maNotifier.dispose();
    ListBox::dispose();
}

/// Only calls FillBox(). Parameter is not used.
void DocListenerBox::onDocumentCreated(const ScriptDocument& /*_rDoc*/) { FillBox(); }

/// Only calls FillBox(). Parameter is not used.
void DocListenerBox::onDocumentOpened(const ScriptDocument& /*_rDoc*/) { FillBox(); }

/// Only calls FillBox(). Parameter is not used.
void DocListenerBox::onDocumentSaveAsDone(const ScriptDocument& /*_rDoc*/) { FillBox(); }

/// Only calls FillBox(). Parameter is not used.
void DocListenerBox::onDocumentClosed(const ScriptDocument& /*_rDoc*/) { FillBox(); }

/// Not interested in. Do nothing.
void DocListenerBox::onDocumentSave(const ScriptDocument& /*_rDoc*/) {}

/// Not interested in. Do nothing.
void DocListenerBox::onDocumentSaveDone(const ScriptDocument& /*_rDoc*/) {}

/// Not interested in. Do nothing.
void DocListenerBox::onDocumentSaveAs(const ScriptDocument& /*_rDoc*/) {}

/// Not interested in. Do nothing.
void DocListenerBox::onDocumentTitleChanged(const ScriptDocument& /*_rDoc*/) {}

/// Not interested in. Do nothing.
void DocListenerBox::onDocumentModeChanged(const ScriptDocument& /*_rDoc*/) {}

LibBox::LibBox(vcl::Window* pParent)
    : DocListenerBox(pParent)
{
    FillBox();
    mbIgnoreSelect = true; // do not yet transfer select of 0
    mbFillBox = true;
    SelectEntryPos(0);
    maCurrentText = GetEntry(0);
    SetSizePixel(Size(250, 200));
    mbIgnoreSelect = false;
}

LibBox::~LibBox() { disposeOnce(); }

void LibBox::dispose()
{
    ClearBox();
    DocListenerBox::dispose();
}

void LibBox::Update(const SfxStringItem* pItem)
{
    //  if ( !pItem  || !pItem->GetValue().Len() )
    FillBox();

    if (pItem)
    {
        maCurrentText = pItem->GetValue();
        if (maCurrentText.isEmpty())
            maCurrentText = IDEResId(RID_STR_ALL);
    }

    if (GetSelectedEntry() != maCurrentText)
        SelectEntry(maCurrentText);
}

void LibBox::ReleaseFocus()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    DBG_ASSERT(pCurSh, "Current ViewShell not found!");

    if (pCurSh)
    {
        vcl::Window* pShellWin = pCurSh->GetWindow();
        if (!pShellWin)
            pShellWin = Application::GetDefDialogParent();

        pShellWin->GrabFocus();
    }
}

void LibBox::FillBox()
{
    SetUpdateMode(false);
    mbIgnoreSelect = true;

    maCurrentText = GetSelectedEntry();

    SelectEntryPos(0);
    ClearBox();

    // create list box entries
    sal_Int32 nPos = InsertEntry(IDEResId(RID_STR_ALL));
    SetEntryData(nPos, new LibEntry(ScriptDocument::getApplicationScriptDocument(),
                                    LIBRARY_LOCATION_UNKNOWN, OUString()));
    InsertEntries(ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER);
    InsertEntries(ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE);

    ScriptDocuments aDocuments(
        ScriptDocument::getAllScriptDocuments(ScriptDocument::DocumentsSorted));
    for (auto const& doc : aDocuments)
    {
        InsertEntries(doc, LIBRARY_LOCATION_DOCUMENT);
    }

    SetUpdateMode(true);

    SelectEntry(maCurrentText);
    if (!GetSelectedEntryCount())
    {
        SelectEntryPos(GetEntryCount());
        maCurrentText = GetSelectedEntry();
    }
    mbIgnoreSelect = false;
}

void LibBox::InsertEntries(const ScriptDocument& rDocument, LibraryLocation eLocation)
{
    // get a sorted list of library names
    Sequence<OUString> aLibNames = rDocument.getLibraryNames();
    sal_Int32 nLibCount = aLibNames.getLength();
    const OUString* pLibNames = aLibNames.getConstArray();

    for (sal_Int32 i = 0; i < nLibCount; ++i)
    {
        OUString aLibName = pLibNames[i];
        if (eLocation == rDocument.getLibraryLocation(aLibName))
        {
            OUString aName(rDocument.getTitle(eLocation));
            OUString aEntryText(CreateMgrAndLibStr(aName, aLibName));
            sal_Int32 nPos = InsertEntry(aEntryText);
            SetEntryData(nPos, new LibEntry(rDocument, eLocation, aLibName));
        }
    }
}

bool LibBox::PreNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        KeyEvent aKeyEvt = *rNEvt.GetKeyEvent();
        sal_uInt16 nKeyCode = aKeyEvt.GetKeyCode().GetCode();
        switch (nKeyCode)
        {
            case KEY_RETURN:
            {
                NotifyIDE();
                bDone = true;
            }
            break;
            case KEY_ESCAPE:
            {
                SelectEntry(maCurrentText);
                ReleaseFocus();
                bDone = true;
            }
            break;
        }
    }
    else if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
    {
        if (mbFillBox)
        {
            FillBox();
            mbFillBox = false;
        }
    }
    else if (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS)
    {
        if (!HasChildPathFocus(true))
        {
            mbIgnoreSelect = true;
            mbFillBox = true;
        }
    }

    return bDone || ListBox::PreNotify(rNEvt);
}

void LibBox::Select()
{
    if (!IsTravelSelect())
    {
        if (!mbIgnoreSelect)
            NotifyIDE();
        else
            SelectEntry(maCurrentText); // since 306... (Select after Escape)
    }
}

void LibBox::NotifyIDE()
{
    sal_Int32 nSelPos = GetSelectedEntryPos();
    if (LibEntry* pEntry = static_cast<LibEntry*>(GetEntryData(nSelPos)))
    {
        const ScriptDocument& aDocument(pEntry->GetDocument());
        SfxUnoAnyItem aDocumentItem(SID_BASICIDE_ARG_DOCUMENT_MODEL,
                                    uno::Any(aDocument.getDocumentOrNull()));
        const OUString& aLibName = pEntry->GetLibName();
        SfxStringItem aLibNameItem(SID_BASICIDE_ARG_LIBNAME, aLibName);
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->ExecuteList(SID_BASICIDE_LIBSELECTED, SfxCallMode::SYNCHRON,
                                     { &aDocumentItem, &aLibNameItem });
    }
    ReleaseFocus();
}

void LibBox::ClearBox()
{
    sal_Int32 nCount = GetEntryCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        LibEntry* pEntry = static_cast<LibEntry*>(GetEntryData(i));
        delete pEntry;
    }
    ListBox::Clear();
}

// class LanguageBoxControl ----------------------------------------------

/*! Macro for implementation two methods for LanguageBoxControl Class
 *
 * @code
 * SfxToolBoxControl* LanguageBoxControl::CreateImpl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
 * {
 *      return new LanguageBoxControl(nSlotId, nId, rTbx);
 * }
 *
 * void LanguageBoxControl::RegisterControl(sal_uInt16 nSlotId, SfxModule* pMod)
 * {
 *      SfxToolBoxControl::RegisterToolBoxControl(
 *          pMod, SfxTbxCtrlFactory(* LanguageBoxControl::CreateImpl, typeid(nItemClass), nSlotId));
 * }
 * @endcode
 * @see Macro SFX_DECL_TOOLBOX_CONTROL
 */
SFX_IMPL_TOOLBOX_CONTROL(LanguageBoxControl, SfxStringItem);

LanguageBoxControl::LanguageBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

void LanguageBoxControl::StateChanged(sal_uInt16, SfxItemState eState, const SfxPoolItem* pItem)
{
    if (LanguageBox* pBox = static_cast<LanguageBox*>(GetToolBox().GetItemWindow(GetId())))
    {
        if (eState != SfxItemState::DEFAULT)
            pBox->Disable();
        else
        {
            pBox->Enable();
            pBox->Update(dynamic_cast<SfxStringItem const*>(pItem));
        }
    }
}

VclPtr<vcl::Window> LanguageBoxControl::CreateItemWindow(vcl::Window* pParent)
{
    return VclPtr<LanguageBox>::Create(pParent);
}

// class basctl::LanguageBox -----------------------------------------------
LanguageBox::LanguageBox(vcl::Window* pParent)
    : DocListenerBox(pParent)
    , msNotLocalizedStr(IDEResId(RID_STR_TRANSLATION_NOTLOCALIZED))
    , msDefaultLanguageStr(IDEResId(RID_STR_TRANSLATION_DEFAULT))
    , mbIgnoreSelect(false)
{
    SetSizePixel(Size(210, 200));
    FillBox();
}

LanguageBox::~LanguageBox() { disposeOnce(); }

void LanguageBox::dispose()
{
    ClearBox();
    DocListenerBox::dispose();
}

void LanguageBox::FillBox()
{
    SetUpdateMode(false);
    mbIgnoreSelect = true;
    msCurrentText = GetSelectedEntry();
    ClearBox();

    std::shared_ptr<LocalizationMgr> pCurMgr(GetShell()->GetCurLocalizationMgr());
    if (pCurMgr->isLibraryLocalized())
    {
        Enable();
        Locale aDefaultLocale = pCurMgr->getStringResourceManager()->getDefaultLocale();
        Locale aCurrentLocale = pCurMgr->getStringResourceManager()->getCurrentLocale();
        Sequence<Locale> aLocaleSeq = pCurMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        sal_Int32 i, nCount = aLocaleSeq.getLength();
        sal_Int32 nSelPos = LISTBOX_ENTRY_NOTFOUND;
        for (i = 0; i < nCount; ++i)
        {
            bool bIsDefault = localesAreEqual(aDefaultLocale, pLocale[i]);
            bool bIsCurrent = localesAreEqual(aCurrentLocale, pLocale[i]);
            LanguageType eLangType = LanguageTag::convertToLanguageType(pLocale[i]);
            OUString sLanguage = SvtLanguageTable::GetLanguageString(eLangType);
            if (bIsDefault)
            {
                sLanguage += " " + msDefaultLanguageStr;
            }
            sal_Int32 nPos = InsertEntry(sLanguage);
            SetEntryData(nPos, new LanguageEntry(pLocale[i], bIsDefault));

            if (bIsCurrent)
                nSelPos = nPos;
        }

        if (nSelPos != LISTBOX_ENTRY_NOTFOUND)
        {
            SelectEntryPos(nSelPos);
            msCurrentText = GetSelectedEntry();
        }
    }
    else
    {
        InsertEntry(msNotLocalizedStr);
        SelectEntryPos(0);
        Disable();
    }

    SetUpdateMode(true);
    mbIgnoreSelect = false;
}

void LanguageBox::ClearBox()
{
    sal_Int32 nCount = GetEntryCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        LanguageEntry* pEntry = static_cast<LanguageEntry*>(GetEntryData(i));
        delete pEntry;
    }
    ListBox::Clear();
}

void LanguageBox::SetLanguage()
{
    LanguageEntry* pEntry = static_cast<LanguageEntry*>(GetSelectedEntryData());
    if (pEntry)
        GetShell()->GetCurLocalizationMgr()->handleSetCurrentLocale(pEntry->m_aLocale);
}

void LanguageBox::Select()
{
    if (!mbIgnoreSelect)
        SetLanguage();
    else
        SelectEntry(msCurrentText); // Select after Escape
}

bool LanguageBox::PreNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        sal_uInt16 nKeyCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
        switch (nKeyCode)
        {
            case KEY_RETURN:
            {
                SetLanguage();
                bDone = true;
            }
            break;
            case KEY_ESCAPE:
            {
                SelectEntry(msCurrentText);
                bDone = true;
            }
            break;
        }
    }

    return bDone || ListBox::PreNotify(rNEvt);
}

void LanguageBox::Update(const SfxStringItem* pItem)
{
    FillBox();

    if (pItem && !pItem->GetValue().isEmpty())
    {
        msCurrentText = pItem->GetValue();
        if (GetSelectedEntry() != msCurrentText)
            SelectEntry(msCurrentText);
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
