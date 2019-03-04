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

#include <memory>
#include <utility>

#include <sal/log.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/builderfactory.hxx>

#include <strings.hrc>
#include <bitmaps.hlst>
#include <scriptdlg.hxx>
#include <dialmgr.hxx>
#include <cfgutil.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/XBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/script/browse/theBrowseNodeFactory.hpp>
#include <com/sun/star/script/provider/ScriptErrorRaisedException.hpp>
#include <com/sun/star/script/provider/ScriptExceptionRaisedException.hpp>
#include <com/sun/star/script/provider/ScriptFrameworkErrorType.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>

#include <basic/sbx.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/treelistentry.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::script;
using namespace css::frame;
using namespace css::document;

static void ShowErrorDialog( const Any& aException )
{
    ScopedVclPtrInstance<SvxScriptErrorDialog> pDlg( aException );
    pDlg->Execute();
}

void SvxScriptOrgDialog::delUserData(weld::TreeIter& rIter)
{
    SFEntry* pUserData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(rIter).toInt64());
    if (pUserData)
    {
        delete pUserData;
        // TBD seem to get a Select event on node that is remove ( below )
        // so need to be able to detect that this node is not to be
        // processed in order to do this, setting userData to NULL ( must
        // be a better way to do this )
        m_xScriptsBox->set_id(rIter, OUString());
    }
}

void SvxScriptOrgDialog::deleteTree(weld::TreeIter& rIter)
{
    delUserData(rIter);
    std::unique_ptr<weld::TreeIter> xIter = m_xScriptsBox->make_iterator(&rIter);
    if (!m_xScriptsBox->iter_children(*xIter))
        return;

    std::unique_ptr<weld::TreeIter> xAltIter = m_xScriptsBox->make_iterator();
    bool bNextEntry;
    do
    {
        m_xScriptsBox->copy_iterator(*xIter, *xAltIter);
        bNextEntry = m_xScriptsBox->iter_next_sibling(*xAltIter);
        deleteTree(*xIter);
        m_xScriptsBox->remove(*xIter);
        m_xScriptsBox->copy_iterator(*xAltIter, *xIter);
    }
    while (bNextEntry);
}

void SvxScriptOrgDialog::deleteAllTree()
{
    std::unique_ptr<weld::TreeIter> xIter = m_xScriptsBox->make_iterator();
    if (!m_xScriptsBox->get_iter_first(*xIter))
        return;

    std::unique_ptr<weld::TreeIter> xAltIter = m_xScriptsBox->make_iterator();
    // TBD - below is a candidate for a destroyAllTrees method
    bool bNextEntry;
    do
    {
        m_xScriptsBox->copy_iterator(*xIter, *xAltIter);
        bNextEntry = m_xScriptsBox->iter_next_sibling(*xAltIter);
        deleteTree(*xIter);
        m_xScriptsBox->remove(*xIter);
        m_xScriptsBox->copy_iterator(*xAltIter, *xIter);
    }
    while (bNextEntry);
}

void SvxScriptOrgDialog::Init( const OUString& language  )
{
    m_xScriptsBox->freeze();

    deleteAllTree();

    Reference< browse::XBrowseNode > rootNode;
    Reference< XComponentContext > xCtx(
        comphelper::getProcessComponentContext() );

    Sequence< Reference< browse::XBrowseNode > > children;

    OUString userStr("user");
    OUString const shareStr("share");

    try
    {
        Reference< browse::XBrowseNodeFactory > xFac = browse::theBrowseNodeFactory::get(xCtx);

        rootNode.set( xFac->createView(
            browse::BrowseNodeFactoryViewTypes::MACROORGANIZER ) );

        if (  rootNode.is() && rootNode->hasChildNodes() )
        {
            children = rootNode->getChildNodes();
        }
    }
    catch( Exception& e )
    {
        SAL_WARN("cui.dialogs", "Exception getting root browse node from factory: " << e );
        // TODO exception handling
    }

    Reference<XModel> xDocumentModel;
    for ( sal_Int32 n = 0; n < children.getLength(); n++ )
    {
        bool app = false;
        OUString uiName = children[ n ]->getName();
        OUString factoryURL;
        if ( uiName == userStr || uiName == shareStr )
        {
            app = true;
            if ( uiName == userStr )
            {
                uiName = m_sMyMacros;
            }
            else
            {
                uiName = m_sProdMacros;
            }
        }
        else
        {
            xDocumentModel.set(getDocumentModel(xCtx, uiName ), UNO_QUERY);

            if ( xDocumentModel.is() )
            {
                Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xCtx) );

                // get the long name of the document:
                Sequence<beans::PropertyValue> moduleDescr;
                try{
                    OUString appModule = xModuleManager->identify( xDocumentModel );
                    xModuleManager->getByName(appModule) >>= moduleDescr;
                } catch(const uno::Exception&)
                    {}

                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if ( pmoduleDescr[ pos ].Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        break;
                    }
                }
            }
        }

        Reference< browse::XBrowseNode > langEntries =
            getLangNodeFromRootNode( children[ n ], language );

        insertEntry( uiName, app ? OUStringLiteral(RID_CUIBMP_HARDDISK) : OUStringLiteral(RID_CUIBMP_DOC),
            nullptr, true, std::make_unique< SFEntry >( langEntries, xDocumentModel ), factoryURL, false );
    }

    m_xScriptsBox->thaw();
}

