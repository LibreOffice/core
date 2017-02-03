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

#include <config_features.h>

#include <hintids.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <osl/diagnose.h>
#include <tools/link.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/event.hxx>
#include <sfx2/objitem.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svl/srchitem.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/restartdialog.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svl/isethint.hxx>
#include <sfx2/request.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/stritem.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/insctrl.hxx>
#include <svx/selctrl.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <sfx2/docfile.hxx>
#include <svx/xmlsecctrl.hxx>
#include <navicfg.hxx>

#include <sfx2/objface.hxx>
#include <sfx2/app.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <pview.hxx>
#include <srcview.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <cmdid.h>
#include <initui.hxx>
#include <uitool.hxx>
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <wview.hxx>
#include <usrpref.hxx>
#include <gloslst.hxx>
#include <glosdoc.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <cfgitems.hxx>
#include <prtopt.hxx>
#include <modcfg.hxx>
#include <globals.h>
#include <app.hrc>
#include <fontcfg.hxx>
#include <barcfg.hxx>
#include <uinums.hxx>
#include <dbconfig.hxx>
#include <mmconfigitem.hxx>
#include <linguistic/lngprops.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertyStateChangeListener.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/TextConnectionSettings.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <swabstdlg.hxx>

#include <vcl/status.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

// Slotmaps for the application's methods

// here are the SlotID's being included
// see Idl-file
#define SwModule
#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include <cfgid.h>

#include <shells.hrc>

namespace
{
    bool lcl_hasAllComponentsAvailable()
    {
        try
        {
            return css::sdb::TextConnectionSettings::create(comphelper::getProcessComponentContext()).is();
        }
        catch (css::uno::Exception & e)
        {
            SAL_INFO(
                "sw.core",
                "assuming Base to be missing; caught " << e.Message);
            return false;
        }
    }
}

SFX_IMPL_INTERFACE(SwModule, SfxModule)

void SwModule::InitInterface_Impl()
{
    GetStaticInterface()->RegisterStatusBar(CFG_STATUSBAR);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER,
                                            RID_MODULE_TOOLBOX);
}

// other states
void SwModule::StateOther(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SwView* pActView = ::GetActiveView();
    bool bWebView = dynamic_cast<SwWebView*>( pActView ) !=  nullptr;

    while(nWhich)
    {
        switch(nWhich)
        {
            case FN_BUSINESS_CARD:
            case FN_LABEL:
            case FN_ENVELOP:
            {
                bool bDisable = false;
                SfxViewShell* pCurrView = SfxViewShell::Current();
                if( !pCurrView || dynamic_cast< const SwView *>( pCurrView ) ==  nullptr )
                    bDisable = true;
                SwDocShell *pDocSh = static_cast<SwDocShell*>( SfxObjectShell::Current());
                if ( bDisable ||
                    (pDocSh &&    (pDocSh->IsReadOnly() ||
                                  pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED)) )
                    rSet.DisableItem( nWhich );

            }
            break;
            case FN_XFORMS_INIT:
                // slot is always active!
                break;
            case FN_EDIT_FORMULA:
                {
                    SwWrtShell* pSh = nullptr;
                    int nSelection = 0;
                    if( pActView )
                        pSh = &pActView->GetWrtShell();
                    if( pSh )
                        nSelection = pSh->GetSelectionType();

                    if( (pSh && pSh->HasSelection()) ||
                        !(nSelection & (nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL)))
                        rSet.DisableItem(nWhich);
                }
            break;
            case SID_ATTR_METRIC:
                rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, static_cast< sal_uInt16 >(::GetDfltMetric(bWebView))));
            break;
            case FN_SET_MODOPT_TBLNUMFMT:
                rSet.Put( SfxBoolItem( nWhich, m_pModuleConfig->
                                            IsInsTableFormatNum( bWebView )));
            break;
            case FN_MAILMERGE_FIRST_ENTRY:
            case FN_MAILMERGE_PREV_ENTRY:
            case FN_MAILMERGE_NEXT_ENTRY:
            case FN_MAILMERGE_LAST_ENTRY:
            {
                SwView* pView = ::GetActiveView();
                std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
                if (pView)
                    xConfigItem = pView->GetMailMergeConfigItem();
                if (!xConfigItem)
                    rSet.DisableItem(nWhich);
                else
                {
                    bool bFirst, bLast;
                    bool bValid = xConfigItem->IsResultSetFirstLast(bFirst, bLast);

                    if (!bValid ||
                        (bFirst && (nWhich == FN_MAILMERGE_FIRST_ENTRY || nWhich == FN_MAILMERGE_PREV_ENTRY)) ||
                        (bLast && (nWhich == FN_MAILMERGE_LAST_ENTRY || nWhich == FN_MAILMERGE_NEXT_ENTRY)))
                    {
                        rSet.DisableItem(nWhich);
                    }
                }
            }
            break;
            case FN_MAILMERGE_CURRENT_ENTRY:
            case FN_MAILMERGE_EXCLUDE_ENTRY:
            {
                // just trigger calling statusChanged() of MMExcludeEntryController
                // resp. MMCurrentEntryController
                rSet.InvalidateItem(nWhich);
            }
            break;
            case FN_MAILMERGE_CREATE_DOCUMENTS:
            case FN_MAILMERGE_SAVE_DOCUMENTS:
            case FN_MAILMERGE_PRINT_DOCUMENTS:
            case FN_MAILMERGE_EMAIL_DOCUMENTS:
            {
                SwView* pView = ::GetActiveView();
                std::shared_ptr<SwMailMergeConfigItem> xConfigItem;
                if (pView)
                    xConfigItem = pView->EnsureMailMergeConfigItem();

                // #i51949# hide e-Mail option if e-Mail is not supported
                // #i63267# printing might be disabled
                if (!xConfigItem ||
                    (nWhich == FN_MAILMERGE_PRINT_DOCUMENTS && Application::GetSettings().GetMiscSettings().GetDisablePrinting()) ||
                    (nWhich == FN_MAILMERGE_EMAIL_DOCUMENTS && !xConfigItem->IsMailAvailable()))
                {
                    rSet.DisableItem(nWhich);
                }
            }
            break;
            default:
                OSL_FAIL("::StateOther: default");
        }
        nWhich = aIter.NextWhich();
    }
}

