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

#include <comphelper/types.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include "framectr.hxx"
#include "datman.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include "bibconfig.hxx"
#include <cppuhelper/implbase.hxx>
#include <utility>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdb/FilterDialog.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/weld.hxx>
#include <osl/mutex.hxx>

#include <unordered_map>

using namespace osl;
using namespace cppu;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star;

namespace {

struct DispatchInfo
{
    const char*   pCommand;
    sal_Int16     nGroupId;
    bool      bActiveConnection;
};

struct CacheDispatchInfo
{
    sal_Int16     nGroupId;
    bool      bActiveConnection;
};

}

// Attention: commands must be sorted by command groups. Implementation is dependent
// on this!!
const DispatchInfo SupportedCommandsArray[] =
{
    { ".uno:Undo"               ,   frame::CommandGroup::EDIT       , false },
    { ".uno:Cut"                ,   frame::CommandGroup::EDIT       , false },
    { ".uno:Copy"               ,   frame::CommandGroup::EDIT       , false },
    { ".uno:Paste"              ,   frame::CommandGroup::EDIT       , false },
    { ".uno:SelectAll"          ,   frame::CommandGroup::EDIT       , false },
    { ".uno:CloseDoc"           ,   frame::CommandGroup::DOCUMENT   , false },
    { ".uno:StatusBarVisible"   ,   frame::CommandGroup::VIEW       , false },
    { ".uno:AvailableToolbars"  ,   frame::CommandGroup::VIEW       , false },
    { ".uno:Bib/standardFilter" ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/DeleteRecord"   ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/InsertRecord"   ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/query"          ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/autoFilter"     ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/source"         ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/removeFilter"   ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/sdbsource"      ,   frame::CommandGroup::DATA       , true  },
    { ".uno:Bib/Mapping"        ,   frame::CommandGroup::DATA       , true  },
};

typedef std::unordered_map< OUString, CacheDispatchInfo > CmdToInfoCache;

static const CmdToInfoCache& GetCommandToInfoCache()
{
    static CmdToInfoCache aCmdToInfoCache = []() {
        CmdToInfoCache aCache;
        for (const auto& command : SupportedCommandsArray)
        {
            OUString aCommand(OUString::createFromAscii(command.pCommand));

            CacheDispatchInfo aDispatchInfo;
            aDispatchInfo.nGroupId = command.nGroupId;
            aDispatchInfo.bActiveConnection = command.bActiveConnection;
            aCache.emplace(aCommand, aDispatchInfo);
        }
        return aCache;
    }();

    return aCmdToInfoCache;
}


class BibFrameCtrl_Impl : public cppu::WeakImplHelper < XFrameActionListener >
{
public:
    Mutex                               aMutex;
    comphelper::OMultiTypeInterfaceContainerHelper2  aLC;

    BibFrameController_Impl*            pController;

                                        BibFrameCtrl_Impl()
                                            : aLC( aMutex )
                                            , pController(nullptr)
                                        {}

    virtual void                        SAL_CALL frameAction(const FrameActionEvent& aEvent) override;
    virtual void                        SAL_CALL disposing( const lang::EventObject& Source ) override;
};

void BibFrameCtrl_Impl::frameAction(const FrameActionEvent& )
{
}

void BibFrameCtrl_Impl::disposing( const lang::EventObject& /*Source*/ )
{
    ::SolarMutexGuard aGuard;
    if ( pController )
        pController->getFrame()->removeFrameActionListener( this );
}

BibFrameController_Impl::BibFrameController_Impl( uno::Reference< awt::XWindow > xComponent,
                                                BibDataManager* pDataManager)
    :m_xWindow(std::move( xComponent ))
    ,m_xDatMan( pDataManager )
{
    m_bDisposing = false;
    m_xImpl = new BibFrameCtrl_Impl;
    m_xImpl->pController = this;
}

BibFrameController_Impl::~BibFrameController_Impl()
{
    m_xImpl->pController = nullptr;
    m_xDatMan.clear();
}

OUString SAL_CALL BibFrameController_Impl::getImplementationName()
{
    return u"com.sun.star.comp.extensions.Bibliography"_ustr;
}

sal_Bool SAL_CALL BibFrameController_Impl::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService( this, sServiceName );
}

