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

#include <boost/unordered_map.hpp>

#define OBJTYPE_BASICMANAGER    1L
#define OBJTYPE_METHOD          2L
#define OBJTYPE_SCRIPTCONTAINER     3L
#define OBJTYPE_SFROOT      4L

#define INPUTMODE_NEWLIB        1
#define INPUTMODE_NEWMACRO      2
#define INPUTMODE_RENAME        3

typedef ::boost::unordered_map < ::rtl::OUString, ::rtl::OUString ,
    ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > Selection_hash;

class SFEntry;

class SFTreeListBox : public SvTreeListBox
{
   friend class SvxScriptOrgDialog;
private:
    sal_uInt16          nMode;
    Image m_hdImage;
    Image m_libImage;
    Image m_macImage;
    Image m_docImage;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;

    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
        getLangNodeFromRootNode( ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& root, ::rtl::OUString& language );
    void delUserData( SvLBoxEntry* pEntry );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );

protected:
    void                    ExpandTree( SvLBoxEntry* pRootEntry );
    virtual void            RequestingChildren( SvLBoxEntry* pParent );
    virtual void            ExpandedHdl();
    virtual long            ExpandingHdl();
public:
    void                    Init( const ::rtl::OUString& language );
    void  RequestSubEntries(  SvLBoxEntry* pRootEntry, ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node,
                              ::com::sun::star::uno::Reference< com::sun::star::frame::XModel>& model  );
                    SFTreeListBox( Window* pParent, const ResId& rRes );
                    ~SFTreeListBox();

    void            ExpandAllTrees();



    SvLBoxEntry * insertEntry(String const & rText, sal_uInt16 nBitmap,
                              SvLBoxEntry * pParent,
                              bool bChildrenOnDemand,
                              std::auto_ptr< SFEntry > aUserData,
                              ::rtl::OUString factoryURL );
    SvLBoxEntry * insertEntry(String const & rText, sal_uInt16 nBitmap,
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
    InputDialog( Window * pParent, sal_uInt16 nMode );
                ~InputDialog();

    String      GetObjectName() const { return aEdit.GetText(); }
    void        SetObjectName( const String& rName ) { aEdit.SetText( rName ); aEdit.SetSelection( Selection( 0, rName.Len() ) );}
};

class SFEntry
{
private:
    sal_uInt8           nType;
    bool            loaded;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > nodes;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > model;
    SFEntry(){}
public:
                    SFEntry( sal_uInt8 nT )             { nType = nT; loaded=false; }
                    SFEntry( sal_uInt8 nT,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& entryNodes ,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& entryModel) { nType = nT; nodes = entryNodes; loaded=false; model = entryModel; }
                    SFEntry( const SFEntry& r ) { nType = r.nType; nodes = r.nodes; loaded = r.loaded; }
    virtual         ~SFEntry() {}
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > GetNode() { return nodes ;}
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > GetModel() { return model ;};
    sal_uInt8           GetType() const                     { return nType; }
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
    DECL_LINK( ScriptSelectHdl, SvTreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    sal_Bool                getBoolProperty( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProps, ::rtl::OUString& propName );
    void                CheckButtons(  ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node );


    void        createEntry( SvLBoxEntry* pEntry );
    void        renameEntry( SvLBoxEntry* pEntry );
    void        deleteEntry( SvLBoxEntry* pEntry );
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
                getBrowseNode( SvLBoxEntry* pEntry );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel( SvLBoxEntry* pEntry );
    String      getListOfChildren( ::com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node, int depth );
    void        StoreCurrentSelection();
    void        RestorePreviousSelection();

public:
                    // prob need another arg in the ctor
                    // to specify the language or provider
                    SvxScriptOrgDialog( Window* pParent, ::rtl::OUString language );
                    ~SvxScriptOrgDialog();

    virtual short   Execute();

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

    short           Execute();
};

#endif // _SCRIPTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
