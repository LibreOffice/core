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

#ifndef INCLUDED_BASCTL_SOURCE_INC_BASIDE3_HXX
#define INCLUDED_BASCTL_SOURCE_INC_BASIDE3_HXX

#include "layout.hxx"
#include "bastypes.hxx"
#include "propbrw.hxx"
#include <svl/undo.hxx>
#include <memory>

class Printer;
class StarBASIC;
class SfxItemSet;
class SfxUndoManager;
class SdrUndoAction;

namespace basctl
{

class DlgEditor;
class DlgEdModel;
class DlgEdPage;
class DlgEdView;

class DialogWindowLayout;
class ObjectCatalog;

bool implImportDialog(weld::Window* pWin, const OUString& rCurPath, const ScriptDocument& rDocument, const OUString& rLibName);

class DialogWindow: public BaseWindow
{
private:
    DialogWindowLayout& m_rLayout;
    std::unique_ptr<DlgEditor> m_pEditor;
    std::unique_ptr<SfxUndoManager> m_pUndoMgr; // never nullptr
    OUString            m_sCurPath;
    sal_uInt16          m_nControlSlotId;

protected:
    virtual void        Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void        Resize() override;
    virtual void        dispose() override;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        LoseFocus() override;

    static void         NotifyUndoActionHdl( std::unique_ptr<SdrUndoAction> );
    virtual void        DoInit() override;
    virtual void        DoScroll( ScrollBar* pCurScrollBar ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    void                InitSettings();

public:
    DialogWindow (DialogWindowLayout* pParent, ScriptDocument const& rDocument, const OUString& aLibName, const OUString& aName, css::uno::Reference<css::container::XNameContainer> const& xDialogModel);

    virtual void        ExecuteCommand( SfxRequest& rReq ) override;
    virtual void        GetState( SfxItemSet& ) override;
    DlgEditor&          GetEditor() const   { return *m_pEditor; }
    css::uno::Reference< css::container::XNameContainer > const & GetDialog() const;
    DlgEdModel&         GetModel() const;
    DlgEdPage&          GetPage() const;
    DlgEdView&          GetView() const;
    bool                RenameDialog( const OUString& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    void                SaveDialog();
    void                ImportDialog();

    virtual OUString             GetTitle() override;
    virtual EntryDescriptor      CreateEntryDescriptor() override;
    virtual void        SetReadOnly (bool bReadOnly) override;
    virtual bool        IsReadOnly() override;

    virtual void        StoreData() override;
    virtual bool        IsModified() override;
    bool                IsPasteAllowed();

    virtual SfxUndoManager* GetUndoManager() override;
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) override;
    // print page
    virtual void        printPage (sal_Int32 nPage, Printer*) override;

    virtual void        Activating () override;
    virtual void        Deactivating () override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    virtual char const* GetHid () const override;
    virtual ItemType GetType () const override;
};


// DialogWindowLayout

class DialogWindowLayout : public Layout
{
public:
    DialogWindowLayout (vcl::Window* pParent, ObjectCatalog&);
    virtual ~DialogWindowLayout() override;
    virtual void dispose() override;
public:
    void ShowPropertyBrowser ();
    void UpdatePropertyBrowser ();
    void DisablePropertyBrowser ();
public:
    // Layout:
    virtual void Activating (BaseWindow&) override;
    virtual void Deactivating () override;
    virtual void ExecuteGlobal (SfxRequest&) override;
    virtual void GetState (SfxItemSet&, unsigned nWhich) override;
    virtual void UpdateDebug (bool) override {};
protected:
    // Layout:
    virtual void OnFirstSize (long nWidth, long nHeight) override;

private:
    // dockable windows:
    // object catalog (owned by Shell)
    ObjectCatalog& rObjectCatalog;
    // property browser (created by this, deleted by toolkit)
    VclPtr<PropBrw> pPropertyBrowser;

private:
    void AddPropertyBrowser ();
private:
    friend class DialogWindow;
};


} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_BASIDE3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