css::uno::Sequence< OUString > SAL_CALL BibFrameController_Impl::getSupportedServiceNames()
{
    // return only top level services ...
    // base services are included there and should be asked by uno-rtti.
    return { u"com.sun.star.frame.Bibliography"_ustr };
}

void BibFrameController_Impl::attachFrame( const uno::Reference< XFrame > & xArg )
{
    m_xFrame = xArg;
    m_xFrame->addFrameActionListener( m_xImpl );
}

sal_Bool BibFrameController_Impl::attachModel( const uno::Reference< XModel > & /*xModel*/ )
{
    return false;
}

sal_Bool BibFrameController_Impl::suspend( sal_Bool bSuspend )
{
    if ( bSuspend )
        getFrame()->removeFrameActionListener( m_xImpl );
    else
        getFrame()->addFrameActionListener( m_xImpl );
    return true;
}

uno::Any BibFrameController_Impl::getViewData()
{
    return uno::Any();
}

void BibFrameController_Impl::restoreViewData( const uno::Any& /*Value*/ )
{
}

uno::Reference< XFrame >  BibFrameController_Impl::getFrame()
{
    return m_xFrame;
}

uno::Reference< XModel >  BibFrameController_Impl::getModel()
{
    return uno::Reference< XModel > ();
}

void BibFrameController_Impl::dispose()
{
    m_bDisposing = true;
    lang::EventObject aObject;
    uno::Reference< XFrame > xFrame = getFrame();

    if (xFrame.is())
        xFrame->removeFrameActionListener( m_xImpl );
    m_xFrame.clear();

    aObject.Source = static_cast<XController*>(this);
    m_xImpl->aLC.disposeAndClear(aObject);
    m_xDatMan.clear();
    m_aStatusListeners.clear();
    m_xLastQueriedFocusWin.reset();
    m_xWindow.clear();
    m_xImpl.clear();
}

void BibFrameController_Impl::addEventListener( const uno::Reference< lang::XEventListener > & aListener )
{
    m_xImpl->aLC.addInterface( cppu::UnoType<lang::XEventListener>::get(), aListener );
}

void BibFrameController_Impl::removeEventListener( const uno::Reference< lang::XEventListener > & aListener )
{
    m_xImpl->aLC.removeInterface( cppu::UnoType<lang::XEventListener>::get(), aListener );
}

uno::Reference< frame::XDispatch >  BibFrameController_Impl::queryDispatch( const util::URL& aURL, const OUString& /*aTarget*/, sal_Int32 /*nSearchFlags*/ )
{
    if ( !m_bDisposing )
    {
        const CmdToInfoCache& rCmdCache = GetCommandToInfoCache();
        CmdToInfoCache::const_iterator pIter = rCmdCache.find( aURL.Complete );
        if ( pIter != rCmdCache.end() )
        {
            if (( m_xDatMan->HasActiveConnection() ) ||
                ( !pIter->second.bActiveConnection ))
                return static_cast<frame::XDispatch*>(this);
        }
    }

    return uno::Reference< frame::XDispatch > ();
}

uno::Sequence<uno::Reference< XDispatch > > BibFrameController_Impl::queryDispatches( const uno::Sequence<DispatchDescriptor>& aDescripts )
{
    uno::Sequence< uno::Reference< XDispatch > > aDispatches( aDescripts.getLength() );
    auto aDispatchesRange = asNonConstRange(aDispatches);
    for ( sal_Int32 i=0; i<aDescripts.getLength(); ++i )
        aDispatchesRange[i] = queryDispatch( aDescripts[i].FeatureURL, aDescripts[i].FrameName, aDescripts[i].SearchFlags );
    return aDispatches;
}

