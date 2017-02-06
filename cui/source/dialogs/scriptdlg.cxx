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

#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>
#include <vcl/layout.hxx>
#include <vcl/builderfactory.hxx>
#include <o3tl/make_unique.hxx>
#include <osl/mutex.hxx>

#include <cuires.hrc>
#include "scriptdlg.hxx"
#include <dialmgr.hxx>
#include "selector.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
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
#include <comphelper/uno3.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>

#include <basic/sbx.hxx>
#include <svtools/imagemgr.hxx>
#include "svtools/treelistentry.hxx"
#include <tools/urlobj.hxx>
#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::script;
using namespace css::frame;
using namespace css::document;

void ShowErrorDialog( const Any& aException )
{
    ScopedVclPtrInstance<SvxScriptErrorDialog> pDlg( aException );
    pDlg->Execute();
}

SFTreeListBox::SFTreeListBox(vcl::Window* pParent)
    : SvTreeListBox(pParent)
    , m_hdImage(BitmapEx(CUI_RES(RID_CUIBMP_HARDDISK)))
    , m_libImage(BitmapEx(CUI_RES(RID_CUIBMP_LIB)))
    , m_macImage(BitmapEx(CUI_RES(RID_CUIBMP_MACRO)))
    , m_docImage(BitmapEx(CUI_RES(RID_CUIBMP_DOC)))
    , m_sMyMacros(CUI_RESSTR(RID_SVXSTR_MYMACROS))
    , m_sProdMacros(CUI_RESSTR(RID_SVXSTR_PRODMACROS))
{
    SetSelectionMode( SelectionMode::Single );

    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL |
                   WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HIDESELECTION |
                   WB_HASLINES | WB_HASLINESATROOT | WB_TABSTOP );
    SetNodeDefaultImages();
}

VCL_BUILDER_FACTORY(SFTreeListBox)

SFTreeListBox::~SFTreeListBox()
{
    disposeOnce();
}

void SFTreeListBox::dispose()
{
    deleteAllTree();
    SvTreeListBox::dispose();
}

void SFTreeListBox::delUserData( SvTreeListEntry* pEntry )
{
    if ( pEntry )
    {
        SFEntry* pUserData = static_cast<SFEntry*>(pEntry->GetUserData());
        if ( pUserData )
        {
            delete pUserData;
            // TBD seem to get a Select event on node that is remove ( below )
            // so need to be able to detect that this node is not to be
            // processed in order to do this, setting userData to NULL ( must
            // be a better way to do this )
            pUserData = nullptr;
            pEntry->SetUserData( pUserData );
        }
    }
}

void SFTreeListBox::deleteTree( SvTreeListEntry* pEntry )
{

    delUserData( pEntry );
    pEntry = FirstChild( pEntry );
    while ( pEntry )
    {
        SvTreeListEntry* pNextEntry = NextSibling( pEntry );
        deleteTree( pEntry );
        GetModel()->Remove( pEntry );
        pEntry = pNextEntry;
    }
}

void SFTreeListBox::deleteAllTree()
{
    SvTreeListEntry* pEntry =  GetEntry( 0 );

    // TBD - below is a candidate for a destroyAllTrees method
    if ( pEntry )
    {
        while ( pEntry )
        {
            SvTreeListEntry* pNextEntry = NextSibling( pEntry ) ;
            deleteTree( pEntry );
            GetModel()->Remove( pEntry );
            pEntry = pNextEntry;
        }
    }
}

void SFTreeListBox::Init( const OUString& language  )
{
    SetUpdateMode( false );

    deleteAllTree();

    Reference< browse::XBrowseNode > rootNode;
    Reference< XComponentContext > xCtx(
        comphelper::getProcessComponentContext() );

    Sequence< Reference< browse::XBrowseNode > > children;

    OUString userStr("user");
    OUString shareStr("share");

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
        SAL_WARN("cui.dialogs", "Exception getting root browse node from factory: " << e.Message );
        // TODO exception handling
    }

    Reference<XModel> xDocumentModel;
    for ( sal_Int32 n = 0; n < children.getLength(); n++ )
    {
        bool app = false;
        OUString uiName = children[ n ]->getName();
        OUString factoryURL;
        if ( uiName.equals( userStr ) || uiName.equals( shareStr ) )
        {
            app = true;
            if ( uiName.equals( userStr ) )
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

        OUString lang( language );
        Reference< browse::XBrowseNode > langEntries =
            getLangNodeFromRootNode( children[ n ], lang );

        insertEntry( uiName, app ? RID_CUIBMP_HARDDISK : RID_CUIBMP_DOC,
            nullptr, true, o3tl::make_unique< SFEntry >( OBJTYPE_SFROOT, langEntries, xDocumentModel ), factoryURL );
    }

    SetUpdateMode( true );
}

