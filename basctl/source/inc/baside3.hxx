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

#include "../basicide/layout.hxx"
#include "bastypes.hxx"
#include "propbrw.hxx"

#include <svl/undo.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>

#include <boost/scoped_ptr.hpp>

class Printer;
class StarBASIC;
class SfxItemSet;
class SfxUndoManager;

namespace basctl
{

class DlgEditor;
class DlgEdModel;
class DlgEdPage;
class DlgEdView;

class DialogWindowLayout;
class ObjectCatalog;

bool implImportDialog( vcl::Window* pWin, const OUString& rCurPath, const ScriptDocument& rDocument, const OUString& aLibName );

class DialogWindow: public BaseWindow
{
private:
    DialogWindowLayout& rLayout;
    boost::scoped_ptr<DlgEditor> pEditor;
    boost::scoped_ptr<SfxUndoManager> pUndoMgr; // never nullptr
    OUString            aCurPath;

protected:
    virtual void        Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        dispose() SAL_OVERRIDE;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void        Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void        LoseFocus() SAL_OVERRIDE;

    DECL_STATIC_LINK( DialogWindow, NotifyUndoActionHdl, SfxUndoAction * );
    virtual void        DoInit() SAL_OVERRIDE;
    virtual void        DoScroll( ScrollBar* pCurScrollBar ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    void                InitSettings(bool bFont, bool bForeground, bool bBackground);

public:
                        TYPEINFO_OVERRIDE();
    DialogWindow (DialogWindowLayout* pParent, ScriptDocument const& rDocument, const OUString& aLibName, const OUString& aName, com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> const& xDialogModel);
    DialogWindow( DialogWindow* pCurView ); // never implemented

    virtual void        ExecuteCommand( SfxRequest& rReq ) SAL_OVERRIDE;
    virtual void        GetState( SfxItemSet& ) SAL_OVERRIDE;
    DlgEditor&          GetEditor() const   { return *pEditor; }
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const;
    DlgEdModel&         GetModel() const;
    DlgEdPage&          GetPage() const;
    DlgEdView&          GetView() const;
    bool                RenameDialog( const OUString& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    bool                SaveDialog();
    bool                ImportDialog();

    virtual OUString             GetTitle() SAL_OVERRIDE;
    virtual EntryDescriptor      CreateEntryDescriptor() SAL_OVERRIDE;
    virtual void        SetReadOnly (bool bReadOnly) SAL_OVERRIDE;
    virtual bool        IsReadOnly() SAL_OVERRIDE;

    virtual void        StoreData() SAL_OVERRIDE;
    virtual bool        IsModified() SAL_OVERRIDE;
    virtual bool        IsPasteAllowed() SAL_OVERRIDE;

    virtual svl::IUndoManager* GetUndoManager() SAL_OVERRIDE;
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter ) SAL_OVERRIDE;
    // print page
    virtual void        printPage (sal_Int32 nPage, Printer*) SAL_OVERRIDE;

    virtual void        Activating () SAL_OVERRIDE;
    virtual void        Deactivating () SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

    virtual char const* GetHid () const SAL_OVERRIDE;
    virtual ItemType GetType () const SAL_OVERRIDE;
};


// DialogWindowLayout

class DialogWindowLayout : public Layout
{
public:
    DialogWindowLayout (vcl::Window* pParent, ObjectCatalog&);
    virtual ~DialogWindowLayout();
    virtual void dispose() SAL_OVERRIDE;
public:
    void ShowPropertyBrowser ();
    void UpdatePropertyBrowser ();
    void DisablePropertyBrowser ();
public:
    // Layout:
    virtual void Activating (BaseWindow&) SAL_OVERRIDE;
    virtual void Deactivating () SAL_OVERRIDE;
    virtual void ExecuteGlobal (SfxRequest&) SAL_OVERRIDE;
    virtual void GetState (SfxItemSet&, unsigned nWhich) SAL_OVERRIDE;
    virtual void UpdateDebug (bool) SAL_OVERRIDE {};
protected:
    // Layout:
    virtual void OnFirstSize (long nWidth, long nHeight) SAL_OVERRIDE;

private:
    // child window
    VclPtr<DialogWindow> pChild;
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
