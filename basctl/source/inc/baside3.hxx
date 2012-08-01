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

#ifndef _BASIDE3_HXX
#define _BASIDE3_HXX

#include <bastypes.hxx>
#include <svl/undo.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <com/sun/star/script/XLibraryContainer.hpp>

class Printer;
class StarBASIC;
class SfxItemSet;
class DlgEditor;
class DlgEdModel;
class DlgEdPage;
class DlgEdView;
class SfxUndoManager;

class DialogWindow: public IDEBaseWindow
{
private:
    DlgEditor*          pEditor;
    SfxUndoManager*     pUndoMgr;
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
    void                InitSettings(sal_Bool bFont,sal_Bool bForeground,sal_Bool bBackground);

public:
                        TYPEINFO();
    DialogWindow( Window* pParent, const ScriptDocument& rDocument, ::rtl::OUString aLibName, ::rtl::OUString aName,
                            const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xDialogModel );
                        DialogWindow( DialogWindow* pCurView ); // never implemented
                        ~DialogWindow();

    virtual void        ExecuteCommand( SfxRequest& rReq );
    virtual void        GetState( SfxItemSet& );
    DlgEditor*          GetEditor() const   { return pEditor; }
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const;
    DlgEdModel*         GetModel() const;
    DlgEdPage*          GetPage() const;
    DlgEdView*          GetView() const;
    bool                RenameDialog( const ::rtl::OUString& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    bool                SaveDialog();
    bool                ImportDialog();

    virtual ::rtl::OUString      GetTitle();
    virtual BasicEntryDescriptor CreateEntryDescriptor();
    virtual void        SetReadOnly( sal_Bool bReadOnly );
    virtual sal_Bool        IsReadOnly();

    virtual void        StoreData();
    virtual sal_Bool        IsModified();
    virtual sal_Bool        IsPasteAllowed();

    virtual ::svl::IUndoManager*
                        GetUndoManager();
    // return number of pages to be printed
    virtual sal_Int32 countPages( Printer* pPrinter );
    // print page
    virtual void printPage( sal_Int32 nPage, Printer* pPrinter );
    virtual void        Deactivating();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

#endif  // _BASIDE3_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