Reference< XInterface  >
SFTreeListBox::getDocumentModel( Reference< XComponentContext >& xCtx, OUString& docName )
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
            if( sTdocUrl.equals( docName ) )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

Reference< browse::XBrowseNode >
SFTreeListBox::getLangNodeFromRootNode( Reference< browse::XBrowseNode >& rootNode, OUString& language )
{
    Reference< browse::XBrowseNode > langNode;

    try
    {
        Sequence < Reference< browse::XBrowseNode > > children = rootNode->getChildNodes();
        for ( sal_Int32 n = 0; n < children.getLength(); n++ )
        {
            if ( children[ n ]->getName().equals( language ) )
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

void SFTreeListBox:: RequestSubEntries( SvTreeListEntry* pRootEntry, Reference< css::script::browse::XBrowseNode >& node,
                                       Reference< XModel >& model )
{
    if (! node.is() )
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
            insertEntry(name, RID_CUIBMP_LIB, pRootEntry, true, o3tl::make_unique< SFEntry >( OBJTYPE_SCRIPTCONTAINER, children[ n ],model));
        }
        else
        {
            insertEntry(name, RID_CUIBMP_MACRO, pRootEntry, false, o3tl::make_unique< SFEntry >( OBJTYPE_METHOD, children[ n ],model));
        }
    }
}

bool SFTreeListBox::ExpandingHdl()
{
    return true;
}

SvTreeListEntry * SFTreeListBox::insertEntry(
    OUString const & rText, sal_uInt16 nBitmap, SvTreeListEntry * pParent,
    bool bChildrenOnDemand, std::unique_ptr< SFEntry > && aUserData, const OUString& factoryURL )
{
    SvTreeListEntry * p;
    if( nBitmap == RID_CUIBMP_DOC && !factoryURL.isEmpty() )
    {
        Image aImage = SvFileInformationManager::GetFileImage( INetURLObject(factoryURL) );
        p = InsertEntry(
            rText, aImage, aImage, pParent, bChildrenOnDemand, TREELIST_APPEND,
            aUserData.release()); // XXX possible leak
    }
    else
    {
        p = insertEntry( rText, nBitmap, pParent, bChildrenOnDemand, std::move(aUserData) );
    }
    return p;
}

SvTreeListEntry * SFTreeListBox::insertEntry(
    OUString const & rText, sal_uInt16 nBitmap, SvTreeListEntry * pParent,
    bool bChildrenOnDemand, std::unique_ptr< SFEntry > && aUserData )
{
    Image aImage;
    if( nBitmap == RID_CUIBMP_HARDDISK )
    {
        aImage = m_hdImage;
    }
    else if( nBitmap == RID_CUIBMP_LIB )
    {
        aImage = m_libImage;
    }
    else if( nBitmap == RID_CUIBMP_MACRO )
    {
        aImage = m_macImage;
    }
    else if( nBitmap == RID_CUIBMP_DOC )
    {
        aImage = m_docImage;
    }
    SvTreeListEntry * p = InsertEntry(
        rText, aImage, aImage, pParent, bChildrenOnDemand, TREELIST_APPEND,
        aUserData.release()); // XXX possible leak
   return p;
}

void SFTreeListBox::RequestingChildren( SvTreeListEntry* pEntry )
{
    SFEntry* userData = nullptr;
    if ( !pEntry )
    {
        return;
    }
    userData = static_cast<SFEntry*>(pEntry->GetUserData());

    Reference< browse::XBrowseNode > node;
    Reference< XModel > model;
    if ( userData && !userData->isLoaded() )
    {
        node = userData->GetNode();
        model = userData->GetModel();
        RequestSubEntries( pEntry, node, model );
        userData->setLoaded();
    }
}

void SFTreeListBox::ExpandedHdl()
{
}


// CuiInputDialog ------------------------------------------------------------

CuiInputDialog::CuiInputDialog(vcl::Window * pParent, InputDialogMode nMode )
    : ModalDialog(pParent, "NewLibDialog",
        "cui/ui/newlibdialog.ui")
{
    get(m_pEdit, "entry");
    m_pEdit->GrabFocus();

    FixedText *pNewLibFT = get<FixedText>("newlibft");

    if ( nMode == InputDialogMode::NEWMACRO )
    {
        pNewLibFT->Hide();
        FixedText *pNewMacroFT = get<FixedText>("newmacroft");
        pNewMacroFT->Show();
        SetText(get<FixedText>("altmacrotitle")->GetText());
    }
    else if ( nMode == InputDialogMode::RENAME )
    {
        pNewLibFT->Hide();
        FixedText *pRenameFT = get<FixedText>("renameft");
        pRenameFT->Show();
        SetText(get<FixedText>("altrenametitle")->GetText());
    }
}

CuiInputDialog::~CuiInputDialog()
{
    disposeOnce();
}

void CuiInputDialog::dispose()
{
    m_pEdit.clear();
    ModalDialog::dispose();
}


// ScriptOrgDialog ------------------------------------------------------------

SvxScriptOrgDialog::SvxScriptOrgDialog( vcl::Window* pParent, const OUString& language )
    : SfxModalDialog(pParent, "ScriptOrganizerDialog",
        "cui/ui/scriptorganizer.ui")
    , m_sLanguage(language)
    , m_delErrStr(CUI_RESSTR(RID_SVXSTR_DELFAILED))
    , m_delErrTitleStr(CUI_RESSTR(RID_SVXSTR_DELFAILED_TITLE))
    , m_delQueryStr(CUI_RES(RID_SVXSTR_DELQUERY))
    , m_delQueryTitleStr(CUI_RESSTR(RID_SVXSTR_DELQUERY_TITLE))
    , m_createErrStr(CUI_RESSTR(RID_SVXSTR_CREATEFAILED))
    , m_createDupStr(CUI_RESSTR(RID_SVXSTR_CREATEFAILEDDUP))
    , m_createErrTitleStr(CUI_RESSTR(RID_SVXSTR_CREATEFAILED_TITLE))
    , m_renameErrStr(CUI_RESSTR(RID_SVXSTR_RENAMEFAILED))
    , m_renameErrTitleStr(CUI_RESSTR(RID_SVXSTR_RENAMEFAILED_TITLE))
{
    get(m_pScriptsBox, "scripts");
    get(m_pRunButton, "run");
    get(m_pCloseButton, "close");
    get(m_pCreateButton, "create");
    get(m_pEditButton, "edit");
    get(m_pRenameButton, "rename");
    get(m_pDelButton, "delete");
    // must be a neater way to deal with the strings than as above
    // append the language to the dialog title
    OUString winTitle( GetText() );
    winTitle = winTitle.replaceFirst( "%MACROLANG", m_sLanguage );
    SetText( winTitle );

    m_pScriptsBox->SetSelectHdl( LINK( this, SvxScriptOrgDialog, ScriptSelectHdl ) );
    m_pRunButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_pCloseButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_pRenameButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_pEditButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_pDelButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );
    m_pCreateButton->SetClickHdl( LINK( this, SvxScriptOrgDialog, ButtonHdl ) );

    m_pRunButton->Disable();
    m_pRenameButton->Disable();
    m_pEditButton->Disable();
    m_pDelButton->Disable();
    m_pCreateButton->Disable();

    m_pScriptsBox->Init( m_sLanguage );
    RestorePreviousSelection();
}