Reference< XInterface  >
SvxScriptOrgDialog::getDocumentModel( Reference< XComponentContext > const & xCtx, OUString const & docName )
{
    Reference< XInterface > xModel;
    Reference< frame::XDesktop2 > desktop  = frame::Desktop::create(xCtx);

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    while (components->hasMoreElements())
    {
        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            OUString sTdocUrl = ::comphelper::DocumentInfo::getDocumentTitle( model );
            if( sTdocUrl == docName )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

Reference< browse::XBrowseNode >
SvxScriptOrgDialog::getLangNodeFromRootNode( Reference< browse::XBrowseNode > const & rootNode, OUString const & language )
{
    Reference< browse::XBrowseNode > langNode;

    try
    {
        Sequence < Reference< browse::XBrowseNode > > children = rootNode->getChildNodes();
        for ( sal_Int32 n = 0; n < children.getLength(); n++ )
        {
            if ( children[ n ]->getName() == language )
            {
                langNode = children[ n ];
                break;
            }
        }
    }
    catch ( Exception& )
    {
        // if getChildNodes() throws an exception we just return
        // the empty Reference
    }
    return langNode;
}

void SvxScriptOrgDialog::RequestSubEntries(const weld::TreeIter& rRootEntry, Reference< css::script::browse::XBrowseNode > const & node,
                                           Reference< XModel >& model)
{
    if (!node.is())
    {
        return;
    }

    Sequence< Reference< browse::XBrowseNode > > children;
    try
    {
        children = node->getChildNodes();
    }
    catch ( Exception& )
    {
        // if we catch an exception in getChildNodes then no entries are added
    }

    for ( sal_Int32 n = 0; n < children.getLength(); n++ )
    {
        OUString name( children[ n ]->getName() );
        if (  children[ n ]->getType() !=  browse::BrowseNodeTypes::SCRIPT)
        {
            insertEntry(name, RID_CUIBMP_LIB, &rRootEntry, true, std::make_unique<SFEntry>(children[n], model), false);
        }
        else
        {
            insertEntry(name, RID_CUIBMP_MACRO, &rRootEntry, false, std::make_unique<SFEntry>(children[n], model), false);
        }
    }
}

void SvxScriptOrgDialog::insertEntry(const OUString& rText, const OUString& rBitmap,
    const weld::TreeIter* pParent, bool bChildrenOnDemand, std::unique_ptr<SFEntry> && aUserData,
    const OUString& factoryURL, bool bSelect)
{
    if (rBitmap == RID_CUIBMP_DOC && !factoryURL.isEmpty())
    {
        OUString aImage = SvFileInformationManager::GetFileImageId(INetURLObject(factoryURL));
        insertEntry(rText, aImage, pParent, bChildrenOnDemand, std::move(aUserData), bSelect);
        return;
    }
    insertEntry(rText, rBitmap, pParent, bChildrenOnDemand, std::move(aUserData), bSelect);
}

void SvxScriptOrgDialog::insertEntry(
    const OUString& rText, const OUString& rBitmap, const weld::TreeIter* pParent,
    bool bChildrenOnDemand, std::unique_ptr<SFEntry> && aUserData, bool bSelect)
{
    std::unique_ptr<weld::TreeIter> xRetIter;
    if (bSelect)
        xRetIter = m_xScriptsBox->make_iterator();
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(aUserData.release()))); // XXX possible leak
    m_xScriptsBox->insert(pParent, -1, &rText, &sId, nullptr, nullptr, &rBitmap,
                          bChildrenOnDemand, xRetIter.get());
    if (bSelect)
    {
        m_xScriptsBox->set_cursor(*xRetIter);
        m_xScriptsBox->select(*xRetIter);
    }
}

IMPL_LINK(SvxScriptOrgDialog, ExpandingHdl, const weld::TreeIter&, rIter, bool)
{
    SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(rIter).toInt64());

    Reference< browse::XBrowseNode > node;
    Reference< XModel > model;
    if ( userData && !userData->isLoaded() )
    {
        node = userData->GetNode();
        model = userData->GetModel();
        RequestSubEntries(rIter, node, model);
        userData->setLoaded();
    }

    return true;
}