// start field dialog
void NewXForms( SfxRequest& rReq ); // implementation: below

#if HAVE_FEATURE_DBCONNECTIVITY

std::shared_ptr<SwMailMergeConfigItem> SwView::EnsureMailMergeConfigItem(const SfxItemSet* pArgs, sal_uInt16 nWizardRestartPage)
{
    // create if it does not exist yet
    std::shared_ptr<SwMailMergeConfigItem> xMMConfig = GetMailMergeConfigItem();
    if (!xMMConfig)
    {
        xMMConfig.reset(new SwMailMergeConfigItem);
        xMMConfig->SetSourceView(this);

        //set the first used database as default source on the config item
        const SfxPoolItem* pItem = nullptr;
        if (pArgs && SfxItemState::SET == pArgs->GetItemState(
               FN_PARAM_DATABASE_PROPERTIES, false, &pItem))
        {
            //mailmerge has been called from the database beamer
            uno::Sequence< beans::PropertyValue> aDBValues;
            if (static_cast<const SfxUsrAnyItem*>(pItem)->GetValue() >>= aDBValues)
            {
                SwDBData aDBData;
                svx::ODataAccessDescriptor aDescriptor(aDBValues);
                aDescriptor[svx::DataAccessDescriptorProperty::DataSource]   >>= aDBData.sDataSource;
                aDescriptor[svx::DataAccessDescriptorProperty::Command]      >>= aDBData.sCommand;
                aDescriptor[svx::DataAccessDescriptorProperty::CommandType]  >>= aDBData.nCommandType;

                uno::Sequence< uno::Any >                   aSelection;
                uno::Reference< sdbc::XConnection>          xConnection;
                uno::Reference< sdbc::XDataSource>          xSource;
                uno::Reference< sdbcx::XColumnsSupplier>    xColumnsSupplier;
                if (aDescriptor.has(svx::DataAccessDescriptorProperty::Selection))
                    aDescriptor[svx::DataAccessDescriptorProperty::Selection] >>= aSelection;
                if (aDescriptor.has(svx::DataAccessDescriptorProperty::Connection))
                    aDescriptor[svx::DataAccessDescriptorProperty::Connection] >>= xConnection;
                uno::Reference<container::XChild> xChild(xConnection, uno::UNO_QUERY);
                if (xChild.is())
                    xSource.set(xChild->getParent(), uno::UNO_QUERY);
                xMMConfig->SetCurrentConnection(
                    xSource, SharedConnection(xConnection, SharedConnection::NoTakeOwnership),
                    xColumnsSupplier, aDBData);
            }
        }
        else
        {
            std::vector<OUString> aDBNameList;
            std::vector<OUString> aAllDBNames;
            GetWrtShell().GetAllUsedDB(aDBNameList, &aAllDBNames);
            if (!aDBNameList.empty())
            {
                OUString sDBName(aDBNameList[0]);
                SwDBData aDBData;
                aDBData.sDataSource = sDBName.getToken(0, DB_DELIM);
                aDBData.sCommand = sDBName.getToken(1, DB_DELIM);
                aDBData.nCommandType = sDBName.getToken(2, DB_DELIM).toInt32();
                //set the currently used database for the wizard
                xMMConfig->SetCurrentDBData(aDBData);
            }
        }

        SetMailMergeConfigItem(xMMConfig, nWizardRestartPage);
    }
    return xMMConfig;
}