SvxScriptOrgDialog::~SvxScriptOrgDialog()
{
    disposeOnce();
}

void SvxScriptOrgDialog::dispose()
{
    // clear the SelectHdl so that it isn't called during the dtor
    m_pScriptsBox->SetSelectHdl( Link<SvTreeListBox*,void>() );
    m_pScriptsBox.clear();
    m_pRunButton.clear();
    m_pCloseButton.clear();
    m_pCreateButton.clear();
    m_pEditButton.clear();
    m_pRenameButton.clear();
    m_pDelButton.clear();
    SfxModalDialog::dispose();
};

short SvxScriptOrgDialog::Execute()
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
            Reference< provider::XScriptProvider > ScriptProvider =
            xSPS->getScriptProvider();
        }

        pDoc = SfxObjectShell::GetNext(*pDoc);
    }

    return ModalDialog::Execute();
}

void SvxScriptOrgDialog::CheckButtons( Reference< browse::XBrowseNode >& node )
{
    if ( node.is() )
    {
        if ( node->getType() == browse::BrowseNodeTypes::SCRIPT)
        {
            m_pRunButton->Enable();
        }
        else
        {
            m_pRunButton->Disable();
        }
        Reference< beans::XPropertySet > xProps( node, UNO_QUERY );

        if ( !xProps.is() )
        {
            m_pEditButton->Disable();
            m_pDelButton->Disable();
            m_pCreateButton->Disable();
            m_pRunButton->Disable();
            return;
        }

        OUString sName("Editable");

        if ( getBoolProperty( xProps, sName ) )
        {
            m_pEditButton->Enable();
        }
        else
        {
            m_pEditButton->Disable();
        }

        sName = "Deletable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_pDelButton->Enable();
        }
        else
        {
            m_pDelButton->Disable();
        }

        sName = "Creatable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_pCreateButton->Enable();
        }
        else
        {
            m_pCreateButton->Disable();
        }

        sName = "Renamable";

        if ( getBoolProperty( xProps, sName ) )
        {
            m_pRenameButton->Enable();
        }
        else
        {
            m_pRenameButton->Disable();
        }
    }
    else
    {
        // no node info available, disable all configurable actions
        m_pDelButton->Disable();
        m_pCreateButton->Disable();
        m_pEditButton->Disable();
        m_pRunButton->Disable();
        m_pRenameButton->Disable();
    }
}