// CuiInputDialog ------------------------------------------------------------
CuiInputDialog::CuiInputDialog(weld::Window * pParent, InputDialogMode nMode)
    : GenericDialogController(pParent, "cui/ui/newlibdialog.ui", "NewLibDialog")
    , m_xEdit(m_xBuilder->weld_entry("entry"))
{
    m_xEdit->grab_focus();

    std::unique_ptr<weld::Label> xNewLibFT(m_xBuilder->weld_label("newlibft"));

    if ( nMode == InputDialogMode::NEWMACRO )
    {
        xNewLibFT->hide();
        std::unique_ptr<weld::Label> xNewMacroFT(m_xBuilder->weld_label("newmacroft"));
        xNewMacroFT->show();
        std::unique_ptr<weld::Label> xAltTitle(m_xBuilder->weld_label("altmacrotitle"));
        m_xDialog->set_title(xAltTitle->get_label());
    }
    else if ( nMode == InputDialogMode::RENAME )
    {
        xNewLibFT->hide();
        std::unique_ptr<weld::Label> xRenameFT(m_xBuilder->weld_label("renameft"));
        xRenameFT->show();
        std::unique_ptr<weld::Label> xAltTitle(m_xBuilder->weld_label("altrenametitle"));
        m_xDialog->set_title(xAltTitle->get_label());
    }
}

// ScriptOrgDialog ------------------------------------------------------------

SvxScriptOrgDialog::SvxScriptOrgDialog(weld::Window* pParent, const OUString& language)
    : SfxDialogController(pParent, "cui/ui/scriptorganizer.ui", "ScriptOrganizerDialog")
    , m_sLanguage(language)
    , m_delErrStr(CuiResId(RID_SVXSTR_DELFAILED))
    , m_delErrTitleStr(CuiResId(RID_SVXSTR_DELFAILED_TITLE))
    , m_delQueryStr(CuiResId(RID_SVXSTR_DELQUERY))
    , m_delQueryTitleStr(CuiResId(RID_SVXSTR_DELQUERY_TITLE))
    , m_createErrStr(CuiResId(RID_SVXSTR_CREATEFAILED))
    , m_createDupStr(CuiResId(RID_SVXSTR_CREATEFAILEDDUP))
    , m_createErrTitleStr(CuiResId(RID_SVXSTR_CREATEFAILED_TITLE))
    , m_renameErrStr(CuiResId(RID_SVXSTR_RENAMEFAILED))
    , m_renameErrTitleStr(CuiResId(RID_SVXSTR_RENAMEFAILED_TITLE))
    , m_sMyMacros(CuiResId(RID_SVXSTR_MYMACROS))
    , m_sProdMacros(CuiResId(RID_SVXSTR_PRODMACROS))
    , m_xScriptsBox(m_xBuilder->weld_tree_view("scripts"))
    , m_xRunButton(m_xBuilder->weld_button("ok"))
    , m_xCloseButton(m_xBuilder->weld_button("close"))
    , m_xCreateButton(m_xBuilder->weld_button("create"))
    , m_xEditButton(m_xBuilder->weld_button("edit"))
    , m_xRenameButton(m_xBuilder->weld_button("rename"))
    , m_xDelButton(m_xBuilder->weld_button("delete"))
{
    // must be a neater way to deal with the strings than as above
    // append the language to the dialog title
    OUString winTitle(m_xDialog->get_title());
    winTitle = winTitle.replaceFirst( "%MACROLANG", m_sLanguage );
    m_xDialog->set_title(winTitle);

    m_xScriptsBox->set_size_request(m_xScriptsBox->get_approximate_digit_width() * 45,
                                    m_xScriptsBox->get_height_rows(12));

    m_xScriptsBox->connect_changed( LINK( this, SvxScriptOrgDialog, ScriptSelectHdl ) );
    m_xScriptsBox->connect_expanding(LINK( this, SvxScriptOrgDialog, ExpandingHdl ) );
    m_xRunButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_xCloseButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_xRenameButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_xEditButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_xDelButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_xCreateButton->connect_clicked( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );

    m_xRunButton->set_sensitive(false);
    m_xRenameButton->set_sensitive(false);
    m_xEditButton->set_sensitive(false);
    m_xDelButton->set_sensitive(false);
    m_xCreateButton->set_sensitive(false);

    Init(m_sLanguage);
    RestorePreviousSelection();
}

SvxScriptOrgDialog::~SvxScriptOrgDialog()
{
    deleteAllTree();
}

short SvxScriptOrgDialog::run()
{
    SfxObjectShell *pDoc = SfxObjectShell::GetFirst();

    // force load of MSPs for all documents
    while ( pDoc )
    {
        Reference< provider::XScriptProviderSupplier > xSPS =
            Reference< provider::XScriptProviderSupplier >
                                        ( pDoc->GetModel(), UNO_QUERY );
        if ( xSPS.is() )
        {
            xSPS->getScriptProvider();
        }

        pDoc = SfxObjectShell::GetNext(*pDoc);
    }

    return SfxDialogController::run();
}

