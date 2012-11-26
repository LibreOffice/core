/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