namespace
{

SwView* lcl_LoadDoc(SwView* pView, const OUString& rURL)
{
    SwView* pNewView = nullptr;
    if(!rURL.isEmpty())
    {
        SfxStringItem aURL(SID_FILE_NAME, rURL);
        SfxStringItem aTargetFrameName( SID_TARGETNAME, OUString("_blank") );
        SfxBoolItem aHidden( SID_HIDDEN, true );
        SfxStringItem aReferer(SID_REFERER, pView->GetDocShell()->GetTitle());
        const SfxObjectItem* pItem = static_cast<const SfxObjectItem*>(
            pView->GetViewFrame()->GetDispatcher()->ExecuteList(SID_OPENDOC,
                SfxCallMode::SYNCHRON,
                { &aURL, &aHidden, &aReferer, &aTargetFrameName }));
        SfxShell* pShell = pItem ? pItem->GetShell() : nullptr;

        if(pShell)
        {
            SfxViewShell* pViewShell = pShell->GetViewShell();
            if(pViewShell)
            {
                if( nullptr!= dynamic_cast<SwView*>(pViewShell) )
                {
                    pNewView = dynamic_cast< SwView* >(pViewShell);
                    pNewView->GetViewFrame()->GetFrame().Appear();
                }
                else
                {
                    pViewShell->GetViewFrame()->DoClose();
                }
            }
        }
    }
    else
    {
        SfxStringItem aFactory(SID_NEWDOCDIRECT, SwDocShell::Factory().GetFilterContainer()->GetName());
        const SfxFrameItem* pItem = static_cast<const SfxFrameItem*>(
            pView->GetViewFrame()->GetDispatcher()->ExecuteList(
                SID_NEWDOCDIRECT, SfxCallMode::SYNCHRON, { &aFactory }));
        SfxFrame* pFrame = pItem ? pItem->GetFrame() : nullptr;
        SfxViewFrame* pViewFrame = pFrame ? pFrame->GetCurrentViewFrame() : nullptr;
        pNewView = pViewFrame ? dynamic_cast<SwView*>( pViewFrame->GetViewShell() ) : nullptr;
    }

    return pNewView;
}

class SwMailMergeWizardExecutor : public salhelper::SimpleReferenceObject
{
    SwView*                  m_pView;       // never owner
    SwView*                  m_pView2Close; // never owner
    VclPtr<AbstractMailMergeWizard> m_pWizard;     // always owner
    VclPtr<AbstractMailMergeWizard> m_pWizardToDestroyInCallback;

    bool                     m_bDestroyMMToolbarOnCancel;

    DECL_LINK( EndDialogHdl, Dialog&, void );
    DECL_LINK( DestroyDialogHdl, void*, void );
    DECL_LINK( DestroyWizardHdl, void*, void );
    DECL_LINK( CancelHdl, void*, void );
    DECL_LINK( CloseFrameHdl, void*, void );

    void ExecutionFinished();
    void ExecuteWizard();

public:
    SwMailMergeWizardExecutor();
    virtual ~SwMailMergeWizardExecutor() override;

