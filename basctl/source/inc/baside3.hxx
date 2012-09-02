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

#ifndef BASCTL_BASIDE3_HXX
#define BASCTL_BASIDE3_HXX

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

class DialogWindow: public BaseWindow
{
private:
    DialogWindowLayout& rLayout;
    boost::scoped_ptr<DlgEditor> pEditor; // never nullptr
    boost::scoped_ptr<SfxUndoManager> pUndoMgr; // never nullptr
    Link                aOldNotifyUndoActionHdl;
    ::rtl::OUString     aCurPath;

protected:
    virtual void        Paint( const Rectangle& );
    virtual void        Resize();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        LoseFocus();

    DECL_LINK( NotifyUndoActionHdl, SfxUndoAction * );
    virtual void        DoInit();
    virtual void        DoScroll( ScrollBar* pCurScrollBar );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    void                InitSettings(bool bFont, bool bForeground, bool bBackground);

public:
                        TYPEINFO();
    DialogWindow (DialogWindowLayout* pParent, ScriptDocument const& rDocument, rtl::OUString aLibName, rtl::OUString aName, com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> const& xDialogModel);
                        DialogWindow( DialogWindow* pCurView ); // never implemented
                        ~DialogWindow();

    virtual void        ExecuteCommand( SfxRequest& rReq );
    virtual void        GetState( SfxItemSet& );
    DlgEditor&          GetEditor() const   { return *pEditor; }
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const;
    DlgEdModel&         GetModel() const;
    DlgEdPage&          GetPage() const;
    DlgEdView&          GetView() const;
    bool                RenameDialog( const ::rtl::OUString& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    bool                SaveDialog();
    bool                ImportDialog();

    virtual ::rtl::OUString      GetTitle();
    virtual EntryDescriptor      CreateEntryDescriptor();
    virtual void        SetReadOnly (bool bReadOnly);
    virtual bool        IsReadOnly();

    virtual void        StoreData();
    virtual bool        IsModified();
    virtual bool        IsPasteAllowed();

    virtual svl::IUndoManager* GetUndoManager();
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void        printPage (sal_Int32 nPage, Printer*);

    virtual void        Activating ();
    virtual void        Deactivating ();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    virtual char const* GetHid () const;
    virtual ItemType GetType () const;
};

//
// DialogWindowLayout
//
class DialogWindowLayout : public Layout
{
public:
    DialogWindowLayout (Window* pParent, ObjectCatalog&);
public:
    void ShowPropertyBrowser ();
    void UpdatePropertyBrowser ();
    void DisablePropertyBrowser ();
    void RemovePropertyBrowser ();
public:
    // Layout:
    virtual void Activating (BaseWindow&);
    virtual void Deactivating ();
    virtual void ExecuteGlobal (SfxRequest&);
    virtual void GetState (SfxItemSet&, unsigned nWhich);
    virtual void UpdateDebug (bool){};
protected:
    // Layout:
    virtual void OnFirstSize (int nWidth, int nHeight);

private:
    // child window
    DialogWindow* pChild;
    // dockable windows:
    // object catalog (owned by Shell)
    ObjectCatalog& rObjectCatalog;
    // property browser (created by this, deleted by toolkit)
    PropBrw* pPropertyBrowser;

private:
    void AddPropertyBrowser ();
private:
    friend class DialogWindow;
};


} // namespace basctl

#endif // BASCTL_BASIDE3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