void SvxScriptOrgDialog::CheckButtons( Reference< browse::XBrowseNode > const & node )
{
    if ( node.is() )
    {
        if ( node->getType() == browse::BrowseNodeTypes::SCRIPT)
        {
            m_xRunButton->set_sensitive(true);
        }
        else
        {
            m_xRunButton->set_sensitive(false);
        }
        Reference< beans::XPropertySet > xProps( node, UNO_QUERY );

        if ( !xProps.is() )
        {
            m_xEditButton->set_sensitive(false);
            m_xDelButton->set_sensitive(false);
            m_xCreateButton->set_sensitive(false);
            m_xRunButton->set_sensitive(false);
            return;
        }

        OUString sName("Editable");

        if ( getBoolProperty( xProps, sName ) )
        {
            m_xEditButton->set_sensitive(true);
        }
        else
        {
            m_xEditButton->set_sensitive(false);
        }

        sName = "Deletable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_xDelButton->set_sensitive(true);
        }
        else
        {
            m_xDelButton->set_sensitive(false);
        }

        sName = "Creatable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_xCreateButton->set_sensitive(true);
        }
        else
        {
            m_xCreateButton->set_sensitive(false);
        }

        sName = "Renamable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_xRenameButton->set_sensitive(true);
        }
        else
        {
            m_xRenameButton->set_sensitive(false);
        }
    }
    else
    {
        // no node info available, disable all configurable actions
        m_xDelButton->set_sensitive(false);
        m_xCreateButton->set_sensitive(false);
        m_xEditButton->set_sensitive(false);
        m_xRunButton->set_sensitive(false);
        m_xRenameButton->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxScriptOrgDialog, ScriptSelectHdl, weld::TreeView&, void)
{
    std::unique_ptr<weld::TreeIter> xIter = m_xScriptsBox->make_iterator();
    if (!m_xScriptsBox->get_selected(xIter.get()))
        return;

    SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(*xIter).toInt64());

    Reference< browse::XBrowseNode > node;
    if (userData)
    {
        node = userData->GetNode();
        CheckButtons(node);
    }
}

IMPL_LINK(SvxScriptOrgDialog, ButtonHdl, weld::Button&, rButton, void)
{
    if ( &rButton == m_xCloseButton.get() )
    {
        StoreCurrentSelection();
        m_xDialog->response(RET_CANCEL);
    }
    if (&rButton == m_xEditButton.get() ||
        &rButton == m_xCreateButton.get() ||
        &rButton == m_xDelButton.get() ||
        &rButton == m_xRunButton.get() ||
        &rButton == m_xRenameButton.get())

    {
        std::unique_ptr<weld::TreeIter> xIter = m_xScriptsBox->make_iterator();
        if (!m_xScriptsBox->get_selected(xIter.get()))
            return;
        SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(*xIter).toInt64());
        if (!userData)
            return;

        Reference< browse::XBrowseNode > node;
        Reference< XModel > xModel;

        node = userData->GetNode();
        xModel = userData->GetModel();

        if ( !node.is() )
        {
            return;
        }

        if (&rButton == m_xRunButton.get())
        {
            OUString tmpString;
            Reference< beans::XPropertySet > xProp( node, UNO_QUERY );
            Reference< provider::XScriptProvider > mspNode;
            if( !xProp.is() )
            {
                return;
            }

            if ( xModel.is() )
            {
                Reference< XEmbeddedScripts >  xEmbeddedScripts( xModel, UNO_QUERY);
                if( !xEmbeddedScripts.is() )
                {
                    return;
                }

                if (!xEmbeddedScripts->getAllowMacroExecution())
                {
                    // Please FIXME: Show a message box if AllowMacroExecution is false
                    return;
                }
            }

            std::unique_ptr<weld::TreeIter> xParentIter = m_xScriptsBox->make_iterator(xIter.get());
            bool bParent = m_xScriptsBox->iter_parent(*xParentIter);
            while (bParent && !mspNode.is() )
            {
                SFEntry* mspUserData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(*xParentIter).toInt64());
                mspNode.set( mspUserData->GetNode() , UNO_QUERY );
                bParent = m_xScriptsBox->iter_parent(*xParentIter);
            }
            xProp->getPropertyValue("URI") >>= tmpString;
            const OUString scriptURL( tmpString );

            if ( mspNode.is() )
            {
                try
                {
                    Reference< provider::XScript > xScript(
                    mspNode->getScript( scriptURL ), UNO_QUERY_THROW );

                    const Sequence< Any > args(0);
                    Sequence< sal_Int16 > outIndex;
                    Sequence< Any > outArgs( 0 );
                    xScript->invoke( args, outIndex, outArgs );
                }
                catch ( reflection::InvocationTargetException& ite )
                {
                    ShowErrorDialog(css::uno::Any(ite));
                }
                catch ( provider::ScriptFrameworkErrorException& ite )
                {
                    ShowErrorDialog(css::uno::Any(ite));
                }
                catch ( RuntimeException& re )
                {
                    ShowErrorDialog(css::uno::Any(re));
                }
                catch ( Exception& e )
                {
                    ShowErrorDialog(css::uno::Any(e));
                }
            }
            StoreCurrentSelection();
            m_xDialog->response(RET_CANCEL);
        }
        else if ( &rButton == m_xEditButton.get() )
        {
            Reference< script::XInvocation > xInv( node, UNO_QUERY );
            if ( xInv.is() )
            {
                StoreCurrentSelection();
                m_xDialog->response(RET_CANCEL);
                Sequence< Any > args(0);
                Sequence< Any > outArgs( 0 );
                Sequence< sal_Int16 > outIndex;
                try
                {
                    // ISSUE need code to run script here
                    xInv->invoke( "Editable", args, outIndex, outArgs );
                }
                catch( Exception const & )
                {
                    css::uno::Any ex( cppu::getCaughtException() );
                    SAL_WARN("cui.dialogs", "Caught exception trying to invoke " << exceptionToString(ex) );
                }
            }
        }
        else if ( &rButton == m_xCreateButton.get() )
        {
            createEntry(*xIter);
        }
        else if ( &rButton == m_xDelButton.get() )
        {
            deleteEntry(*xIter);
        }
        else if ( &rButton == m_xRenameButton.get() )
        {
            renameEntry(*xIter);
        }
    }
}