    void ExecuteMailMergeWizard( const SfxItemSet * pArgs );
};

SwMailMergeWizardExecutor::SwMailMergeWizardExecutor()
    : m_pView( nullptr ),
      m_pView2Close( nullptr ),
      m_pWizard( nullptr ),
      m_bDestroyMMToolbarOnCancel( false )
{
}

SwMailMergeWizardExecutor::~SwMailMergeWizardExecutor()
{
    OSL_ENSURE( m_pWizard == nullptr, "SwMailMergeWizardExecutor: m_pWizard must be Null!" );
}

void SwMailMergeWizardExecutor::ExecuteMailMergeWizard( const SfxItemSet * pArgs )
{
    if(!lcl_hasAllComponentsAvailable())
    {
        try
        {
            using namespace org::freedesktop::PackageKit;
            using namespace svtools;
            css::uno::Reference< XSyncDbusSessionHelper > xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
            const css::uno::Sequence< OUString > vPackages{ "libreoffice-base" };
            OUString sInteraction;
            xSyncDbusSessionHelper->InstallPackageNames(0, vPackages, sInteraction);
            SolarMutexGuard aGuard;
            executeRestartDialog(comphelper::getProcessComponentContext(), nullptr, RESTART_REASON_MAILMERGE_INSTALL);
        }
        catch (const css::uno::Exception & e)
        {
            SAL_INFO(
                "sw.core",
                "trying to install LibreOffice Base, caught " << e.Message);
        }
        return;
    }
    if ( m_pView )
    {
        OSL_FAIL("SwMailMergeWizardExecutor::ExecuteMailMergeWizard: Already executing the wizard!" );
        return;
    }

    m_pView = ::GetActiveView();
    if (!m_pView)
        return;

    // keep self alive until done.
    acquire();

    // create if it does not exist yet
    std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->EnsureMailMergeConfigItem(pArgs, m_pWizard ? m_pWizard->GetRestartPage() : 0);

    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    m_pWizard = pFact->CreateMailMergeWizard(*m_pView, xMMConfig);

    uno::Reference<beans::XPropertySet> xPropSet(m_pView->GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    uno::Reference<frame::XLayoutManager> xLayoutManager;
    uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
    aValue >>= xLayoutManager;
    if (!xLayoutManager.is())
        return;

    const OUString sResourceURL( "private:resource/toolbar/mailmerge" );
    uno::Reference<ui::XUIElement> xUIElement = xLayoutManager->getElement(sResourceURL);
    if (!xUIElement.is())
    {
        // ensure the mail-merge toolbar is displayed and remember if it was before
        m_bDestroyMMToolbarOnCancel = true;
        xLayoutManager->createElement(sResourceURL);
        xLayoutManager->showElement(sResourceURL);
    }
    ExecuteWizard();
}

void SwMailMergeWizardExecutor::ExecutionFinished()
{
    std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
    if (xMMConfig)
        xMMConfig->Commit();

    // release/destroy asynchronously
    Application::PostUserEvent( LINK( this, SwMailMergeWizardExecutor, DestroyDialogHdl ) );
}

void SwMailMergeWizardExecutor::ExecuteWizard()
{
    m_pWizard->StartExecuteModal(
        LINK( this, SwMailMergeWizardExecutor, EndDialogHdl ) );
}

IMPL_LINK_NOARG( SwMailMergeWizardExecutor, EndDialogHdl, Dialog&, void )
{
    long nRet = m_pWizard->GetResult();
    sal_uInt16 nRestartPage = m_pWizard->GetRestartPage();

    switch ( nRet )
    {
    case RET_LOAD_DOC:
        {
            SwView* pNewView = lcl_LoadDoc(m_pView, m_pWizard->GetReloadDocument());

            // Destroy wizard asynchronously, since we are deep inside the wizard and dialog
            // machinery code here
            m_pWizardToDestroyInCallback = m_pWizard;
            Application::PostUserEvent(
                LINK( this, SwMailMergeWizardExecutor, DestroyWizardHdl ), nullptr );

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
            if (pNewView)
            {
                m_pView = pNewView;
                xMMConfig->DocumentReloaded();
                //new source view!
                xMMConfig->SetSourceView( m_pView );
                m_pWizard = pFact->CreateMailMergeWizard(*m_pView, xMMConfig);
                m_pWizard->ShowPage( nRestartPage );
            }
            else
            {
                m_pWizard = pFact->CreateMailMergeWizard(*m_pView, xMMConfig);
            }

            // execute the wizard again
            ExecuteWizard();
            break;
        }
    case RET_TARGET_CREATED:
        {
            std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
            SwView* pTargetView = xMMConfig->GetTargetView();
            OSL_ENSURE(pTargetView, "No target view has been created");
            if(pTargetView)
            {
                // destroy wizard asynchronously
                m_pWizardToDestroyInCallback = m_pWizard;
                Application::PostUserEvent(
                    LINK( this, SwMailMergeWizardExecutor, DestroyWizardHdl ), nullptr );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                m_pWizard = pFact->CreateMailMergeWizard(*pTargetView, xMMConfig);
                m_pWizard->ShowPage( nRestartPage );

                // execute the wizard again
                ExecuteWizard();
            }
            else
            {
                // should not happen - just in case no target view has been created
                ExecutionFinished();
            }
            break;
        }
    case RET_REMOVE_TARGET:
        {
            std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
            SwView* pTargetView = xMMConfig->GetTargetView();
            SwView* pSourceView = xMMConfig->GetSourceView();
            OSL_ENSURE(pTargetView && pSourceView, "source or target view not available" );
            if(pTargetView && pSourceView)
            {
                m_pView2Close = pTargetView;
                pTargetView->GetViewFrame()->GetTopViewFrame()->GetWindow().Hide();
                pSourceView->GetViewFrame()->GetFrame().AppearWithUpdate();
                // the current view has be set when the target is destroyed
                m_pView = pSourceView;
                xMMConfig->SetTargetView(nullptr);

                // destroy wizard asynchronously
                Application::PostUserEvent(
                    LINK( this, SwMailMergeWizardExecutor, CloseFrameHdl ), m_pWizard );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                m_pWizard = pFact->CreateMailMergeWizard(*pSourceView, xMMConfig);
                m_pWizard->ShowPage( nRestartPage );

                // execute the wizard again
                ExecuteWizard();
            }
            else
            {
                // should not happen - just in case no target view has been created
                ExecutionFinished();
            }
            break;
        }
    case RET_CANCEL:
        {
            // close frame and destroy wizard asynchronously
            Application::PostUserEvent(
                LINK( this, SwMailMergeWizardExecutor, CancelHdl ), m_pWizard );
            break;
        }
    default: // finish
        {
            std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
            SwView* pSourceView = xMMConfig.get() ? xMMConfig->GetSourceView() : nullptr;
            if(pSourceView)
            {
                xMMConfig->GetSourceView()->GetViewFrame()->GetFrame().Appear();
            }
            ExecutionFinished();
            break;
        }

    } // switch
}

IMPL_LINK_NOARG(SwMailMergeWizardExecutor, DestroyDialogHdl, void*, void)
{
    m_pWizard.disposeAndClear();

    release();
}

IMPL_LINK_NOARG(SwMailMergeWizardExecutor, DestroyWizardHdl, void*, void)
{
    m_pWizardToDestroyInCallback.disposeAndClear();
}

IMPL_LINK_NOARG(SwMailMergeWizardExecutor, CancelHdl, void*, void)
{
    std::shared_ptr<SwMailMergeConfigItem> xMMConfig = m_pView->GetMailMergeConfigItem();
    if (xMMConfig)
    {
        if (xMMConfig->GetTargetView())
        {
            xMMConfig->GetTargetView()->GetViewFrame()->DoClose();
            xMMConfig->SetTargetView(nullptr);
        }
        if (xMMConfig->GetSourceView())
        {
            auto pViewFrame(xMMConfig->GetSourceView()->GetViewFrame());
            pViewFrame->GetFrame().AppearWithUpdate();
            uno::Reference<beans::XPropertySet> xPropSet(pViewFrame->GetFrame().GetFrameInterface(), uno::UNO_QUERY);
            if (xPropSet.is() && m_bDestroyMMToolbarOnCancel)
            {
                // hide mailmerge toolbar if it hasn't been there before
                uno::Reference<frame::XLayoutManager> xLayoutManager;
                uno::Any aValue = xPropSet->getPropertyValue("LayoutManager");
                aValue >>= xLayoutManager;
                if (xLayoutManager.is())
                {
                    const OUString sResourceURL( "private:resource/toolbar/mailmerge" );
                    xLayoutManager->destroyElement( sResourceURL );
                }
            }
        }
        xMMConfig->Commit();
    }

    m_pWizard.disposeAndClear();
    release();
}

IMPL_LINK_NOARG(SwMailMergeWizardExecutor, CloseFrameHdl, void*, void)
{
    if ( m_pView2Close )
    {
        m_pView2Close->GetViewFrame()->DoClose();
        m_pView2Close = nullptr;
    }
}
} // namespace

#endif // HAVE_FEATURE_DBCONNECTIVITY

void SwModule::ExecOther(SfxRequest& rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;

    sal_uInt16 nWhich = rReq.GetSlot();
    switch (nWhich)
    {
        case FN_ENVELOP:
            InsertEnv( rReq );
            break;

        case FN_BUSINESS_CARD:
        case FN_LABEL:
            InsertLab(rReq, nWhich == FN_LABEL);
            break;

        case FN_XFORMS_INIT:
            NewXForms( rReq );
            break;

        case SID_ATTR_METRIC:
        if(pArgs && SfxItemState::SET == pArgs->GetItemState(nWhich, false, &pItem))
        {
            FieldUnit eUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            switch( eUnit )
            {
                case FUNIT_MM:
                case FUNIT_CM:
                case FUNIT_INCH:
                case FUNIT_PICA:
                case FUNIT_POINT:
                {
                    SwView* pActView = ::GetActiveView();
                    bool bWebView = dynamic_cast<SwWebView*>( pActView ) !=  nullptr;
                    ::SetDfltMetric(eUnit, bWebView);
                }
                break;
                default:;//prevent warning
            }
        }
        break;

        case FN_SET_MODOPT_TBLNUMFMT:
            {
                bool bWebView = dynamic_cast<SwWebView*>( ::GetActiveView() )!=  nullptr ,
                     bSet;

                if( pArgs && SfxItemState::SET == pArgs->GetItemState(
                        nWhich, false, &pItem ))
                    bSet = static_cast<const SfxBoolItem*>(pItem)->GetValue();
                else
                    bSet = !m_pModuleConfig->IsInsTableFormatNum( bWebView );

                m_pModuleConfig->SetInsTableFormatNum( bWebView, bSet );
            }
            break;
#if HAVE_FEATURE_DBCONNECTIVITY
        case FN_MAILMERGE_WIZARD:
        {
            // show the mailmerge wizard
            rtl::Reference< SwMailMergeWizardExecutor > xEx( new SwMailMergeWizardExecutor );
            xEx->ExecuteMailMergeWizard( pArgs );
        }
        break;
        case FN_MAILMERGE_FIRST_ENTRY:
        case FN_MAILMERGE_PREV_ENTRY:
        case FN_MAILMERGE_NEXT_ENTRY:
        case FN_MAILMERGE_LAST_ENTRY:
        case FN_MAILMERGE_CURRENT_ENTRY:
        {
            SwView* pView = ::GetActiveView();
            std::shared_ptr<SwMailMergeConfigItem> xConfigItem = pView->GetMailMergeConfigItem();
            if (!xConfigItem)
                return;

            sal_Int32 nPos = xConfigItem->GetResultSetPosition();
            switch (nWhich)
            {
                case FN_MAILMERGE_FIRST_ENTRY: xConfigItem->MoveResultSet(1); break;
                case FN_MAILMERGE_PREV_ENTRY:  xConfigItem->MoveResultSet(nPos - 1); break;
                case FN_MAILMERGE_NEXT_ENTRY:  xConfigItem->MoveResultSet(nPos + 1); break;
                case FN_MAILMERGE_LAST_ENTRY:  xConfigItem->MoveResultSet(-1); break;
                case FN_MAILMERGE_CURRENT_ENTRY: /* don't move the result set, just update the document */ break;
                default: break;
            }

            // now the record has to be merged into the source document
            // TODO can we re-use PerformMailMerge() here somehow?
            const SwDBData& rDBData = xConfigItem->GetCurrentDBData();
            uno::Sequence<uno::Any> vSelection({ uno::makeAny(xConfigItem->GetResultSetPosition()) });
            svx::ODataAccessDescriptor aDescriptor(::comphelper::InitPropertySequence({
                        {"Selection",        uno::makeAny(vSelection)},
                        {"DataSourceName",   uno::makeAny(rDBData.sDataSource)},
                        {"Command",          uno::makeAny(rDBData.sCommand)},
                        {"CommandType",      uno::makeAny(rDBData.nCommandType)},
                        {"ActiveConnection", uno::makeAny(xConfigItem->GetConnection().getTyped())},
                        {"Filter",           uno::makeAny(xConfigItem->GetFilter())},
                        {"Cursor",           uno::makeAny(xConfigItem->GetResultSet())}
                        }));

            SwWrtShell& rSh = pView->GetWrtShell();
            SwMergeDescriptor aMergeDesc(DBMGR_MERGE, rSh, aDescriptor);
            rSh.GetDBManager()->Merge(aMergeDesc);

            // update enabled / disabled status of the buttons in the toolbar
            SfxBindings& rBindings = rSh.GetView().GetViewFrame()->GetBindings();
            rBindings.Invalidate(FN_MAILMERGE_FIRST_ENTRY);
            rBindings.Invalidate(FN_MAILMERGE_PREV_ENTRY);
            rBindings.Invalidate(FN_MAILMERGE_NEXT_ENTRY);
            rBindings.Invalidate(FN_MAILMERGE_LAST_ENTRY);
            rBindings.Invalidate(FN_MAILMERGE_CURRENT_ENTRY);
            rBindings.Invalidate(FN_MAILMERGE_EXCLUDE_ENTRY);
            rBindings.Update();
        }
        break;
        case FN_MAILMERGE_CREATE_DOCUMENTS:
        {
            std::shared_ptr<SwMailMergeConfigItem> xConfigItem = SwDBManager::PerformMailMerge(GetActiveView());

            if (xConfigItem && xConfigItem->GetTargetView())
                xConfigItem->GetTargetView()->GetViewFrame()->GetFrame().Appear();
        }
        break;
        case FN_MAILMERGE_SAVE_DOCUMENTS:
        case FN_MAILMERGE_PRINT_DOCUMENTS:
        case FN_MAILMERGE_EMAIL_DOCUMENTS:
        {
            std::shared_ptr<SwMailMergeConfigItem> xConfigItem = GetActiveView()->GetMailMergeConfigItem();
            if(!xConfigItem)
                return;
            xConfigItem->SetTargetView(nullptr);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            switch (nWhich)
            {
                case FN_MAILMERGE_SAVE_DOCUMENTS: pFact->ExecuteMMResultSaveDialog(); break;
                case FN_MAILMERGE_PRINT_DOCUMENTS: pFact->ExecuteMMResultPrintDialog(); break;
                case FN_MAILMERGE_EMAIL_DOCUMENTS: pFact->ExecuteMMResultEmailDialog(); break;
            }
        }
        break;
#endif
    }
}

// Catch notifications

// Catch hint for DocInfo
void SwModule::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if( dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        const SfxEventHint& rEvHint = static_cast<const SfxEventHint&>( rHint);
        SwDocShell* pDocSh = dynamic_cast<SwDocShell*>( rEvHint.GetObjShell()  );
        if( pDocSh )
        {
            SwWrtShell* pWrtSh = pDocSh->GetWrtShell();
            switch( rEvHint.GetEventId() )
            {
            case SFX_EVENT_LOADFINISHED:
                OSL_ASSERT(!pWrtSh);
                // if it is a new document created from a template,
                // update fixed fields
                if (pDocSh->GetMedium())
                {
                    const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(pDocSh->GetMedium()->GetItemSet(), SID_TEMPLATE, false);
                    if (pTemplateItem && pTemplateItem->GetValue())
                    {
                        pDocSh->GetDoc()->getIDocumentFieldsAccess().SetFixFields(nullptr);
                    }
                }
                break;
            case SFX_EVENT_CREATEDOC:
                // Update all FIX-Date/Time fields
                if( pWrtSh )
                {
                    const SfxUInt16Item* pUpdateDocItem = SfxItemSet::GetItem<SfxUInt16Item>(pDocSh->GetMedium()->GetItemSet(), SID_UPDATEDOCMODE, false);
                    bool bUpdateFields = true;
                    if( pUpdateDocItem &&  pUpdateDocItem->GetValue() == document::UpdateDocMode::NO_UPDATE)
                        bUpdateFields = false;
                    if(bUpdateFields)
                    {
                        pWrtSh->UpdateInputFields();

                        // Are database fields contained?
                        // Get all used databases for the first time
                        SwDoc *pDoc = pDocSh->GetDoc();
                        std::vector<OUString> aDBNameList;
                        pDoc->GetAllUsedDB( aDBNameList );
                        if(!aDBNameList.empty())
                        {   // Open database beamer
                            ShowDBObj(pWrtSh->GetView(), pDoc->GetDBData());
                        }
                    }
                }
                break;
            }
        }
    }
    else if(dynamic_cast<const SfxItemSetHint*>(&rHint))
    {
        if( SfxItemState::SET == static_cast<const SfxItemSetHint&>(rHint).GetItemSet().GetItemState(SID_ATTR_PATHNAME))
        {
            ::GetGlossaries()->UpdateGlosPath( false );
            SwGlossaryList* pList = ::GetGlossaryList();
            if(pList->IsActive())
                pList->Update();
        }
    }
    else
    {
        if (rHint.GetId() == SFX_HINT_DEINITIALIZING)
        {
            DELETEZ(m_pWebUsrPref);
            DELETEZ(m_pUsrPref);
            DELETEZ(m_pModuleConfig);
            DELETEZ(m_pPrintOptions);
            DELETEZ(m_pWebPrintOptions);
            DELETEZ(m_pChapterNumRules);
            DELETEZ(m_pStdFontConfig);
            DELETEZ(m_pNavigationConfig);
            DELETEZ(m_pToolbarConfig);
            DELETEZ(m_pWebToolbarConfig);
            DELETEZ(m_pAuthorNames);
            DELETEZ(m_pDBConfig);
            if( m_pColorConfig )
            {
                m_pColorConfig->RemoveListener(this);
                DELETEZ(m_pColorConfig);
            }
            if( m_pAccessibilityOptions )
            {
                m_pAccessibilityOptions->RemoveListener(this);
                DELETEZ(m_pAccessibilityOptions);
            }
            if( m_pCTLOptions )
            {
                m_pCTLOptions->RemoveListener(this);
                DELETEZ(m_pCTLOptions);
            }
            if( m_pUserOptions )
            {
                m_pUserOptions->RemoveListener(this);
                DELETEZ(m_pUserOptions);
            }
        }
    }
}

