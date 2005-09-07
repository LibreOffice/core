/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baside3.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:17:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BASIDE3_HXX
#define _BASIDE3_HXX

#ifndef _SVHEADER_HXX
//#include <svheader.hxx>
#endif

#include <bastypes.hxx>
#include <svtools/undo.hxx>

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
    void                InitSettings(BOOL bFont,BOOL bForeground,BOOL bBackground);

public:
                        TYPEINFO();
                        DialogWindow( Window* pParent, SfxObjectShell* pShell, String aLibName, String aName,
                            const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xDialogModel );
                        DialogWindow( DialogWindow* pCurView );
                        ~DialogWindow();

    virtual void        ExecuteCommand( SfxRequest& rReq );
    virtual void        GetState( SfxItemSet& );
    DlgEditor*          GetEditor() const   { return pEditor; }
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > GetDialog() const;
    DlgEdModel*         GetModel() const;
    DlgEdPage*          GetPage() const;
    DlgEdView*          GetView() const;
    BOOL                RenameDialog( const String& rNewName );
    void                DisableBrowser();
    void                UpdateBrowser();
    virtual String      GetTitle();
    virtual BasicEntryDescriptor CreateEntryDescriptor();
    virtual void        SetReadOnly( BOOL bReadOnly );
    virtual BOOL        IsReadOnly();

    virtual void        StoreData();
    virtual BOOL        IsModified();
    virtual BOOL        IsPasteAllowed();

    virtual SfxUndoManager* GetUndoManager();
    virtual void        PrintData( Printer* pPrinter );
    virtual void        Deactivating();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

#endif  // _BASIDE3_HXX