Reference< browse::XBrowseNode > SvxScriptOrgDialog::getBrowseNode(const weld::TreeIter& rEntry)
{
    Reference< browse::XBrowseNode > node;
    SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(rEntry).toInt64());
    if (userData)
    {
        node = userData->GetNode();
    }
    return node;
}

Reference< XModel > SvxScriptOrgDialog::getModel(const weld::TreeIter& rEntry)
{
    Reference< XModel > model;
    SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(rEntry).toInt64());
    if ( userData )
    {
        model = userData->GetModel();
    }
    return model;
}

void SvxScriptOrgDialog::createEntry(weld::TreeIter& rEntry)
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode( rEntry );
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        OUString aNewName;
        OUString aNewStdName;
        InputDialogMode nMode = InputDialogMode::NEWLIB;
        if (m_xScriptsBox->get_iter_depth(rEntry) == 0)
        {
            aNewStdName = "Library" ;
        }
        else
        {
            aNewStdName = "Macro" ;
            nMode = InputDialogMode::NEWMACRO;
        }
        //do we need L10N for this? ie something like:
        //String aNewStdName( ResId( STR_STDMODULENAME ) );
        bool bValid = false;
        sal_Int32 i = 1;

        Sequence< Reference< browse::XBrowseNode > > childNodes;
        // no children => ok to create Parcel1 or Script1 without checking
        try
        {
            if( !node->hasChildNodes() )
            {
                aNewName = aNewStdName + OUString::number(i);
                bValid = true;
            }
            else
            {
                childNodes = node->getChildNodes();
            }
        }
        catch ( Exception& )
        {
            // ignore, will continue on with empty sequence
        }

        OUString extn;
        while ( !bValid )
        {
            aNewName = aNewStdName + OUString::number(i);
            bool bFound = false;
            if(childNodes.getLength() > 0 )
            {
                OUString nodeName = childNodes[0]->getName();
                sal_Int32 extnPos = nodeName.lastIndexOf( '.' );
                if(extnPos>0)
                    extn = nodeName.copy(extnPos);
            }
            for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
            {
                if (aNewName+extn == childNodes[index]->getName())
                {
                    bFound = true;
                    break;
                }
            }
            if( bFound )
            {
                i++;
            }
            else
            {
                bValid = true;
            }
        }

        CuiInputDialog aNewDlg(m_xDialog.get(), nMode);
        aNewDlg.SetObjectName(aNewName);

        do
        {
            if (aNewDlg.run() && !aNewDlg.GetObjectName().isEmpty())
            {
                OUString aUserSuppliedName = aNewDlg.GetObjectName();
                bValid = true;
                for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
                {
                    if (aUserSuppliedName+extn == childNodes[index]->getName())
                    {
                        bValid = false;
                        OUString aError = m_createErrStr + m_createDupStr;

                        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                       VclMessageType::Warning, VclButtonsType::Ok, aError));
                        xErrorBox->set_title(m_createErrTitleStr);
                        xErrorBox->run();
                        aNewDlg.SetObjectName(aNewName);
                        break;
                    }
                }
                if( bValid )
                    aNewName = aUserSuppliedName;
            }
            else
            {
                // user hit cancel or hit OK with nothing in the editbox

                return;
            }
        }
        while ( !bValid );

        // open up parent node (which ensures it's loaded)
        m_xScriptsBox->expand_row(rEntry);

        Sequence< Any > args( 1 );
        args[ 0 ] <<= aNewName;
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( "Creatable", args, outIndex, outArgs );
            Reference< browse::XBrowseNode > newNode( aResult, UNO_QUERY );
            aChildNode = newNode;

        }
        catch( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("cui.dialogs", "Caught exception trying to Create " << exceptionToString(ex) );
        }
    }
    if ( aChildNode.is() )
    {
        OUString aChildName = aChildNode->getName();

        Reference<XModel> xDocumentModel = getModel( rEntry );

        // ISSUE do we need to remove all entries for parent
        // to achieve sort? Just need to determine position
        // SvTreeListBox::InsertEntry can take position arg
        // -- Basic doesn't do this on create.
        // Suppose we could avoid this too. -> created nodes are
        // not in alphabetical order
        if ( aChildNode->getType() == browse::BrowseNodeTypes::SCRIPT )
        {
            insertEntry(aChildName, RID_CUIBMP_MACRO, &rEntry, false,
                        std::make_unique<SFEntry>(aChildNode,xDocumentModel), true);
        }
        else
        {
            insertEntry(aChildName, RID_CUIBMP_LIB, &rEntry, false,
                        std::make_unique<SFEntry>(aChildNode,xDocumentModel), true);

            // If the Parent is not loaded then set to
            // loaded, this will prevent RequestingChildren ( called
            // from vcl via RequestingChildren ) from
            // creating new ( duplicate ) children
            SFEntry* userData = reinterpret_cast<SFEntry*>(m_xScriptsBox->get_id(rEntry).toInt64());
            if ( userData &&  !userData->isLoaded() )
            {
                userData->setLoaded();
            }
        }
    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError( m_createErrStr );
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, aError));
        xErrorBox->set_title(m_createErrTitleStr);
        xErrorBox->run();
    }
}