void SwModule::ConfigurationChanged( utl::ConfigurationBroadcaster* pBrdCst, sal_uInt32 )
{
    if( pBrdCst == m_pUserOptions )
    {
        m_bAuthorInitialised = false;
    }
    else if ( pBrdCst == m_pColorConfig || pBrdCst == m_pAccessibilityOptions )
    {
        bool bAccessibility = false;
        if( pBrdCst == m_pColorConfig )
            SwViewOption::ApplyColorConfigValues(*m_pColorConfig);
        else
            bAccessibility = true;

        //invalidate all edit windows
        SfxViewShell* pViewShell = SfxViewShell::GetFirst();
        while(pViewShell)
        {
            if(pViewShell->GetWindow())
            {
                if((dynamic_cast< const SwView *>( pViewShell ) !=  nullptr ||
                    dynamic_cast< const SwPagePreview *>( pViewShell ) !=  nullptr ||
                    dynamic_cast< const SwSrcView *>( pViewShell ) !=  nullptr))
                {
                    if(bAccessibility)
                    {
                        if(dynamic_cast< const SwView *>( pViewShell ) !=  nullptr)
                            static_cast<SwView*>(pViewShell)->ApplyAccessiblityOptions(*m_pAccessibilityOptions);
                        else if(dynamic_cast< const SwPagePreview *>( pViewShell ) !=  nullptr)
                            static_cast<SwPagePreview*>(pViewShell)->ApplyAccessiblityOptions(*m_pAccessibilityOptions);
                    }
                    pViewShell->GetWindow()->Invalidate();
                }
            }
            pViewShell = SfxViewShell::GetNext( *pViewShell );
        }
    }
    else if( pBrdCst == m_pCTLOptions )
    {
        const SfxObjectShell* pObjSh = SfxObjectShell::GetFirst();
        while( pObjSh )
        {
            if( dynamic_cast<const SwDocShell*>(pObjSh) !=  nullptr )
            {
                SwDoc* pDoc = const_cast<SwDocShell*>(static_cast<const SwDocShell*>(pObjSh))->GetDoc();
                SwViewShell* pVSh = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
                if ( pVSh )
                    pVSh->ChgNumberDigits();
            }
            pObjSh = SfxObjectShell::GetNext(*pObjSh);
        }
    }

}