uno::Sequence< ::sal_Int16 > SAL_CALL BibFrameController_Impl::getSupportedCommandGroups()
{
    uno::Sequence< ::sal_Int16 > aDispatchInfo{ frame::CommandGroup::EDIT,
                                                frame::CommandGroup::DOCUMENT,
                                                frame::CommandGroup::DATA,
                                                frame::CommandGroup::VIEW };

    return aDispatchInfo;
}

uno::Sequence< frame::DispatchInformation > SAL_CALL BibFrameController_Impl::getConfigurableDispatchInformation( ::sal_Int16 nCommandGroup )
{
    const CmdToInfoCache& rCmdCache = GetCommandToInfoCache();

    frame::DispatchInformation                  aDispatchInfo;
    std::vector< frame::DispatchInformation >   aDispatchInfoVector;

    if (( nCommandGroup == frame::CommandGroup::EDIT ) ||
        ( nCommandGroup == frame::CommandGroup::DOCUMENT ) ||
        ( nCommandGroup == frame::CommandGroup::DATA ) ||
        ( nCommandGroup == frame::CommandGroup::VIEW ))
    {
        bool bGroupFound = false;
        for (auto const& item : rCmdCache)
        {
            if ( item.second.nGroupId == nCommandGroup )
            {
                bGroupFound = true;
                aDispatchInfo.Command = item.first;
                aDispatchInfo.GroupId = item.second.nGroupId;
                aDispatchInfoVector.push_back( aDispatchInfo );
            }
            else if ( bGroupFound )
                break;
        }
    }

    return comphelper::containerToSequence( aDispatchInfoVector );
}

static bool canInsertRecords(const Reference< beans::XPropertySet>& _rxCursorSet)
{
    sal_Int32 nPriv = 0;
    _rxCursorSet->getPropertyValue(u"Privileges"_ustr) >>= nPriv;
    return _rxCursorSet.is() && (nPriv & sdbcx::Privilege::INSERT) != 0;
}

bool BibFrameController_Impl::SaveModified(const Reference< form::runtime::XFormController>& xController)
{
    if (!xController.is())
        return false;

    Reference< XResultSetUpdate> _xCursor(xController->getModel(), UNO_QUERY);

    if (!_xCursor.is())
        return false;

    Reference< beans::XPropertySet> _xSet(_xCursor, UNO_QUERY);
    if (!_xSet.is())
        return false;

    // need to save?
    bool  bIsNew        = ::comphelper::getBOOL(_xSet->getPropertyValue(u"IsNew"_ustr));
    bool  bIsModified   = ::comphelper::getBOOL(_xSet->getPropertyValue(u"IsModified"_ustr));
    bool bResult = !bIsModified;
    if (bIsModified)
    {
        try
        {
            if (bIsNew)
                _xCursor->insertRow();
            else
                _xCursor->updateRow();
            bResult = true;
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION("extensions.biblio", "");
        }
    }
    return bResult;
}

static vcl::Window* lcl_GetFocusChild( vcl::Window const * pParent )
{
    sal_uInt16 nChildren = pParent->GetChildCount();
    for( sal_uInt16 nChild = 0; nChild < nChildren; ++nChild)
    {
        vcl::Window* pChild = pParent->GetChild( nChild );
        if(pChild->HasFocus())
            return pChild;
        vcl::Window* pSubChild = lcl_GetFocusChild( pChild );
        if(pSubChild)
            return pSubChild;
    }
    return nullptr;
}