void SvxScriptOrgDialog::renameEntry(weld::TreeIter& rEntry)
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode(rEntry);
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        OUString aNewName = node->getName();
        sal_Int32 extnPos = aNewName.lastIndexOf( '.' );
        if(extnPos>0)
        {
            aNewName = aNewName.copy(0,extnPos);
        }
        CuiInputDialog aNewDlg(m_xDialog.get(), InputDialogMode::RENAME);
        aNewDlg.SetObjectName(aNewName);

        if (!aNewDlg.run() || aNewDlg.GetObjectName().isEmpty())
            return; // user hit cancel or hit OK with nothing in the editbox

        aNewName = aNewDlg.GetObjectName();

        Sequence< Any > args( 1 );
        args[ 0 ] <<= aNewName;
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( "Renamable", args, outIndex, outArgs );
            Reference< browse::XBrowseNode > newNode( aResult, UNO_QUERY );
            aChildNode = newNode;

        }
        catch( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("cui.dialogs", "Caught exception trying to Rename " << exceptionToString(ex) );
        }
    }
    if ( aChildNode.is() )
    {
        m_xScriptsBox->set_text(rEntry, aChildNode->getName());
        m_xScriptsBox->set_cursor(rEntry);
        m_xScriptsBox->select(rEntry);

    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError( m_renameErrStr );
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, aError));
        xErrorBox->set_title(m_renameErrTitleStr);
        xErrorBox->run();
    }
}

void SvxScriptOrgDialog::deleteEntry(weld::TreeIter& rEntry)
{
    bool result = false;
    Reference< browse::XBrowseNode > node = getBrowseNode(rEntry);
    // ISSUE L10N string & can we centre list?
    OUString aQuery = m_delQueryStr + getListOfChildren( node, 0 );
    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                   VclMessageType::Question, VclButtonsType::YesNo, aQuery));
    xQueryBox->set_title(m_delQueryTitleStr);
    if (xQueryBox->run() == RET_NO)
    {
        return;
    }

    Reference< script::XInvocation > xInv( node, UNO_QUERY );
    if ( xInv.is() )
    {
        Sequence< Any > args( 0 );
        Sequence< Any > outArgs( 0 );
        Sequence< sal_Int16 > outIndex;
        try
        {
            Any aResult;
            aResult = xInv->invoke( "Deletable", args, outIndex, outArgs );
            aResult >>= result; // or do we just assume true if no exception ?
        }
        catch( Exception const & )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("cui.dialogs", "Caught exception trying to delete " << exceptionToString(ex) );
        }
    }

    if ( result )
    {
        deleteTree(rEntry);
        m_xScriptsBox->remove(rEntry);
    }
    else
    {
        //ISSUE L10N & message from exception?
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok, m_delErrStr));
        xErrorBox->set_title(m_delErrTitleStr);
        xErrorBox->run();
    }

}

bool SvxScriptOrgDialog::getBoolProperty( Reference< beans::XPropertySet > const & xProps,
                OUString const & propName )
{
    bool result = false;
    try
    {
        xProps->getPropertyValue( propName ) >>= result;
    }
    catch ( Exception& )
    {
        return result;
    }
    return result;
}

