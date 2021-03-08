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
#include <basobj.hxx>
#include <IDEComboBox.hxx>
#include <iderdll.hxx>
#include <iderid.hxx>
#include <localizationmgr.hxx>
#include <managelang.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svtools/langtab.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
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

LibBoxControl::LibBoxControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
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
        pBox->set_sensitive(false);
    else
    {
        pBox->set_sensitive(true);
        pBox->Update(dynamic_cast<SfxStringItem const*>(pState));
    }
}

VclPtr<InterimItemWindow> LibBoxControl::CreateItemWindow(vcl::Window* pParent)
{
    return VclPtr<LibBox>::Create(pParent);
}

DocListenerBox::DocListenerBox(vcl::Window* pParent)
    : InterimItemWindow(pParent, "modules/BasicIDE/ui/combobox.ui", "ComboBox")
    , m_xWidget(m_xBuilder->weld_combo_box("combobox"))
    , maNotifier(*this)
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_changed(LINK(this, DocListenerBox, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, DocListenerBox, KeyInputHdl));
}

void DocListenerBox::set_sensitive(bool bSensitive)
{
    Enable(bSensitive);
    m_xWidget->set_sensitive(bSensitive);
}

IMPL_LINK(DocListenerBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return HandleKeyInput(rKEvt);
}

bool DocListenerBox::HandleKeyInput(const KeyEvent& rKEvt) { return ChildKeyInput(rKEvt); }

IMPL_LINK_NOARG(DocListenerBox, SelectHdl, weld::ComboBox&, void) { Select(); }

DocListenerBox::~DocListenerBox() { disposeOnce(); }

void DocListenerBox::dispose()
{
    maNotifier.dispose();
    m_xWidget.reset();
    InterimItemWindow::dispose();
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
    m_xWidget->set_active(0);
    maCurrentText = m_xWidget->get_text(0);
    mbIgnoreSelect = false;

    m_xWidget->connect_focus_in(LINK(this, LibBox, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, LibBox, FocusOutHdl));

    SetSizePixel(m_xWidget->get_preferred_size());
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

    if (m_xWidget->get_active_text() != maCurrentText)
        m_xWidget->set_active_text(maCurrentText);
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
    m_xWidget->freeze();
    mbIgnoreSelect = true;

    maCurrentText = m_xWidget->get_active_text();

    ClearBox();

    // create list box entries
    LibEntry* pEntry = new LibEntry(ScriptDocument::getApplicationScriptDocument(),
                                    LIBRARY_LOCATION_UNKNOWN, OUString());
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
    m_xWidget->append(sId, IDEResId(RID_STR_ALL));

    InsertEntries(ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_USER);
    InsertEntries(ScriptDocument::getApplicationScriptDocument(), LIBRARY_LOCATION_SHARE);

    ScriptDocuments aDocuments(
        ScriptDocument::getAllScriptDocuments(ScriptDocument::DocumentsSorted));
    for (auto const& doc : aDocuments)
    {
        InsertEntries(doc, LIBRARY_LOCATION_DOCUMENT);
    }

    m_xWidget->thaw();

    int nIndex = m_xWidget->find_text(maCurrentText);
    if (nIndex != -1)
        m_xWidget->set_active(nIndex);
    else
        m_xWidget->set_active(0);
    maCurrentText = m_xWidget->get_active_text();
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
            LibEntry* pEntry = new LibEntry(rDocument, eLocation, aLibName);
            m_xWidget->append(OUString::number(reinterpret_cast<sal_Int64>(pEntry)), aEntryText);
        }
    }
}

bool LibBox::HandleKeyInput(const KeyEvent& rKEvt)
{
    bool bDone = false;

    sal_uInt16 nKeyCode = rKEvt.GetKeyCode().GetCode();
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
            m_xWidget->set_active_text(maCurrentText);
            ReleaseFocus();
            bDone = true;
        }
        break;
    }

    return bDone || DocListenerBox::HandleKeyInput(rKEvt);
}

IMPL_LINK_NOARG(LibBox, FocusInHdl, weld::Widget&, void)
{
    if (mbFillBox)
    {
        FillBox();
        mbFillBox = false;
    }
}

IMPL_LINK_NOARG(LibBox, FocusOutHdl, weld::Widget&, void)
{
    // comboboxes can be comprised of multiple widgets, ensure all have lost focus
    if (m_xWidget && !m_xWidget->has_focus())
        mbFillBox = true;
}

