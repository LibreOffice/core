/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scriptdlg.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:40:16 $
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
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/frame/XModel.hpp>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    Image m_hdImage;
    Image m_hdImage_hc;
    Image m_libImage;
    Image m_libImage_hc;
    Image m_macImage;
    Image m_macImage_hc;
    Image m_docImage;
    Image m_docImage_hc;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;

    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
        getLangNodeFromRootNode( ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& root, ::rtl::OUString& language );
    void delUserData( SvLBoxEntry* pEntry );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );

protected:
    void                    ExpandTree( SvLBoxEntry* pRootEntry );
    virtual void            RequestingChilds( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    SvLBoxEntry*            FindEntry( SvLBoxEntry* pParent, const String& rText, BYTE nType );
    virtual long            ExpandingHdl();
    static BOOL             dialogSort1( com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node1,
            com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node2 );
    static BOOL             dialogSort2( com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node1,
            com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node2 );

public:
    void                    Init( const ::rtl::OUString& language );
    void  RequestSubEntries(  SvLBoxEntry* pRootEntry, ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node );
                    SFTreeListBox( Window* pParent, const ResId& rRes );
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
        ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > nodes;
    SFEntry(){}
public:
                    SFEntry( BYTE nT )              { nType = nT; loaded=false; }
                    SFEntry( BYTE nT, const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& entryNodes ) { nType = nT; nodes = entryNodes; loaded=false; }
                    SFEntry( const SFEntry& r ) { nType = r.nType; nodes = r.nodes; loaded = r.loaded; }
    virtual         ~SFEntry() {}
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > GetNode() { return nodes ;}

    BYTE            GetType() const                     { return nType; }
    bool            isLoaded() const                    { return loaded; }
    void            setLoaded()                         { loaded=true; }
};

class SVX_DLLPUBLIC SvxScriptOrgDialog : public SfxModalDialog
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
    void                CheckButtons(  ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node );

    void        createEntry( SvLBoxEntry* pEntry );
    void        renameEntry( SvLBoxEntry* pEntry );
    void        deleteEntry( SvLBoxEntry* pEntry );
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
                getBrowseNode( SvLBoxEntry* pEntry );

    void        EnableButton( Button& rButton, BOOL bEnable );
    String      getListOfChildren( ::com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node, int depth );
    void        StoreCurrentSelection();
    void        RestorePreviousSelection();
    //String                GetInfo( SbxVariable* pVar );

public:
                    // prob need another arg in the ctor
                    // to specify the language or provider
                    SvxScriptOrgDialog( Window* pParent, ::rtl::OUString language );
                    ~SvxScriptOrgDialog();

    virtual short   Execute();

    //DECL_LINK( ActivatePageHdl, TabControl * );
};

class SVX_DLLPUBLIC SvxScriptErrorDialog : public VclAbstractDialog
{
private:

    ::rtl::OUString m_sMessage;

    DECL_LINK( ShowDialog, ::rtl::OUString* );

public:

    SvxScriptErrorDialog(
        Window* parent, ::com::sun::star::uno::Any aException );

    ~SvxScriptErrorDialog();

    short           Execute();
};

#endif // _SCRIPTDLG_HXX
