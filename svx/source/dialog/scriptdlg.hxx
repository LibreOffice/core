/*************************************************************************
 *
 *  $RCSfile: scriptdlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 14:16:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SCRIPTDLG_HXX
#define _SCRIPTDLG_HXX

#include <memory>

#include "tools/solar.h"

#include <svtools/svtreebx.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/abstdlg.hxx>
#include <sfx2/basedlgs.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <drafts/com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <hash_map>

#define OBJTYPE_BASICMANAGER    1L
//#define OBJTYPE_LIB               2L
//#define OBJTYPE_MODULE            3L
#define OBJTYPE_METHOD          2L
//#define OBJTYPE_METHODINOBJ       5L
//#define OBJTYPE_OBJECT            6L
//#define OBJTYPE_SUBOBJ            7L
//#define OBJTYPE_PROPERTY      8L
#define OBJTYPE_SCRIPTCONTAINER     3L
#define OBJTYPE_SFROOT      4L

#define BROWSEMODE_MODULES      0x01
#define BROWSEMODE_SUBS         0x02
#define BROWSEMODE_OBJS         0x04
#define BROWSEMODE_PROPS        0x08
#define BROWSEMODE_SUBOBJS      0x10

#define INPUTMODE_NEWLIB        1
#define INPUTMODE_NEWMACRO      2
#define INPUTMODE_RENAME        3

typedef ::std::hash_map < ::rtl::OUString, ::rtl::OUString ,
    ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > Selection_hash;

class SFEntry;

class SFTreeListBox : public SvTreeListBox
{
   friend class SvxScriptOrgDialog;
private:
    USHORT          nMode;
    ImageList m_aImagesNormal;
    ImageList m_aImagesHighContrast;
    Image m_hdImage;
    Image m_hdImage_hc;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >
        getLangNodeFromRootNode( ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >& root, ::rtl::OUString& language );
    void delUserData( SvLBoxEntry* pEntry );

    void setEntryBitmap(SvLBoxEntry * pEntry, USHORT nBitmap);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );
    ::rtl::OUString SFTreeListBox::xModelToDocTitle( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel );
    ::rtl::OUString SFTreeListBox::parseLocationName( const ::rtl::OUString& location );
protected:
    void                    ExpandTree( SvLBoxEntry* pRootEntry );
    virtual void            RequestingChilds( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    SvLBoxEntry*            FindEntry( SvLBoxEntry* pParent, const String& rText, BYTE nType );
    virtual long            ExpandingHdl();
    static BOOL             dialogSort1( com::sun::star::uno::Reference< drafts::com::sun::star::script::browse::XBrowseNode > node1,
            com::sun::star::uno::Reference< drafts::com::sun::star::script::browse::XBrowseNode > node2 );
    static BOOL             dialogSort2( com::sun::star::uno::Reference< drafts::com::sun::star::script::browse::XBrowseNode > node1,
            com::sun::star::uno::Reference< drafts::com::sun::star::script::browse::XBrowseNode > node2 );

public:
    void                    Init( const ::rtl::OUString& language );
    void  RequestSubEntries(  SvLBoxEntry* pRootEntry, ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >& node );
                    SFTreeListBox( Window* pParent, const ResId& rRes, ResMgr* pBasResMgr );
                    ~SFTreeListBox();

    void            UpdateEntries();

    void            ExpandAllTrees();



    SvLBoxEntry * insertEntry(String const & rText, USHORT nBitmap,
                              SvLBoxEntry * pParent,
                              bool bChildrenOnDemand,
                              std::auto_ptr< SFEntry > aUserData,
                              ::rtl::OUString factoryURL );
    SvLBoxEntry * insertEntry(String const & rText, USHORT nBitmap,
                              SvLBoxEntry * pParent,
                              bool bChildrenOnDemand,
                              std::auto_ptr< SFEntry > aUserData );
    void deleteTree( SvLBoxEntry * pEntry );
    void deleteAllTree( );
};

class InputDialog : public ModalDialog
{
private:
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;

public:
    InputDialog( Window * pParent, USHORT nMode );
                ~InputDialog();

    String      GetObjectName() const { return aEdit.GetText(); }
    void        SetObjectName( const String& rName ) { aEdit.SetText( rName ); aEdit.SetSelection( Selection( 0, rName.Len() ) );}
};

class SFEntry
{
private:
    BYTE            nType;
    bool            loaded;
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode > nodes;
    SFEntry(){}
public:
                    SFEntry( BYTE nT )              { nType = nT; loaded=false; }
                    SFEntry( BYTE nT, const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >& entryNodes ) { nType = nT; nodes = entryNodes; loaded=false; }
                    SFEntry( const SFEntry& r ) { nType = r.nType; nodes = r.nodes; loaded = r.loaded; }
    virtual         ~SFEntry() { OSL_TRACE("*** in SFEntry dtor ***" );}
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode > GetNode() { return nodes ;}

    BYTE            GetType() const                     { return nType; }
    bool            isLoaded() const                    { return loaded; }
    void            setLoaded()                         { loaded=true; }
};

class SvxScriptOrgDialog : public SfxModalDialog
{
protected:
    FixedText               aScriptsTxt;
    SFTreeListBox           aScriptsBox;

    PushButton              aRunButton;
    CancelButton            aCloseButton;
    PushButton              aCreateButton;
    PushButton              aEditButton;
    PushButton              aRenameButton;
    PushButton              aDelButton;
    HelpButton              aHelpButton;

    ::rtl::OUString         m_sLanguage;
    static Selection_hash   m_lastSelection;
    const String m_delErrStr;
    const String m_delErrTitleStr;
    const String m_delQueryStr;
    const String m_delQueryTitleStr;
    const String m_createErrStr;
    const String m_createDupStr;
    const String m_createErrTitleStr;
    const String m_renameErrStr;
    const String m_renameDupStr;
    const String m_renameErrTitleStr;

    DECL_LINK( MacroSelectHdl, SvTreeListBox * );
    DECL_LINK( MacroDoubleClickHdl, SvTreeListBox * );
    DECL_LINK( ScriptSelectHdl, SvTreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    BOOL                getBoolProperty( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProps, ::rtl::OUString& propName );
    void                CheckButtons(  ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >& node );

    void        createEntry( SvLBoxEntry* pEntry );
    void        renameEntry( SvLBoxEntry* pEntry );
    void        deleteEntry( SvLBoxEntry* pEntry );
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::script::browse::XBrowseNode >
                getBrowseNode( SvLBoxEntry* pEntry );

    void        EnableButton( Button& rButton, BOOL bEnable );
    String      getListOfChildren( ::com::sun::star::uno::Reference< drafts::com::sun::star::script::browse::XBrowseNode > node, int depth );
    void        StoreCurrentSelection();
    void        RestorePreviousSelection();
    //String                GetInfo( SbxVariable* pVar );

public:
                    // prob need another arg in the ctor
                    // to specify the language or provider
                    SvxScriptOrgDialog( Window* pParent, ResMgr* pBasResMgr,
                            ::rtl::OUString language );
                    ~SvxScriptOrgDialog();

    virtual short   Execute();

    //DECL_LINK( ActivatePageHdl, TabControl * );
};

class SvxScriptErrorDialog : public VclAbstractDialog
{
private:

    ::rtl::OUString m_sMessage;

    DECL_LINK( ShowDialog, ::rtl::OUString* );

public:

    SvxScriptErrorDialog(
        Window* parent, ::com::sun::star::uno::Any aException );

    ~SvxScriptErrorDialog();

    USHORT          Execute();
};

#endif // _SCRIPTDLG_HXX