IMPL_LINK( SvxScriptOrgDialog, ScriptSelectHdl, SvTreeListBox *, pBox, void )
{
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
    {
        return;
    }

    SvTreeListEntry* pEntry = pBox->GetHdlEntry();

    SFEntry* userData = nullptr;
    if ( !pEntry )
    {
        return;
    }
    userData = static_cast<SFEntry*>(pEntry->GetUserData());

    Reference< browse::XBrowseNode > node;
    if ( userData )
    {
              node = userData->GetNode();
        CheckButtons( node );
    }
}

IMPL_LINK( SvxScriptOrgDialog, ButtonHdl, Button *, pButton, void )
{
    if ( pButton == m_pCloseButton )
    {
        StoreCurrentSelection();
        EndDialog();
    }
    if ( pButton == m_pEditButton ||
            pButton == m_pCreateButton ||
            pButton == m_pDelButton ||
            pButton == m_pRunButton ||
            pButton == m_pRenameButton )

    {
        if ( m_pScriptsBox->IsSelected( m_pScriptsBox->GetHdlEntry() ) )
        {
            SvTreeListEntry* pEntry = m_pScriptsBox->GetHdlEntry();
            SFEntry* userData = nullptr;
            if ( !pEntry )
            {
                return;
            }
            userData = static_cast<SFEntry*>(pEntry->GetUserData());
            if ( userData )
            {
                Reference< browse::XBrowseNode > node;
                Reference< XModel > xModel;

                node = userData->GetNode();
                xModel = userData->GetModel();

                if ( !node.is() )
                {
                    return;
                }

                if ( pButton == m_pRunButton )
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


                    SvTreeListEntry* pParent = m_pScriptsBox->GetParent( pEntry );
                    while ( pParent && !mspNode.is() )
                    {
                        SFEntry* mspUserData = static_cast<SFEntry*>(pParent->GetUserData());
                        mspNode.set( mspUserData->GetNode() , UNO_QUERY );
                        pParent = m_pScriptsBox->GetParent( pParent );
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
                            Any aRet;
                            Sequence< sal_Int16 > outIndex;
                            Sequence< Any > outArgs( 0 );
                            aRet = xScript->invoke( args, outIndex, outArgs );
                        }
                        catch ( reflection::InvocationTargetException& ite )
                        {
                            css::uno::Any a = makeAny(ite);
                            ShowErrorDialog(a);
                        }
                        catch ( provider::ScriptFrameworkErrorException& ite )
                        {
                            css::uno::Any a = makeAny(ite);
                            ShowErrorDialog(a);
                        }
                        catch ( RuntimeException& re )
                        {
                            css::uno::Any a = makeAny(re);
                            ShowErrorDialog(a);
                        }
                        catch ( Exception& e )
                        {
                            css::uno::Any a = makeAny(e);
                            ShowErrorDialog(a);
                        }
                    }
                    StoreCurrentSelection();
                    EndDialog();
                }
                else if ( pButton == m_pEditButton )
                {
                    Reference< script::XInvocation > xInv( node, UNO_QUERY );
                    if ( xInv.is() )
                    {
                        StoreCurrentSelection();
                        EndDialog();
                        Sequence< Any > args(0);
                        Sequence< Any > outArgs( 0 );
                        Sequence< sal_Int16 > outIndex;
                        try
                        {
                            // ISSUE need code to run script here
                            xInv->invoke( "Editable", args, outIndex, outArgs );
                        }
                        catch( Exception& e )
                        {
                            SAL_WARN("cui.dialogs", "Caught exception trying to invoke " << e.Message );
                        }
                    }
                }
                else if ( pButton == m_pCreateButton )
                {
                    createEntry( pEntry );
                }
                else if ( pButton == m_pDelButton )
                {
                    deleteEntry( pEntry );
                }
                else if ( pButton == m_pRenameButton )
                {
                    renameEntry( pEntry );
                }
            }
        }
    }
}