OUString SvxScriptOrgDialog::getListOfChildren( const Reference< browse::XBrowseNode >& node, int depth )
{
    OUStringBuffer result = "\n";
    for( int i=0;i<=depth;i++ )
    {
        result.append("\t");
    }
    result.append(node->getName());

    try
    {
        if ( node->hasChildNodes() )
        {
            Sequence< Reference< browse::XBrowseNode > > children
                = node->getChildNodes();
            for ( sal_Int32 n = 0; n < children.getLength(); n++ )
            {
                result.append( getListOfChildren( children[ n ] , depth+1 ) );
            }
        }
    }
    catch ( Exception& )
    {
        // ignore, will return an empty string
    }

    return result.makeStringAndClear();
}

Selection_hash SvxScriptOrgDialog::m_lastSelection;

void SvxScriptOrgDialog::StoreCurrentSelection()
{
    std::unique_ptr<weld::TreeIter> xIter = m_xScriptsBox->make_iterator();
    if (!m_xScriptsBox->get_selected(xIter.get()))
        return;
    OUString aDescription;
    bool bEntry;
    do
    {
        aDescription = m_xScriptsBox->get_text(*xIter) + aDescription;
        bEntry = m_xScriptsBox->iter_parent(*xIter);
        if (bEntry)
            aDescription = ";" + aDescription;
    }
    while (bEntry);
    OUString sDesc( aDescription );
    m_lastSelection[ m_sLanguage ] = sDesc;
}

void SvxScriptOrgDialog::RestorePreviousSelection()
{
    OUString aStoredEntry = m_lastSelection[ m_sLanguage ];
    if( aStoredEntry.isEmpty() )
        return;
    std::unique_ptr<weld::TreeIter> xEntry;
    std::unique_ptr<weld::TreeIter> xTmpEntry(m_xScriptsBox->make_iterator());
    sal_Int32 nIndex = 0;
    while (nIndex != -1)
    {
        OUString aTmp( aStoredEntry.getToken( 0, ';', nIndex ) );

        bool bTmpEntry;
        if (!xEntry)
        {
            xEntry = m_xScriptsBox->make_iterator();
            bTmpEntry = m_xScriptsBox->get_iter_first(*xEntry);
            m_xScriptsBox->copy_iterator(*xEntry, *xTmpEntry);
        }
        else
        {
            m_xScriptsBox->copy_iterator(*xEntry, *xTmpEntry);
            bTmpEntry = m_xScriptsBox->iter_children(*xTmpEntry);
        }

        while (bTmpEntry)
        {
            if (m_xScriptsBox->get_text(*xTmpEntry) == aTmp)
            {
                m_xScriptsBox->copy_iterator(*xTmpEntry, *xEntry);
                break;
            }
            bTmpEntry = m_xScriptsBox->iter_next_sibling(*xTmpEntry);
        }

        if (!bTmpEntry)
            break;

        m_xScriptsBox->expand_row(*xEntry);
    }

    if (xEntry)
        m_xScriptsBox->set_cursor(*xEntry);
}

