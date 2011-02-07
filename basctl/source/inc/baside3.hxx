/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BASIDE3_HXX
#define _BASIDE3_HXX

#ifndef _SVHEADER_HXX
//#include <svheader.hxx>
#endif

#include <bastypes.hxx>
#include <svl/undo.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif

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
    String              aCurPath;

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
                        DialogWindow( Window* pParent, const ScriptDocument& rDocument, String aLibName, String aName,
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
    sal_Bool                RenameDialog( const String& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    sal_Bool                SaveDialog();
    sal_Bool                ImportDialog();

    virtual String      GetTitle();
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