Reference< browse::XBrowseNode > SvxScriptOrgDialog::getBrowseNode( SvTreeListEntry* pEntry )
{
    Reference< browse::XBrowseNode > node;
    if ( pEntry )
    {
        SFEntry* userData = static_cast<SFEntry*>(pEntry->GetUserData());
        if ( userData )
        {
            node = userData->GetNode();
        }
    }

    return node;
}

Reference< XModel > SvxScriptOrgDialog::getModel( SvTreeListEntry* pEntry )
{
    Reference< XModel > model;
    if ( pEntry )
    {
        SFEntry* userData = static_cast<SFEntry*>(pEntry->GetUserData());
        if ( userData )
        {
            model = userData->GetModel();
        }
    }

    return model;
}

void SvxScriptOrgDialog::createEntry( SvTreeListEntry* pEntry )
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        OUString aNewName;
        OUString aNewStdName;
        InputDialogMode nMode = InputDialogMode::NEWLIB;
        if( m_pScriptsBox->GetModel()->GetDepth( pEntry ) == 0 )
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

        ScopedVclPtrInstance< CuiInputDialog > xNewDlg( static_cast<vcl::Window*>(this), nMode );
        xNewDlg->SetObjectName( aNewName );

        do
        {
            if ( xNewDlg->Execute() && !xNewDlg->GetObjectName().isEmpty() )
            {
                OUString aUserSuppliedName = xNewDlg->GetObjectName();
                bValid = true;
                for( sal_Int32 index = 0; index < childNodes.getLength(); index++ )
                {
                    if (aUserSuppliedName+extn == childNodes[index]->getName())
                    {
                        bValid = false;
                        OUString aError = m_createErrStr + m_createDupStr;
                        ScopedVclPtrInstance< MessageDialog > aErrorBox(static_cast<vcl::Window*>(this), aError);
                        aErrorBox->SetText( m_createErrTitleStr );
                        aErrorBox->Execute();
                        xNewDlg->SetObjectName( aNewName );
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
        m_pScriptsBox->RequestingChildren( pEntry );

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
        catch( Exception& e )
        {
            SAL_WARN("cui.dialogs", "Caught exception trying to Create " << e.Message );
        }
    }
    if ( aChildNode.is() )
    {
        OUString aChildName = aChildNode->getName();
        SvTreeListEntry* pNewEntry = nullptr;

        Reference<XModel> xDocumentModel = getModel( pEntry );

        // ISSUE do we need to remove all entries for parent
        // to achieve sort? Just need to determine position
        // SvTreeListBox::InsertEntry can take position arg
        // -- Basic doesn't do this on create.
        // Suppose we could avoid this too. -> created nodes are
        // not in alphabetical order
        if ( aChildNode->getType() == browse::BrowseNodeTypes::SCRIPT )
        {
            pNewEntry = m_pScriptsBox->insertEntry( aChildName,
                    RID_CUIBMP_MACRO, pEntry, false, o3tl::make_unique< SFEntry >( OBJTYPE_METHOD, aChildNode,xDocumentModel ) );
        }
        else
        {
            pNewEntry = m_pScriptsBox->insertEntry( aChildName,
                RID_CUIBMP_LIB, pEntry, false, o3tl::make_unique< SFEntry >( OBJTYPE_SCRIPTCONTAINER, aChildNode,xDocumentModel ) );

            // If the Parent is not loaded then set to
            // loaded, this will prevent RequestingChildren ( called
            // from vcl via RequestingChildren ) from
            // creating new ( duplicate ) children
            SFEntry* userData = static_cast<SFEntry*>(pEntry->GetUserData());
            if ( userData &&  !userData->isLoaded() )
            {
                userData->setLoaded();
            }
        }
        m_pScriptsBox->SetCurEntry( pNewEntry );
        m_pScriptsBox->Select( m_pScriptsBox->GetCurEntry() );

    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError( m_createErrStr );
        ScopedVclPtrInstance< MessageDialog > aErrorBox(static_cast<vcl::Window*>(this), aError);
        aErrorBox->SetText( m_createErrTitleStr );
        aErrorBox->Execute();
    }
}

void SvxScriptOrgDialog::renameEntry( SvTreeListEntry* pEntry )
{

    Reference< browse::XBrowseNode >  aChildNode;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    Reference< script::XInvocation > xInv( node, UNO_QUERY );

    if ( xInv.is() )
    {
        OUString aNewName = node->getName();
        sal_Int32 extnPos = aNewName.lastIndexOf( '.' );
        OUString extn;
        if(extnPos>0)
        {
            extn = aNewName.copy(extnPos);
            aNewName = aNewName.copy(0,extnPos);
        }
        ScopedVclPtrInstance< CuiInputDialog > xNewDlg( static_cast<vcl::Window*>(this), InputDialogMode::RENAME );
        xNewDlg->SetObjectName( aNewName );

        bool bValid;
        do
        {
            if ( xNewDlg->Execute() && !xNewDlg->GetObjectName().isEmpty() )
            {
                OUString aUserSuppliedName = xNewDlg->GetObjectName();
                bValid = true;
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
        catch( Exception& e )
        {
            SAL_WARN("cui.dialogs", "Caught exception trying to Rename " << e.Message );
        }
    }
    if ( aChildNode.is() )
    {
        m_pScriptsBox->SetEntryText( pEntry, aChildNode->getName() );
        m_pScriptsBox->SetCurEntry( pEntry );
        m_pScriptsBox->Select( m_pScriptsBox->GetCurEntry() );

    }
    else
    {
        //ISSUE L10N & message from exception?
        OUString aError( m_renameErrStr );
        ScopedVclPtrInstance< MessageDialog > aErrorBox(static_cast<vcl::Window*>(this), aError);
        aErrorBox->SetText( m_renameErrTitleStr );
        aErrorBox->Execute();
    }
}
void SvxScriptOrgDialog::deleteEntry( SvTreeListEntry* pEntry )
{
    bool result = false;
    Reference< browse::XBrowseNode > node = getBrowseNode( pEntry );
    // ISSUE L10N string & can we centre list?
    OUString aQuery = m_delQueryStr + getListOfChildren( node, 0 );
    VclPtrInstance< MessageDialog > aQueryBox(static_cast<vcl::Window*>(this), aQuery, VclMessageType::Question, VclButtonsType::YesNo);
    aQueryBox->SetText( m_delQueryTitleStr );
    if ( aQueryBox->Execute() == RET_NO )
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
        catch( Exception& e )
        {
            SAL_WARN("cui.dialogs", "Caught exception trying to delete " << e.Message );
        }
    }

    if ( result )
    {
        m_pScriptsBox->deleteTree( pEntry );
        m_pScriptsBox->GetModel()->Remove( pEntry );
    }
    else
    {
        //ISSUE L10N & message from exception?
        ScopedVclPtrInstance< MessageDialog > aErrorBox(static_cast<vcl::Window*>(this), m_delErrStr);
        aErrorBox->SetText( m_delErrTitleStr );
        aErrorBox->Execute();
    }

}

bool SvxScriptOrgDialog::getBoolProperty( Reference< beans::XPropertySet >& xProps,
                OUString& propName )
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
    OUString result = "\n";
    for( int i=0;i<=depth;i++ )
    {
        result += "\t";
    }
    result += node->getName();

    try
    {
        if ( node->hasChildNodes() )
        {
            Sequence< Reference< browse::XBrowseNode > > children
                = node->getChildNodes();
            for ( sal_Int32 n = 0; n < children.getLength(); n++ )
            {
                result += getListOfChildren( children[ n ] , depth+1 );
            }
        }
    }
    catch ( Exception& )
    {
        // ignore, will return an empty string
    }

    return result;
}

Selection_hash SvxScriptOrgDialog::m_lastSelection;

void SvxScriptOrgDialog::StoreCurrentSelection()
{
    OUString aDescription;
    if ( m_pScriptsBox->IsSelected( m_pScriptsBox->GetHdlEntry() ) )
    {
        SvTreeListEntry* pEntry = m_pScriptsBox->GetHdlEntry();
        while( pEntry )
        {
            aDescription = m_pScriptsBox->GetEntryText( pEntry ) + aDescription;
            pEntry = m_pScriptsBox->GetParent( pEntry );
            if ( pEntry )
                aDescription = ";" + aDescription;
        }
        OUString sDesc( aDescription );
        m_lastSelection[ m_sLanguage ] = sDesc;
    }
}

void SvxScriptOrgDialog::RestorePreviousSelection()
{
    OUString aStoredEntry = m_lastSelection[ m_sLanguage ];
    if( aStoredEntry.isEmpty() )
        return;
    SvTreeListEntry* pEntry = nullptr;
    sal_Int32 nIndex = 0;
    while ( nIndex != -1 )
    {
        OUString aTmp( aStoredEntry.getToken( 0, ';', nIndex ) );
        SvTreeListEntry* pTmpEntry = m_pScriptsBox->FirstChild( pEntry );
        while ( pTmpEntry )
        {
            if ( m_pScriptsBox->GetEntryText( pTmpEntry ) == aTmp )
            {
                pEntry = pTmpEntry;
                break;
            }
            pTmpEntry = SvTreeListBox::NextSibling( pTmpEntry );
        }
        if ( !pTmpEntry )
            break;
        m_pScriptsBox->RequestingChildren( pEntry );
    }
    m_pScriptsBox->SetCurEntry( pEntry );
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
        result += "\n\n" + OUString(CUI_RES(RID_SVXSTR_ERROR_TYPE_LABEL)) + " " + type;
    }

    if ( !message.isEmpty() )
    {
        result += "\n\n" + OUString(CUI_RES(RID_SVXSTR_ERROR_MESSAGE_LABEL)) + " " + message;
    }

    return result;
}

OUString GetErrorMessage(
    const provider::ScriptErrorRaisedException& eScriptError )
{
    OUString unformatted = CUI_RES( RID_SVXSTR_ERROR_AT_LINE );

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
            unformatted = CUI_RES( RID_SVXSTR_ERROR_AT_LINE );
        }
        else
        {
            unformatted = CUI_RES( RID_SVXSTR_ERROR_RUNNING );
        }

    return FormatErrorString(
        unformatted, language, script, line, type, message );
}

OUString GetErrorMessage(
    const provider::ScriptExceptionRaisedException& eScriptException )
{
    OUString unformatted = CUI_RES( RID_SVXSTR_EXCEPTION_AT_LINE );

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
        unformatted = CUI_RES( RID_SVXSTR_EXCEPTION_AT_LINE );
    }
    else
    {
        unformatted = CUI_RES( RID_SVXSTR_EXCEPTION_RUNNING );
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
    OUString unformatted = CUI_RES( RID_SVXSTR_FRAMEWORK_ERROR_RUNNING );

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
        message = OUString(
            CUI_RES(  RID_SVXSTR_ERROR_LANG_NOT_SUPPORTED ) );
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

SvxScriptErrorDialog::SvxScriptErrorDialog( css::uno::Any aException )
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
        message = OUString( CUI_RES( RID_SVXSTR_ERROR_TITLE ) );
    }

    ScopedVclPtrInstance<MessageDialog> pBox( nullptr, message, VclMessageType::Warning );
    pBox->SetText( CUI_RES( RID_SVXSTR_ERROR_TITLE ) );
    pBox->Execute();

    delete pMessage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