//class XDispatch
void BibFrameController_Impl::dispatch(const util::URL& _rURL, const uno::Sequence< beans::PropertyValue >& aArgs)
{
    if ( m_bDisposing )
        return;

    ::SolarMutexGuard aGuard;
    weld::Window* pParent = Application::GetFrameWeld(m_xWindow);
    weld::WaitObject aWaitObject(pParent);

    OUString aCommand( _rURL.Path);
    if(aCommand == "Bib/Mapping")
    {
        m_xDatMan->CreateMappingDialog(pParent);
    }
    else if(aCommand == "Bib/source")
    {
        ChangeDataSource(aArgs);
    }
    else if(aCommand == "Bib/sdbsource")
    {
        OUString aURL = m_xDatMan->CreateDBChangeDialog(pParent);
        if(!aURL.isEmpty())
        {
            try
            {
                uno::Sequence< beans::PropertyValue > aNewDataSource
                {
                    comphelper::makePropertyValue( {}, OUString() ),
                    comphelper::makePropertyValue( {}, aURL )
                };
                ChangeDataSource(aNewDataSource);
            }
            catch(const Exception&)
            {
                TOOLS_WARN_EXCEPTION("extensions.biblio",
                                     "Exception caught while changing the data source");
            }
        }
    }
    else if(aCommand == "Bib/autoFilter")
    {
        sal_uInt16 nCount = m_aStatusListeners.size();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            BibStatusDispatch *pObj = m_aStatusListeners[n].get();
            if ( pObj->aURL.Path == "Bib/removeFilter" )
            {
                FeatureStateEvent  aEvent;
                aEvent.FeatureURL = pObj->aURL;
                aEvent.IsEnabled  = true;
                aEvent.Requery    = false;
                aEvent.Source     = static_cast<XDispatch *>(this);
                pObj->xListener->statusChanged( aEvent );
                //break; because there are more than one
            }
        }

        OUString aQuery;
        aArgs[0].Value >>= aQuery;

        OUString aQueryField;
        aArgs[1].Value >>= aQueryField;
        BibConfig* pConfig = BibModul::GetConfig();
        pConfig->setQueryField(aQueryField);
        m_xDatMan->startQueryWith(aQuery);
    }
    else if(aCommand == "Bib/standardFilter")
    {
        try
        {
            const uno::Reference< uno::XComponentContext >& xContext = ::comphelper::getProcessComponentContext();

            // create the dialog object
            uno::Reference< ui::dialogs::XExecutableDialog > xDialog = sdb::FilterDialog::createWithQuery(xContext, m_xDatMan->getParser(),
                       Reference<sdbc::XRowSet>(m_xDatMan->getForm(), uno::UNO_QUERY_THROW), m_xWindow);
            // execute it
            if ( xDialog->execute( ) )
            {
                // the dialog has been executed successfully, and the filter on the query composer
                // has been changed
                OUString sNewFilter = m_xDatMan->getParser()->getFilter();
                m_xDatMan->setFilter( sNewFilter );
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "extensions.biblio", "BibFrameController_Impl::dispatch" );
        }

        sal_uInt16 nCount = m_aStatusListeners.size();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            BibStatusDispatch *pObj = m_aStatusListeners[n].get();
            if ( pObj->aURL.Path == "Bib/removeFilter" && m_xDatMan->getParser().is())
            {
                FeatureStateEvent  aEvent;
                aEvent.FeatureURL = pObj->aURL;
                aEvent.IsEnabled  = !m_xDatMan->getParser()->getFilter().isEmpty();
                aEvent.Requery    = false;
                aEvent.Source     = static_cast<XDispatch *>(this);
                pObj->xListener->statusChanged( aEvent );
            }
        }
    }
    else if(aCommand == "Bib/removeFilter")
    {
        RemoveFilter();
    }
    else if( _rURL.Complete == ".uno:CloseDoc" || aCommand == "CloseDoc" )
    {
        Application::PostUserEvent( LINK( this, BibFrameController_Impl,
                                    DisposeHdl ) );

    }
    else if(aCommand == "Bib/InsertRecord")
    {
        Reference<form::runtime::XFormController > xFormCtrl = m_xDatMan->GetFormController();
        if(SaveModified(xFormCtrl))
        {
            try
            {
                Reference< sdbc::XResultSet >  xCursor( m_xDatMan->getForm(), UNO_QUERY );
                xCursor->last();

                Reference< XResultSetUpdate >  xUpdateCursor( m_xDatMan->getForm(), UNO_QUERY );
                xUpdateCursor->moveToInsertRow();
            }
            catch(const Exception&)
            {
                TOOLS_WARN_EXCEPTION("extensions.biblio",
                                     "Exception in last() or moveToInsertRow()");
            }
        }
    }
    else if(aCommand == "Bib/DeleteRecord")
    {
        Reference< css::sdbc::XResultSet >  xCursor(m_xDatMan->getForm(), UNO_QUERY);
        Reference< XResultSetUpdate >       xUpdateCursor(xCursor, UNO_QUERY);
        Reference< beans::XPropertySet >    xSet(m_xDatMan->getForm(), UNO_QUERY);
        bool  bIsNew  = ::comphelper::getBOOL(xSet->getPropertyValue(u"IsNew"_ustr));
        if(!bIsNew)
        {
            sal_uInt32 nCount = 0;
            xSet->getPropertyValue(u"RowCount"_ustr) >>= nCount;
            // determine next position
            bool bSuccess = false;
            bool bLeft = false;
            bool bRight = false;
            try
            {
                bLeft = xCursor->isLast() && nCount > 1;
                bRight= !xCursor->isLast();
                // ask for confirmation
                Reference< form::XConfirmDeleteListener >  xConfirm(m_xDatMan->GetFormController(),UNO_QUERY);
                if (xConfirm.is())
                {
                    sdb::RowChangeEvent aEvent;
                    aEvent.Source.set(xCursor, UNO_QUERY);
                    aEvent.Action = sdb::RowChangeAction::DELETE;
                    aEvent.Rows = 1;
                    bSuccess = xConfirm->confirmDelete(aEvent);
                }

                // delete it
                if (bSuccess)
                    xUpdateCursor->deleteRow();
            }
            catch(const Exception&)
            {
                bSuccess = false;
            }
            if (bSuccess)
            {
                if (bLeft || bRight)
                    xCursor->relative(bRight ? 1 : -1);
                else
                {
                    bool bCanInsert = canInsertRecords(xSet);
                    // can another entry be inserted?
                    try
                    {
                        if (bCanInsert)
                            xUpdateCursor->moveToInsertRow();
                        else
                            // move data entry to reset state
                            xCursor->first();
                    }
                    catch(const Exception&)
                    {
                        TOOLS_WARN_EXCEPTION("extensions.biblio",
                                             "DeleteRecord: exception caught!");
                    }
                }
            }
        }
    }
    else if(aCommand == "Cut")
    {
        vcl::Window* pChild = m_xLastQueriedFocusWin.get();
        if(pChild)
        {
            KeyEvent aEvent( 0, KeyFuncType::CUT );
            pChild->KeyInput( aEvent );
        }
    }
    else if(aCommand == "Copy")
    {
        vcl::Window* pChild = m_xLastQueriedFocusWin.get();
        if(pChild)
        {
            KeyEvent aEvent( 0, KeyFuncType::COPY );
            pChild->KeyInput( aEvent );
        }
    }
    else if(aCommand == "Paste")
    {
        vcl::Window* pChild = m_xLastQueriedFocusWin.get();
        if(pChild)
        {
            KeyEvent aEvent( 0, KeyFuncType::PASTE );
            pChild->KeyInput( aEvent );
        }
    }
}
IMPL_LINK_NOARG( BibFrameController_Impl, DisposeHdl, void*, void )
{
    if (m_xFrame.is())
        m_xFrame->dispose();
};

