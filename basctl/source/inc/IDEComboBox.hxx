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

#pragma once

#include <svl/stritem.hxx>
#include <sfx2/tbxctrl.hxx>
#include <vcl/InterimItemWindow.hxx>

#include "doceventnotifier.hxx"
#include "scriptdocument.hxx"

namespace basctl
{
/*!
 * @brief Manage states of macro and dialog Library ComboBox
 *
 * @see LibBox Class
 */
class LibBoxControl : public SfxToolBoxControl
{
public:
    /*!
     * Macro for registering two methods
     *
     * @code
     * static SfxToolBoxControl* CreateImpl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
     * static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule* pMod=nullptr)
     * @endcode
     * @see Macro SFX_IMPL_TOOLBOX_CONTROL
     */
    SFX_DECL_TOOLBOX_CONTROL();

    /*!
     * @param nSlotId -- the slot as internal operation number
     * @param nId -- this item's unique id in ToolBox
     * @param rTbx -- the ToolBox which contains this ComboBox
     */
    LibBoxControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);

    /*!
     * Triggered if state was changed
     *
     * @param nSlotID -- the slot as internal operation number (not used in this place)
     * @param eState -- enum value which contains ComboBox state
     * @param pState --
     */
    virtual void StateChanged(sal_uInt16 nSlotID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    /*!
     * Create combobox of Macro and Dialog Library
     *
     * @param pParent -- parent window
     * @return ComboBox of macro and dialog Library
     */
    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

/*!
 * @brief Base class for all ComboBox elements.
 *
 * Base class for ComboBoxes which need to update their content according
 * to the list of open documents.
 */
class DocListenerBox : public InterimItemWindow, public DocumentEventListener
{
private:
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

protected:
    std::unique_ptr<weld::ComboBox> m_xWidget;

    /// @param pParent -- parent window
    DocListenerBox(vcl::Window* pParent);
    virtual ~DocListenerBox() override;
    virtual void dispose() override;

    virtual void Select() = 0;
    virtual void FillBox() = 0;

    /// key strokes the ComboBox receives
    virtual bool HandleKeyInput(const KeyEvent& rKEvt);

private:
    // DocumentEventListener
    virtual void onDocumentCreated(const ScriptDocument& _rDoc) override;
    virtual void onDocumentOpened(const ScriptDocument& _rDoc) override;
    virtual void onDocumentSave(const ScriptDocument& _rDoc) override;
    virtual void onDocumentSaveDone(const ScriptDocument& _rDoc) override;
    virtual void onDocumentSaveAs(const ScriptDocument& _rDoc) override;
    virtual void onDocumentSaveAsDone(const ScriptDocument& _rDoc) override;
    virtual void onDocumentClosed(const ScriptDocument& _rDoc) override;
    virtual void onDocumentTitleChanged(const ScriptDocument& _rDoc) override;
    virtual void onDocumentModeChanged(const ScriptDocument& _rDoc) override;

    DocumentEventNotifier maNotifier;

public:
    void set_sensitive(bool bSensitive);
};

/*!
 * @brief Macros and Dialogs Library ComboBox
 *
 * @see LibBoxControl Class
 */
class LibBox : public DocListenerBox
{
public:
    /// @param pParent
    LibBox(vcl::Window* pParent);
    virtual ~LibBox() override;
    virtual void dispose() override;

    /*!
     * Update selection in ComboBox of macro and dialog Library
     *
     * @param pItem -- string that was selected
     */
    void Update(const SfxStringItem* pItem);

protected:
    /// Called for setting language when user selects a language in ComboBox
    virtual void Select() override;

    /*!
     * Handle keystrokes and mouse
     *
     * @param rNEvt represents mouse event
     * @return a bool value: true if was handled, and false if there was nothing handled
     */
    //TODO    virtual bool PreNotify(NotifyEvent& rNEvt) override;

private:
    static void ReleaseFocus();

    /*!
     * Insert name library in specified position
     *
     * @param rDocument -- macro or dialog
     * @param eLocation -- enum value of Locations
     */
    void InsertEntries(const ScriptDocument& rDocument, LibraryLocation eLocation);

    void ClearBox();
    void NotifyIDE();

    /// Fill up the combobox
    virtual void FillBox() override;

    virtual bool HandleKeyInput(const KeyEvent& rKEvt) override;

    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);

    OUString maCurrentText;
    bool mbIgnoreSelect;
    bool mbFillBox; ///< If true, when FillBox() is called
};

/*!
 * @brief Manage stats of Language ComboBox
 *
 * @see LanguageBox Class
 */
class LanguageBoxControl : public SfxToolBoxControl
{
public:
    /*! Macro for registering two methods
     *
     * @code
     * static SfxToolBoxControl* CreateImpl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
     * static void RegisterControl(sal_uInt16 nSlotId = 0, SfxModule* pMod=nullptr)
     * @endcode
     * @see Macro SFX_IMPL_TOOLBOX_CONTROL
     */
    SFX_DECL_TOOLBOX_CONTROL();

    /*!
     * @param nSlotId -- the slot as internal operation number
     * @param nId -- this item's unique id in ToolBox
     * @param rTbx -- the ToolBox which contains this ComboBox
     */
    LanguageBoxControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx);

    /*!
     * Triggered if state was changed
     *
     * @param nSlotID -- the slot as internal operation number (not used in this place)
     * @param eState -- enum value which contains ComboBox state
     * @param pState --
     */
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    /*!
     * Create ComboBox of Language
     *
     * @param pParent
     * @return LanguageBox ComboBox
     */
    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

/*!
 * @brief Class language ComboBox
 *
 * @see LanguageBoxControl Class
 */
class LanguageBox : public DocListenerBox
{
public:
    /*!
     * @param pParent
     */
    LanguageBox(vcl::Window* pParent);
    virtual ~LanguageBox() override;
    virtual void dispose() override;

    /*!
     * Update selection in ComboBox of macro and dialog Library
     *
     * @param pItem -- string that was selected
     */
    void Update(const SfxStringItem* pItem);

protected:
    /// Called for setting language when user selects a language in ComboBox
    virtual void Select() override;

    virtual bool HandleKeyInput(const KeyEvent& rKEvt) override;

    /*!
     * Handle keystrokes and mouse
     *
     * @param rNEvt represents mouse event
     * @return a bool value: true if was handled, and false if there was nothing handled
     */
    //TODO    virtual bool PreNotify(NotifyEvent& rNEvt) override;

private:
    /// Delete all languages from ComboBox
    void ClearBox();
    /// Switch interface of dialog to selected language
    void SetLanguage();

    /// Fill up the language combobox
    virtual void FillBox() override;

    OUString msNotLocalizedStr;
    OUString msDefaultLanguageStr;
    OUString msCurrentText;

    bool mbIgnoreSelect; ///< do not use in this class
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