void LibBox::Select()
{
    if (m_xWidget->changed_by_direct_pick())
    {
        if (!mbIgnoreSelect)
            NotifyIDE();
        else
            m_xWidget->set_active_text(maCurrentText); // (Select after Escape)
    }
}

void LibBox::NotifyIDE()
{
    LibEntry* pEntry = reinterpret_cast<LibEntry*>(m_xWidget->get_active_id().toInt64());
    if (pEntry)
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
    sal_Int32 nCount = m_xWidget->get_count();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        LibEntry* pEntry = reinterpret_cast<LibEntry*>(m_xWidget->get_id(i).toInt64());
        delete pEntry;
    }
    m_xWidget->clear();
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

LanguageBoxControl::LanguageBoxControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

void LanguageBoxControl::StateChanged(sal_uInt16, SfxItemState eState, const SfxPoolItem* pItem)
{
    if (LanguageBox* pBox = static_cast<LanguageBox*>(GetToolBox().GetItemWindow(GetId())))
    {
        if (eState != SfxItemState::DEFAULT)
            pBox->set_sensitive(false);
        else
        {
            pBox->set_sensitive(true);
            pBox->Update(dynamic_cast<SfxStringItem const*>(pItem));
        }
    }
}

VclPtr<InterimItemWindow> LanguageBoxControl::CreateItemWindow(vcl::Window* pParent)
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
    FillBox();

    SetSizePixel(m_xWidget->get_preferred_size());
}

LanguageBox::~LanguageBox() { disposeOnce(); }

void LanguageBox::dispose()
{
    ClearBox();
    DocListenerBox::dispose();
}

void LanguageBox::FillBox()
{
    m_xWidget->freeze();
    mbIgnoreSelect = true;
    msCurrentText = m_xWidget->get_active_text();
    ClearBox();

    sal_Int32 nSelPos = -1;

    std::shared_ptr<LocalizationMgr> pCurMgr(GetShell()->GetCurLocalizationMgr());
    if (pCurMgr->isLibraryLocalized())
    {
        set_sensitive(true);
        Locale aDefaultLocale = pCurMgr->getStringResourceManager()->getDefaultLocale();
        Locale aCurrentLocale = pCurMgr->getStringResourceManager()->getCurrentLocale();
        Sequence<Locale> aLocaleSeq = pCurMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        sal_Int32 i, nCount = aLocaleSeq.getLength();
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
            LanguageEntry* pEntry = new LanguageEntry(pLocale[i], bIsDefault);
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntry)));
            m_xWidget->append(sId, sLanguage);

            if (bIsCurrent)
                nSelPos = i;
        }

        if (nSelPos != -1)
            msCurrentText = m_xWidget->get_text(nSelPos);
    }
    else
    {
        m_xWidget->append_text(msNotLocalizedStr);
        nSelPos = 0;
        set_sensitive(false);
    }

    m_xWidget->thaw();
    m_xWidget->set_active(nSelPos);
    mbIgnoreSelect = false;
}

void LanguageBox::ClearBox()
{
    sal_Int32 nCount = m_xWidget->get_count();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        LanguageEntry* pEntry = reinterpret_cast<LanguageEntry*>(m_xWidget->get_id(i).toInt64());
        delete pEntry;
    }
    m_xWidget->clear();
}

void LanguageBox::SetLanguage()
{
    LanguageEntry* pEntry = reinterpret_cast<LanguageEntry*>(m_xWidget->get_active_id().toInt64());
    if (pEntry)
        GetShell()->GetCurLocalizationMgr()->handleSetCurrentLocale(pEntry->m_aLocale);
}

void LanguageBox::Select()
{
    if (!mbIgnoreSelect)
        SetLanguage();
    else
        m_xWidget->set_active_text(msCurrentText); // Select after Escape
}

bool LanguageBox::HandleKeyInput(const KeyEvent& rKEvt)
{
    bool bDone = false;

    sal_uInt16 nKeyCode = rKEvt.GetKeyCode().GetCode();
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
            m_xWidget->set_active_text(msCurrentText);
            bDone = true;
        }
        break;
    }

    return bDone || DocListenerBox::HandleKeyInput(rKEvt);
}

void LanguageBox::Update(const SfxStringItem* pItem)
{
    FillBox();

    if (pItem && !pItem->GetValue().isEmpty())
    {
        msCurrentText = pItem->GetValue();
        if (m_xWidget->get_active_text() != msCurrentText)
            m_xWidget->set_active_text(msCurrentText);
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