void BibFrameController_Impl::addStatusListener(
    const uno::Reference< frame::XStatusListener > & aListener,
    const util::URL& aURL)
{
    BibConfig* pConfig = BibModul::GetConfig();
    // create a new Reference and insert into listener array
    m_aStatusListeners.push_back( std::make_unique<BibStatusDispatch>( aURL, aListener ) );

    // send first status synchronously
    FeatureStateEvent aEvent;
    aEvent.FeatureURL = aURL;
    aEvent.Requery    = false;
    aEvent.Source     = static_cast<XDispatch *>(this);
    if ( aURL.Path == "StatusBarVisible" )
    {
        aEvent.IsEnabled  = false;
        aEvent.State <<= false;
    }
    else if ( aURL.Path == "Bib/hierarchical" )
    {
        aEvent.IsEnabled  = true;
        aEvent.State <<= OUString();
    }
    else if(aURL.Path == "Bib/MenuFilter")
    {
        aEvent.IsEnabled  = true;
        aEvent.FeatureDescriptor=m_xDatMan->getQueryField();

        aEvent.State <<= m_xDatMan->getQueryFields();

    }
    else if ( aURL.Path == "Bib/source")
    {
        aEvent.IsEnabled  = true;
        aEvent.FeatureDescriptor=m_xDatMan->getActiveDataTable();

        aEvent.State <<= m_xDatMan->getDataSources();
    }
    else if( aURL.Path == "Bib/sdbsource" ||
             aURL.Path == "Bib/Mapping" ||
             aURL.Path == "Bib/autoFilter" ||
             aURL.Path == "Bib/standardFilter" )
    {
        aEvent.IsEnabled  = true;
    }
    else if(aURL.Path == "Bib/query")
    {
        aEvent.IsEnabled  = true;
        aEvent.State <<= pConfig->getQueryText();
    }
    else if (aURL.Path == "Bib/removeFilter" )
    {
        OUString aFilterStr=m_xDatMan->getFilter();
        aEvent.IsEnabled  = !aFilterStr.isEmpty();
    }
    else if(aURL.Path == "Cut")
    {
        m_xLastQueriedFocusWin = lcl_GetFocusChild( VCLUnoHelper::GetWindow( m_xWindow ) );
        if (m_xLastQueriedFocusWin)
        {
            Reference<css::awt::XTextComponent> xEdit(m_xLastQueriedFocusWin->GetComponentInterface(), css::uno::UNO_QUERY);
            aEvent.IsEnabled = xEdit && xEdit->isEditable() && !xEdit->getSelectedText().isEmpty();
        }
    }
    if(aURL.Path == "Copy")
    {
        m_xLastQueriedFocusWin = lcl_GetFocusChild( VCLUnoHelper::GetWindow( m_xWindow ) );
        if (m_xLastQueriedFocusWin)
        {
            Reference<css::awt::XTextComponent> xEdit(m_xLastQueriedFocusWin->GetComponentInterface(), css::uno::UNO_QUERY);
            aEvent.IsEnabled = xEdit && !xEdit->getSelectedText().isEmpty();
        }
    }
    else if(aURL.Path == "Paste" )
    {
        aEvent.IsEnabled = false;
        m_xLastQueriedFocusWin = lcl_GetFocusChild( VCLUnoHelper::GetWindow( m_xWindow ) );
        if (m_xLastQueriedFocusWin)
        {
            Reference<css::awt::XTextComponent> xEdit(m_xLastQueriedFocusWin->GetComponentInterface(), css::uno::UNO_QUERY);
            if (xEdit && !xEdit->isEditable())
            {
                uno::Reference< datatransfer::clipboard::XClipboard > xClip = m_xLastQueriedFocusWin->GetClipboard();
                if(xClip.is())
                {
                    uno::Reference< datatransfer::XTransferable > xDataObj;

                    try
                        {
                            SolarMutexReleaser aReleaser;
                            xDataObj = xClip->getContents();
                        }
                    catch( const uno::Exception& )
                        {
                        }

                    if ( xDataObj.is() )
                    {
                        datatransfer::DataFlavor aFlavor;
                        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
                        try
                        {
                            uno::Any aData = xDataObj->getTransferData( aFlavor );
                            OUString aText;
                            aData >>= aText;
                            aEvent.IsEnabled  = !aText.isEmpty();
                        }
                        catch( const uno::Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
    else if(aURL.Path == "Bib/DeleteRecord")
    {
        Reference< beans::XPropertySet >    xSet(m_xDatMan->getForm(), UNO_QUERY);
        bool  bIsNew  = ::comphelper::getBOOL(xSet->getPropertyValue(u"IsNew"_ustr));
        if(!bIsNew)
        {
            sal_uInt32 nCount = 0;
            xSet->getPropertyValue(u"RowCount"_ustr) >>= nCount;
            aEvent.IsEnabled  = nCount > 0;
        }
    }
    else if (aURL.Path == "Bib/InsertRecord")
    {
        Reference< beans::XPropertySet >  xSet(m_xDatMan->getForm(), UNO_QUERY);
        aEvent.IsEnabled = canInsertRecords(xSet);
    }
    aListener->statusChanged( aEvent );
}

void BibFrameController_Impl::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & aObject, const util::URL& aURL)
{
    // search listener array for given listener
    // for checking equality always "cast" to XInterface
    if ( m_bDisposing )
        return;

    sal_uInt16 nCount = m_aStatusListeners.size();
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = m_aStatusListeners[n].get();
        bool bFlag=pObj->xListener.is();
        if (!bFlag || (pObj->xListener == aObject &&
            ( aURL.Complete.isEmpty() || pObj->aURL.Path == aURL.Path  )))
        {
            m_aStatusListeners.erase( m_aStatusListeners.begin() + n );
            break;
        }
    }
}

void BibFrameController_Impl::RemoveFilter()
{
    OUString aQuery;
    m_xDatMan->startQueryWith(aQuery);

    sal_uInt16 nCount = m_aStatusListeners.size();

    bool bRemoveFilter=false;
    bool bQueryText=false;

    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = m_aStatusListeners[n].get();
        if ( pObj->aURL.Path == "Bib/removeFilter" )
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = false;
            aEvent.Requery    = false;
            aEvent.Source     = static_cast<XDispatch *>(this);
            pObj->xListener->statusChanged( aEvent );
            bRemoveFilter=true;
        }
        else if(pObj->aURL.Path == "Bib/query")
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = true;
            aEvent.Requery    = false;
            aEvent.Source     = static_cast<XDispatch *>(this);
            aEvent.State <<= aQuery;
            pObj->xListener->statusChanged( aEvent );
            bQueryText=true;
        }

        if(bRemoveFilter && bQueryText)
            break;

    }
}

void BibFrameController_Impl::ChangeDataSource(const uno::Sequence< beans::PropertyValue >& aArgs)
{
    uno::Any aValue = aArgs[0].Value;
    OUString aDBTableName;
    aValue >>= aDBTableName;


    if(aArgs.getLength() > 1)
    {
        uno::Any aDB = aArgs[1].Value;
        OUString aURL;
        aDB >>= aURL;
        m_xDatMan->setActiveDataSource(aURL);
        aDBTableName = m_xDatMan->getActiveDataTable();
    }
    else
    {
        m_xDatMan->unload();
        m_xDatMan->setActiveDataTable(aDBTableName);
        m_xDatMan->updateGridModel();
        m_xDatMan->load();
    }


    sal_uInt16 nCount = m_aStatusListeners.size();

    bool bMenuFilter=false;
    bool bQueryText=false;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = m_aStatusListeners[n].get();
        if (pObj->aURL.Path == "Bib/MenuFilter")
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = true;
            aEvent.Requery    = false;
            aEvent.Source     = static_cast<XDispatch *>(this);
            aEvent.FeatureDescriptor=m_xDatMan->getQueryField();

            uno::Sequence<OUString> aStringSeq=m_xDatMan->getQueryFields();
            aEvent.State <<= aStringSeq;

            pObj->xListener->statusChanged( aEvent );
            bMenuFilter=true;
        }
        else if (pObj->aURL.Path == "Bib/query")
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = true;
            aEvent.Requery    = false;
            aEvent.Source     = static_cast<XDispatch *>(this);
            BibConfig* pConfig = BibModul::GetConfig();
            aEvent.State <<= pConfig->getQueryText();
            pObj->xListener->statusChanged( aEvent );
            bQueryText=true;
        }

        if (bMenuFilter && bQueryText)
            break;

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