namespace {

OUString ReplaceString(
    const OUString& source,
    const OUString& token,
    const OUString& value )
{
    sal_Int32 pos = source.indexOf( token );

    if ( pos != -1 && !value.isEmpty() )
    {
        return source.replaceAt( pos, token.getLength(), value );
    }
    else
    {
        return source;
    }
}

OUString FormatErrorString(
    const OUString& unformatted,
    const OUString& language,
    const OUString& script,
    const OUString& line,
    const OUString& type,
    const OUString& message )
{
    OUString result = unformatted.copy( 0 );

    result = ReplaceString(result, "%LANGUAGENAME", language );
    result = ReplaceString(result, "%SCRIPTNAME", script );
    result = ReplaceString(result, "%LINENUMBER", line );

    if ( !type.isEmpty() )
    {
        result += "\n\n" + CuiResId(RID_SVXSTR_ERROR_TYPE_LABEL) + " " + type;
    }

    if ( !message.isEmpty() )
    {
        result += "\n\n" + CuiResId(RID_SVXSTR_ERROR_MESSAGE_LABEL) + " " + message;
    }

    return result;
}

OUString GetErrorMessage(
    const provider::ScriptErrorRaisedException& eScriptError )
{
    OUString unformatted = CuiResId( RID_SVXSTR_ERROR_AT_LINE );

    OUString unknown("UNKNOWN");
    OUString language = unknown;
    OUString script = unknown;
    OUString line = unknown;
    OUString type = "";
    OUString message = eScriptError.Message;

    if ( !eScriptError.language.isEmpty() )
    {
        language = eScriptError.language;
    }

    if ( !eScriptError.scriptName.isEmpty() )
    {
        script = eScriptError.scriptName;
    }

    if ( !eScriptError.Message.isEmpty() )
    {
        message = eScriptError.Message;
    }
    if ( eScriptError.lineNum != -1 )
    {
        line = OUString::number( eScriptError.lineNum );
        unformatted = CuiResId( RID_SVXSTR_ERROR_AT_LINE );
    }
    else
    {
        unformatted = CuiResId( RID_SVXSTR_ERROR_RUNNING );
    }

    return FormatErrorString(
        unformatted, language, script, line, type, message );
}

OUString GetErrorMessage(
    const provider::ScriptExceptionRaisedException& eScriptException )
{
    OUString unformatted = CuiResId( RID_SVXSTR_EXCEPTION_AT_LINE );

    OUString unknown("UNKNOWN");
    OUString language = unknown;
    OUString script = unknown;
    OUString line = unknown;
    OUString type = unknown;
    OUString message = eScriptException.Message;

    if ( !eScriptException.language.isEmpty() )
    {
        language = eScriptException.language;
    }
    if ( !eScriptException.scriptName.isEmpty() )
    {
        script = eScriptException.scriptName;
    }

    if ( !eScriptException.Message.isEmpty() )
    {
        message = eScriptException.Message;
    }

    if ( eScriptException.lineNum != -1 )
    {
        line = OUString::number( eScriptException.lineNum );
        unformatted = CuiResId( RID_SVXSTR_EXCEPTION_AT_LINE );
    }
    else
    {
        unformatted = CuiResId( RID_SVXSTR_EXCEPTION_RUNNING );
    }

    if ( !eScriptException.exceptionType.isEmpty() )
    {
        type = eScriptException.exceptionType;
    }

    return FormatErrorString(
        unformatted, language, script, line, type, message );

}
OUString GetErrorMessage(
    const provider::ScriptFrameworkErrorException& sError )
{
    OUString unformatted = CuiResId( RID_SVXSTR_FRAMEWORK_ERROR_RUNNING );

    OUString language("UNKNOWN");

    OUString script("UNKNOWN");

    OUString message;

    if ( !sError.scriptName.isEmpty() )
    {
        script = sError.scriptName;
    }
    if ( !sError.language.isEmpty() )
    {
        language = sError.language;
    }
    if ( sError.errorType == provider::ScriptFrameworkErrorType::NOTSUPPORTED )
    {
        message =
            CuiResId(  RID_SVXSTR_ERROR_LANG_NOT_SUPPORTED );
        message = ReplaceString(message, "%LANGUAGENAME", language );

    }
    else
    {
        message = sError.Message;
    }
    return FormatErrorString(
        unformatted, language, script, OUString(), OUString(), message );
}

OUString GetErrorMessage( const css::uno::Any& aException )
{
    if ( aException.getValueType() ==
         cppu::UnoType<reflection::InvocationTargetException>::get())
    {
        reflection::InvocationTargetException ite;
        aException >>= ite;
        if ( ite.TargetException.getValueType() == cppu::UnoType<provider::ScriptErrorRaisedException>::get())
        {
            // Error raised by script
            provider::ScriptErrorRaisedException scriptError;
            ite.TargetException >>= scriptError;
            return GetErrorMessage( scriptError );
        }
        else if ( ite.TargetException.getValueType() == cppu::UnoType<provider::ScriptExceptionRaisedException>::get())
        {
            // Exception raised by script
            provider::ScriptExceptionRaisedException scriptException;
            ite.TargetException >>= scriptException;
            return GetErrorMessage( scriptException );
        }
        else
        {
            // Unknown error, shouldn't happen
            // OSL_ASSERT(...)
        }

    }
    else if ( aException.getValueType() == cppu::UnoType<provider::ScriptFrameworkErrorException>::get())
    {
        // A Script Framework error has occurred
        provider::ScriptFrameworkErrorException sfe;
        aException >>= sfe;
        return GetErrorMessage( sfe );

    }
    // unknown exception
    auto msg = aException.getValueTypeName();
    Exception e;
    if ( (aException >>= e) && !e.Message.isEmpty() )
    {
        msg += ": " + e.Message;
    }
    return msg;
}

}

SvxScriptErrorDialog::SvxScriptErrorDialog( css::uno::Any const & aException )
    : m_sMessage()
{
    SolarMutexGuard aGuard;
    m_sMessage = GetErrorMessage( aException );
}

SvxScriptErrorDialog::~SvxScriptErrorDialog()
{
}

short SvxScriptErrorDialog::Execute()
{
    // Show Error dialog asynchronously

    // Pass a copy of the message to the ShowDialog method as the
    // SvxScriptErrorDialog may be deleted before ShowDialog is called
    Application::PostUserEvent(
        LINK( this, SvxScriptErrorDialog, ShowDialog ),
        new OUString( m_sMessage ) );

    return 0;
}

IMPL_STATIC_LINK( SvxScriptErrorDialog, ShowDialog, void*, p, void )
{
    OUString* pMessage = static_cast<OUString*>(p);
    OUString message;

    if ( pMessage && !pMessage->isEmpty() )
    {
        message = *pMessage;
    }
    else
    {
        message = CuiResId( RID_SVXSTR_ERROR_TITLE );
    }

    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                              VclMessageType::Warning, VclButtonsType::Ok, message));
    xBox->set_title(CuiResId(RID_SVXSTR_ERROR_TITLE));
    xBox->run();

    delete pMessage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