SwDBConfig* SwModule::GetDBConfig()
{
    if(!m_pDBConfig)
        m_pDBConfig = new SwDBConfig;
    return m_pDBConfig;
}

svtools::ColorConfig& SwModule::GetColorConfig()
{
    if(!m_pColorConfig)
    {
        m_pColorConfig = new svtools::ColorConfig;
        SwViewOption::ApplyColorConfigValues(*m_pColorConfig);
        m_pColorConfig->AddListener(this);
    }
    return *m_pColorConfig;
}

SvtAccessibilityOptions& SwModule::GetAccessibilityOptions()
{
    if(!m_pAccessibilityOptions)
    {
        m_pAccessibilityOptions = new SvtAccessibilityOptions;
        m_pAccessibilityOptions->AddListener(this);
    }
    return *m_pAccessibilityOptions;
}

SvtCTLOptions& SwModule::GetCTLOptions()
{
    if(!m_pCTLOptions)
    {
        m_pCTLOptions = new SvtCTLOptions;
        m_pCTLOptions->AddListener(this);
    }
    return *m_pCTLOptions;
}

SvtUserOptions& SwModule::GetUserOptions()
{
    if(!m_pUserOptions)
    {
        m_pUserOptions = new SvtUserOptions;
        m_pUserOptions->AddListener(this);
    }
    return *m_pUserOptions;
}

const SwMasterUsrPref *SwModule::GetUsrPref(bool bWeb) const
{
    SwModule* pNonConstModule = const_cast<SwModule*>(this);
    if(bWeb && !m_pWebUsrPref)
    {
        // The SpellChecker is needed in SwMasterUsrPref's Load, but it must not
        // be created there #58256#
        pNonConstModule->m_pWebUsrPref = new SwMasterUsrPref(true);
    }
    else if(!bWeb && !m_pUsrPref)
    {
        pNonConstModule->m_pUsrPref = new SwMasterUsrPref(false);
    }
    return  bWeb ? m_pWebUsrPref : m_pUsrPref;
}

void NewXForms( SfxRequest& rReq )
{
    // copied & excerpted from SwModule::InsertLab(..)

    // create new document
    SfxObjectShellLock xDocSh( new SwDocShell( SfxObjectCreateMode::STANDARD) );
    xDocSh->DoInitNew();

    // initialize XForms
    static_cast<SwDocShell*>( &xDocSh )->GetDoc()->initXForms( true );

    // load document into frame
    SfxViewFrame::DisplayNewDocument( *xDocSh, rReq );

    // set return value
    rReq.SetReturnValue( SfxVoidItem( rReq.GetSlot() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
