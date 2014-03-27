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

#ifndef INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX

#include <memory>
#include <svtools/treelistbox.hxx>
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
#define OBJTYPE_SCRIPTCONTAINER 3L
#define OBJTYPE_SFROOT          4L

#define INPUTMODE_NEWLIB        1
#define INPUTMODE_NEWMACRO      2
#define INPUTMODE_RENAME        3

typedef ::boost::unordered_map < OUString, OUString ,
    OUStringHash, ::std::equal_to< OUString > > Selection_hash;

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
    OUString m_sMyMacros;
    OUString m_sProdMacros;

    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
        getLangNodeFromRootNode( ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& root, OUString& language );
    void delUserData( SvTreeListEntry* pEntry );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, OUString& docName );

protected:
    void                    ExpandTree( SvTreeListEntry* pRootEntry );
    virtual void            RequestingChildren( SvTreeListEntry* pParent ) SAL_OVERRIDE;
    virtual void            ExpandedHdl() SAL_OVERRIDE;
    virtual bool            ExpandingHdl() SAL_OVERRIDE;
public:
    void                    Init( const OUString& language );
    void  RequestSubEntries(  SvTreeListEntry* pRootEntry, ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node,
                              ::com::sun::star::uno::Reference< com::sun::star::frame::XModel>& model  );
                    SFTreeListBox(Window* pParent);
                    ~SFTreeListBox();

    void            ExpandAllTrees();



    SvTreeListEntry * insertEntry(OUString const & rText, sal_uInt16 nBitmap,
                              SvTreeListEntry * pParent,
                              bool bChildrenOnDemand,
                              std::auto_ptr< SFEntry > aUserData,
                              const OUString& factoryURL );
    SvTreeListEntry * insertEntry(OUString const & rText, sal_uInt16 nBitmap,
                              SvTreeListEntry * pParent,
                              bool bChildrenOnDemand,
                              std::auto_ptr< SFEntry > aUserData );
    void deleteTree( SvTreeListEntry * pEntry );
    void deleteAllTree( );
};

class CuiInputDialog : public ModalDialog
{
private:
    Edit* m_pEdit;
public:
    CuiInputDialog(Window * pParent, sal_uInt16 nMode);

    OUString GetObjectName() const { return m_pEdit->GetText(); }
    void SetObjectName(const OUString& rName)
    {
        m_pEdit->SetText( rName );
        m_pEdit->SetSelection( Selection( 0, rName.getLength() ) );
    }
};

class SFEntry
{
private:
    sal_uInt8       nType;
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
    SFTreeListBox*          m_pScriptsBox;

    PushButton*             m_pRunButton;
    CloseButton*            m_pCloseButton;
    PushButton*             m_pCreateButton;
    PushButton*             m_pEditButton;
    PushButton*             m_pRenameButton;
    PushButton*             m_pDelButton;

    OUString         m_sLanguage;
    static Selection_hash   m_lastSelection;
    const OUString m_delErrStr;
    const OUString m_delErrTitleStr;
    const OUString m_delQueryStr;
    const OUString m_delQueryTitleStr;
    const OUString m_createErrStr;
    const OUString m_createDupStr;
    const OUString m_createErrTitleStr;
    const OUString m_renameErrStr;
    const OUString m_renameDupStr;
    const OUString m_renameErrTitleStr;

    DECL_LINK( MacroSelectHdl, SvTreeListBox * );
    DECL_LINK( ScriptSelectHdl, SvTreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    sal_Bool                getBoolProperty( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xProps, OUString& propName );
    void                CheckButtons(  ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >& node );


    void        createEntry( SvTreeListEntry* pEntry );
    void        renameEntry( SvTreeListEntry* pEntry );
    void        deleteEntry( SvTreeListEntry* pEntry );
    ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode >
                getBrowseNode( SvTreeListEntry* pEntry );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel( SvTreeListEntry* pEntry );
    OUString    getListOfChildren( ::com::sun::star::uno::Reference< com::sun::star::script::browse::XBrowseNode > node, int depth );
    void        StoreCurrentSelection();
    void        RestorePreviousSelection();

public:
                    // prob need another arg in the ctor
                    // to specify the language or provider
                    SvxScriptOrgDialog( Window* pParent, const OUString& language );
                    ~SvxScriptOrgDialog();

    virtual short   Execute() SAL_OVERRIDE;

};

class SvxScriptErrorDialog : public VclAbstractDialog
{
private:

    OUString m_sMessage;

    DECL_LINK( ShowDialog, OUString* );

public:

    SvxScriptErrorDialog(
        Window* parent, ::com::sun::star::uno::Any aException );

    ~SvxScriptErrorDialog();

    short           Execute() SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_SCRIPTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
