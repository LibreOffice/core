/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <vcl/waitobj.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/util/URL.hpp>
#include <vcl/msgbox.hxx>
#include <tools/debug.hxx>
#include <vcl/stdtext.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequence.hxx>
#include "framectr.hxx"
#include "datman.hxx"
#include "bibresid.hxx"
#include "bib.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include "bibconfig.hxx"
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <vcl/svapp.hxx>
#include "bibliography.hrc"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <vcl/edit.hxx>
#include <osl/mutex.hxx>

#include <boost/unordered_map.hpp>

using namespace osl;
using namespace cppu;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star;

using ::rtl::OUString;

#define C2U(cChar) OUString::createFromAscii(cChar)

struct DispatchInfo
{
    const char*   pCommand;
    sal_Int16     nGroupId;
    sal_Bool      bActiveConnection;
};

struct CacheDispatchInfo
{
    sal_Int16     nGroupId;
    sal_Bool      bActiveConnection;
};

// Attention: commands must be sorted by command groups. Implementation is dependent
// on this!!
static DispatchInfo SupportedCommandsArray[] =
{
    { ".uno:Undo"               ,   frame::CommandGroup::EDIT       , sal_False },
    { ".uno:Cut"                ,   frame::CommandGroup::EDIT       , sal_False },
    { ".uno:Copy"               ,   frame::CommandGroup::EDIT       , sal_False },
    { ".uno:Paste"              ,   frame::CommandGroup::EDIT       , sal_False },
    { ".uno:SelectAll"          ,   frame::CommandGroup::EDIT       , sal_False },
    { ".uno:CloseDoc"           ,   frame::CommandGroup::DOCUMENT   , sal_False },
    { ".uno:StatusBarVisible"   ,   frame::CommandGroup::VIEW       , sal_False },
    { ".uno:AvailableToolbars"  ,   frame::CommandGroup::VIEW       , sal_False },
    { ".uno:Bib/standardFilter" ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/DeleteRecord"   ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/InsertRecord"   ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/query"          ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/autoFilter"     ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/source"         ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/removeFilter"   ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/sdbsource"      ,   frame::CommandGroup::DATA       , sal_True  },
    { ".uno:Bib/Mapping"        ,   frame::CommandGroup::DATA       , sal_True  },
    { 0                         ,   0                               , sal_False }
};

typedef ::boost::unordered_map< ::rtl::OUString, CacheDispatchInfo, rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > CmdToInfoCache;

const CmdToInfoCache& GetCommandToInfoCache()
{
    static sal_Bool       bCacheInitialized = sal_False;
    static CmdToInfoCache aCmdToInfoCache;

    if ( !bCacheInitialized )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !bCacheInitialized )
        {
            sal_Int32 i( 0 );
            while ( SupportedCommandsArray[i].pCommand != 0 )
            {
                rtl::OUString aCommand( rtl::OUString::createFromAscii( SupportedCommandsArray[i].pCommand ));

                CacheDispatchInfo aDispatchInfo;
                aDispatchInfo.nGroupId          = SupportedCommandsArray[i].nGroupId;
                aDispatchInfo.bActiveConnection = SupportedCommandsArray[i].bActiveConnection;
                aCmdToInfoCache.insert( CmdToInfoCache::value_type( aCommand, aDispatchInfo ));
                ++i;
            }
            bCacheInitialized = sal_True;
        }
    }

    return aCmdToInfoCache;
}


class BibFrameCtrl_Impl : public cppu::WeakImplHelper1 < XFrameActionListener >
{
public:
    Mutex                               aMutex;
    OMultiTypeInterfaceContainerHelper  aLC;

    BibFrameController_Impl*            pController;

                                        BibFrameCtrl_Impl()
                                            : aLC( aMutex )
                                            , pController(0)
                                        {}

                                        ~BibFrameCtrl_Impl();

    virtual void                        SAL_CALL frameAction(const FrameActionEvent& aEvent) throw( RuntimeException );
    virtual void                        SAL_CALL disposing( const lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
};


BibFrameCtrl_Impl::~BibFrameCtrl_Impl()
{
}

void BibFrameCtrl_Impl::frameAction(const FrameActionEvent& aEvent) throw( uno::RuntimeException )
{
    if ( pController && aEvent.Frame == pController->getFrame())
    {
        if(aEvent.Action == FrameAction_FRAME_ACTIVATED)
        {
            pController->activate();
        }
        else if(aEvent.Action == FrameAction_FRAME_DEACTIVATING)
        {
            pController->deactivate();
        }
    }
}

void BibFrameCtrl_Impl::disposing( const lang::EventObject& /*Source*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;
    if ( pController )
        pController->getFrame()->removeFrameActionListener( this );
}

BibFrameController_Impl::BibFrameController_Impl( const uno::Reference< awt::XWindow > & xComponent,
                                                BibDataManager* pDataManager)
    :xWindow( xComponent )
    ,m_xDatMan( pDataManager )
    ,pDatMan( pDataManager )
    ,pBibMod(NULL)
{
    Window* pParent = VCLUnoHelper::GetWindow( xWindow );
    pParent->SetUniqueId(UID_BIB_FRAME_WINDOW);
    bDisposing=sal_False;
    bHierarchical=sal_True;
    pImp = new BibFrameCtrl_Impl;
    pImp->pController = this;
    pImp->acquire();
}

BibFrameController_Impl::~BibFrameController_Impl()
{
    pImp->pController = NULL;
    pImp->release();
    delete pDatMan;
    if(pBibMod)
        CloseBibModul(pBibMod);
}

::rtl::OUString SAL_CALL BibFrameController_Impl::getImplementationName() throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.Bibliography"));
}

sal_Bool SAL_CALL BibFrameController_Impl::supportsService( const ::rtl::OUString& sServiceName ) throw (::com::sun::star::uno::RuntimeException)
{
    return ( sServiceName == "com.sun.star.frame.Bibliography" || sServiceName == "com.sun.star.frame.Controller" );
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL BibFrameController_Impl::getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException)
{
    // return only top level services ...
    // base services are included there and should be asked by uno-rtti.
    ::com::sun::star::uno::Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Bibliography"));
    return lNames;
}

void BibFrameController_Impl::attachFrame( const uno::Reference< XFrame > & xArg ) throw (::com::sun::star::uno::RuntimeException)
{
    xFrame = xArg;
    xFrame->addFrameActionListener( pImp );
}

sal_Bool BibFrameController_Impl::attachModel( const uno::Reference< XModel > & /*xModel*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

sal_Bool BibFrameController_Impl::suspend( sal_Bool bSuspend ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( bSuspend )
        getFrame()->removeFrameActionListener( pImp );
    else
        getFrame()->addFrameActionListener( pImp );
    return sal_True;
}

uno::Any BibFrameController_Impl::getViewData() throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Any();
}

void BibFrameController_Impl::restoreViewData( const uno::Any& /*Value*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

uno::Reference< XFrame >  BibFrameController_Impl::getFrame() throw (::com::sun::star::uno::RuntimeException)
{
    return xFrame;
}

uno::Reference< XModel >  BibFrameController_Impl::getModel() throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference< XModel > ();
}

void BibFrameController_Impl::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    bDisposing = sal_True;
    lang::EventObject aObject;
    aObject.Source = (XController*)this;
    pImp->aLC.disposeAndClear(aObject);
    m_xDatMan = 0;
    pDatMan = 0;
    aStatusListeners.clear();
 }

void BibFrameController_Impl::addEventListener( const uno::Reference< lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    pImp->aLC.addInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

void BibFrameController_Impl::removeEventListener( const uno::Reference< lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    pImp->aLC.removeInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

uno::Reference< frame::XDispatch >  BibFrameController_Impl::queryDispatch( const util::URL& aURL, const rtl::OUString& /*aTarget*/, sal_Int32 /*nSearchFlags*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !bDisposing )
    {
        const CmdToInfoCache& rCmdCache = GetCommandToInfoCache();
        CmdToInfoCache::const_iterator pIter = rCmdCache.find( aURL.Complete );
        if ( pIter != rCmdCache.end() )
        {
            if (( pDatMan->HasActiveConnection() ) ||
                ( !pIter->second.bActiveConnection ))
                return (frame::XDispatch*) this;
        }
    }

    return uno::Reference< frame::XDispatch > ();
}

uno::Sequence<uno::Reference< XDispatch > > BibFrameController_Impl::queryDispatches( const uno::Sequence<DispatchDescriptor>& aDescripts ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< uno::Reference< XDispatch > > aDispatches( aDescripts.getLength() );
    for ( sal_Int32 i=0; i<aDescripts.getLength(); ++i )
        aDispatches[i] = queryDispatch( aDescripts[i].FeatureURL, aDescripts[i].FrameName, aDescripts[i].SearchFlags );
    return aDispatches;
}

uno::Sequence< ::sal_Int16 > SAL_CALL BibFrameController_Impl::getSupportedCommandGroups()
throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence< ::sal_Int16 > aDispatchInfo( 4 );

    aDispatchInfo[0] = frame::CommandGroup::EDIT;
    aDispatchInfo[1] = frame::CommandGroup::DOCUMENT;
    aDispatchInfo[2] = frame::CommandGroup::DATA;
    aDispatchInfo[3] = frame::CommandGroup::VIEW;

    return aDispatchInfo;
}

uno::Sequence< frame::DispatchInformation > SAL_CALL BibFrameController_Impl::getConfigurableDispatchInformation( ::sal_Int16 nCommandGroup )
throw (::com::sun::star::uno::RuntimeException)
{
    const CmdToInfoCache& rCmdCache = GetCommandToInfoCache();

    sal_Bool                                    bGroupFound( sal_False );
    frame::DispatchInformation                  aDispatchInfo;
    std::list< frame::DispatchInformation >     aDispatchInfoList;

    if (( nCommandGroup == frame::CommandGroup::EDIT ) ||
        ( nCommandGroup == frame::CommandGroup::DOCUMENT ) ||
        ( nCommandGroup == frame::CommandGroup::DATA ) ||
        ( nCommandGroup == frame::CommandGroup::VIEW ))
    {
        CmdToInfoCache::const_iterator pIter = rCmdCache.begin();
        while ( pIter != rCmdCache.end() )
        {
            if ( pIter->second.nGroupId == nCommandGroup )
            {
                bGroupFound = sal_True;
                aDispatchInfo.Command = pIter->first;
                aDispatchInfo.GroupId = pIter->second.nGroupId;
                aDispatchInfoList.push_back( aDispatchInfo );
            }
            else if ( bGroupFound )
                break;

            ++pIter;
        }
    }

    ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchInformation > aSeq =
        comphelper::containerToSequence< ::com::sun::star::frame::DispatchInformation, std::list< ::com::sun::star::frame::DispatchInformation > >( aDispatchInfoList );

    return aSeq;
}

sal_Bool canInsertRecords(const Reference< beans::XPropertySet>& _rxCursorSet)
{
    sal_Int32 nPriv = 0;
    _rxCursorSet->getPropertyValue(C2U("Privileges")) >>= nPriv;
    return ((_rxCursorSet.is() && (nPriv & sdbcx::Privilege::INSERT) != 0));
}

sal_Bool BibFrameController_Impl::SaveModified(const Reference< form::runtime::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    Reference< XResultSetUpdate> _xCursor = Reference< XResultSetUpdate>(xController->getModel(), UNO_QUERY);

    if (!_xCursor.is())
        return sal_False;

    Reference< beans::XPropertySet> _xSet = Reference< beans::XPropertySet>(_xCursor, UNO_QUERY);
    if (!_xSet.is())
        return sal_False;

    // need to save?
    sal_Bool  bIsNew        = ::comphelper::getBOOL(_xSet->getPropertyValue(C2U("IsNew")));
    sal_Bool  bIsModified   = ::comphelper::getBOOL(_xSet->getPropertyValue(C2U("IsModified")));
    sal_Bool bResult = !bIsModified;
    if (bIsModified)
    {
        try
        {
            if (bIsNew)
                _xCursor->insertRow();
            else
                _xCursor->updateRow();
            bResult = sal_True;
        }
        catch(const Exception&)
        {
            OSL_FAIL("SaveModified: Exception occurred!");
        }
    }
    return bResult;
}

static Window* lcl_GetFocusChild( Window* pParent )
{
    sal_uInt16 nChildren = pParent->GetChildCount();
    for( sal_uInt16 nChild = 0; nChild < nChildren; ++nChild)
    {
        Window* pChild = pParent->GetChild( nChild );
        if(pChild->HasFocus())
            return pChild;
        Window* pSubChild = lcl_GetFocusChild( pChild );
        if(pSubChild)
            return pSubChild;
    }
    return 0;
}

//class XDispatch
void BibFrameController_Impl::dispatch(const util::URL& _rURL, const uno::Sequence< beans::PropertyValue >& aArgs) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !bDisposing )
    {
        ::SolarMutexGuard aGuard;
        Window* pParent = VCLUnoHelper::GetWindow( xWindow );
        WaitObject aWaitObject( pParent );

        String aCommand( _rURL.Path);
        if(aCommand.EqualsAscii("Bib/Mapping"))
        {
            pDatMan->CreateMappingDialog(pParent);
        }
        else if(aCommand.EqualsAscii("Bib/source"))
        {
            ChangeDataSource(aArgs);
        }
        else if(aCommand.EqualsAscii("Bib/sdbsource"))
        {
            rtl::OUString aURL = pDatMan->CreateDBChangeDialog(pParent);
            if(!aURL.isEmpty())
            {
                try
                {
                    uno::Sequence< beans::PropertyValue > aNewDataSource(2);
                    beans::PropertyValue* pProps = aNewDataSource.getArray();
                    pProps[0].Value <<= rtl::OUString();
                    pProps[1].Value <<= aURL;
                    ChangeDataSource(aNewDataSource);
                }
                catch(const Exception&)
                {
                    OSL_FAIL("Exception catched while changing the data source");
                }
            }
        }
        else if(aCommand.EqualsAscii("Bib/autoFilter"))
        {
            sal_uInt16 nCount = aStatusListeners.size();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                BibStatusDispatch *pObj = &aStatusListeners[n];
                if ( pObj->aURL.Path == C2U("Bib/removeFilter") )
                {
                    FeatureStateEvent  aEvent;
                    aEvent.FeatureURL = pObj->aURL;
                    aEvent.IsEnabled  = sal_True;
                    aEvent.Requery    = sal_False;
                    aEvent.Source     = (XDispatch *) this;
                    pObj->xListener->statusChanged( aEvent );
                    //break; because there are more than one
                }
            }

            const beans::PropertyValue* pPropertyValue = aArgs.getConstArray();
            uno::Any aValue=pPropertyValue[0].Value;
            rtl::OUString aQuery;
            aValue >>= aQuery;

            aValue=pPropertyValue[1].Value;
            rtl::OUString aQueryField;
            aValue >>= aQueryField;
            BibConfig* pConfig = BibModul::GetConfig();
            pConfig->setQueryField(aQueryField);
            pDatMan->startQueryWith(aQuery);
        }
        else if(aCommand.EqualsAscii("Bib/standardFilter"))
        {
            try
            {
                uno::Reference< lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();

                // build the arguments for the filter dialog to be created
                Sequence< Any > aDialogCreationArgs( 3 );
                Any* pDialogCreationArgs = aDialogCreationArgs.getArray();
                // the query composer
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "QueryComposer" )),
                                                        -1,
                                                        makeAny( pDatMan->getParser() ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );

                // the rowset
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowSet" )),
                                                        -1,
                                                        makeAny( pDatMan->getForm() ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );
                // the parent window for the dialog
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" )),
                                                        -1,
                                                        makeAny( xWindow ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );

                // create the dialog object
                const ::rtl::OUString sDialogServiceName(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.FilterDialog" ));
                uno::Reference< uno::XInterface > xDialog = xORB->createInstanceWithArguments(
                    sDialogServiceName,
                    aDialogCreationArgs
                );
                if ( !xDialog.is() )
                {
                    ShowServiceNotAvailableError( VCLUnoHelper::GetWindow( xWindow ), sDialogServiceName, sal_True );
                }
                else
                {
                    // execute it
                    uno::Reference< ui::dialogs::XExecutableDialog > xExec( xDialog, UNO_QUERY );
                    DBG_ASSERT( xExec.is(), "BibFrameController_Impl::dispatch: missing an interface on the dialog!" );
                    if ( xExec.is() )
                        if ( xExec->execute( ) )
                        {
                            // the dialog has been executed successfully, and the filter on the query composer
                            // has been changed
                            ::rtl::OUString sNewFilter = pDatMan->getParser()->getFilter();
                            pDatMan->setFilter( sNewFilter );
                        }
                }
            }
            catch( const uno::Exception& )
            {
                OSL_FAIL( "BibFrameController_Impl::dispatch: caught an exception!" );
            }

            sal_uInt16 nCount = aStatusListeners.size();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                BibStatusDispatch *pObj = &aStatusListeners[n];
                if ( pObj->aURL.Path == C2U("Bib/removeFilter") && pDatMan->getParser().is())
                {
                    FeatureStateEvent  aEvent;
                    aEvent.FeatureURL = pObj->aURL;
                    aEvent.IsEnabled  = !pDatMan->getParser()->getFilter().isEmpty();
                    aEvent.Requery    = sal_False;
                    aEvent.Source     = (XDispatch *) this;
                    pObj->xListener->statusChanged( aEvent );
                }
            }
        }
        else if(aCommand.EqualsAscii("Bib/removeFilter"))
        {
            RemoveFilter();
        }
        else if( _rURL.Complete == "slot:5503" || aCommand.EqualsAscii("CloseDoc") )
        {
            Application::PostUserEvent( STATIC_LINK( this, BibFrameController_Impl,
                                        DisposeHdl ), 0 );

        }
        else if(aCommand.EqualsAscii("Bib/InsertRecord"))
        {
            Reference<form::runtime::XFormController > xFormCtrl = pDatMan->GetFormController();
            if(SaveModified(xFormCtrl))
            {
                try
                {
                    Reference< sdbc::XResultSet >  xCursor( pDatMan->getForm(), UNO_QUERY );
                    xCursor->last();

                    Reference< XResultSetUpdate >  xUpdateCursor( pDatMan->getForm(), UNO_QUERY );
                    xUpdateCursor->moveToInsertRow();
                }
                catch(const Exception&)
                {
                    OSL_FAIL("Exception in last() or moveToInsertRow()");
                }
            }
        }
        else if(aCommand.EqualsAscii("Bib/DeleteRecord"))
        {
            Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(pDatMan->getForm(), UNO_QUERY);
            Reference< XResultSetUpdate >  xUpdateCursor(xCursor, UNO_QUERY);
            Reference< beans::XPropertySet >  xSet(pDatMan->getForm(), UNO_QUERY);
            sal_Bool  bIsNew  = ::comphelper::getBOOL(xSet->getPropertyValue(C2U("IsNew")));
            if(!bIsNew)
            {
                sal_uInt32 nCount = 0;
                xSet->getPropertyValue(C2U("RowCount")) >>= nCount;
                // naechste position festellen
                sal_Bool bSuccess = sal_False;
                sal_Bool bLeft = sal_False;
                sal_Bool bRight = sal_False;
                try
                {
                    bLeft = xCursor->isLast() && nCount > 1;
                    bRight= !xCursor->isLast();
                    // ask for confirmation
                    Reference< frame::XController > xCtrl = pImp->pController;
                    Reference< form::XConfirmDeleteListener >  xConfirm(pDatMan->GetFormController(),UNO_QUERY);
                    if (xConfirm.is())
                    {
                        sdb::RowChangeEvent aEvent;
                        aEvent.Source = Reference< XInterface > (xCursor, UNO_QUERY);
                        aEvent.Action = sdb::RowChangeAction::DELETE;
                        aEvent.Rows = 1;
                        bSuccess = xConfirm->confirmDelete(aEvent);
                    }

                    // das Ding loeschen
                    if (bSuccess)
                        xUpdateCursor->deleteRow();
                }
                catch(const Exception&)
                {
                    bSuccess = sal_False;
                }
                if (bSuccess)
                {
                    if (bLeft || bRight)
                        xCursor->relative(bRight ? 1 : -1);
                    else
                    {
                        sal_Bool bCanInsert = canInsertRecords(xSet);
                        // kann noch ein Datensatz eingefuegt weden
                        try
                        {
                            if (bCanInsert)
                                xUpdateCursor->moveToInsertRow();
                            else
                                // Datensatz bewegen um Stati neu zu setzen
                                xCursor->first();
                        }
                        catch(const Exception&)
                        {
                            OSL_FAIL("DeleteRecord : exception caught !");
                        }
                    }
                }
            }
        }
        else if(aCommand.EqualsAscii("Cut"))
        {
            Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
            if(pChild)
            {
                KeyEvent aEvent( 0, KEYFUNC_CUT );
                pChild->KeyInput( aEvent );
            }
        }
        else if(aCommand.EqualsAscii("Copy"))
        {
            Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
            if(pChild)
            {
                KeyEvent aEvent( 0, KEYFUNC_COPY );
                pChild->KeyInput( aEvent );
            }
        }
        else if(aCommand.EqualsAscii("Paste"))
        {
            Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
            if(pChild)
            {
                KeyEvent aEvent( 0, KEYFUNC_PASTE );
                pChild->KeyInput( aEvent );
            }
        }
    }
}
IMPL_STATIC_LINK( BibFrameController_Impl, DisposeHdl, void*, EMPTYARG )
{
    pThis->xFrame->dispose();
    return 0;
};

//-----------------------------------------------------------------------------
void BibFrameController_Impl::addStatusListener(
    const uno::Reference< frame::XStatusListener > & aListener,
    const util::URL& aURL)
    throw (::com::sun::star::uno::RuntimeException)
{
    BibConfig* pConfig = BibModul::GetConfig();
    // create a new Reference and insert into listener array
    aStatusListeners.push_back( new BibStatusDispatch( aURL, aListener ) );

    // den ersten Status synchron zusenden
    FeatureStateEvent aEvent;
    aEvent.FeatureURL = aURL;
    aEvent.Requery    = sal_False;
    aEvent.Source     = (XDispatch *) this;
    if ( aURL.Path == C2U("StatusBarVisible") )
    {
        aEvent.IsEnabled  = sal_False;
        aEvent.State <<= sal_Bool( sal_False );
    }
    else if ( aURL.Path == C2U("Bib/hierarchical") )
    {
        aEvent.IsEnabled  = sal_True;
        const char*  pHier = bHierarchical? "" : "*" ;
        aEvent.State <<= rtl::OUString::createFromAscii(pHier);
    }
    else if(aURL.Path == C2U("Bib/MenuFilter"))
    {
        aEvent.IsEnabled  = sal_True;
        aEvent.FeatureDescriptor=pDatMan->getQueryField();

        uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getQueryFields();
        aEvent.State.setValue(&aStringSeq,::getCppuType((uno::Sequence<rtl::OUString>*)0));

    }
    else if ( aURL.Path == C2U("Bib/source"))
    {
        aEvent.IsEnabled  = sal_True;
        aEvent.FeatureDescriptor=pDatMan->getActiveDataTable();

        uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getDataSources();
        aEvent.State.setValue(&aStringSeq,::getCppuType((uno::Sequence<rtl::OUString>*)0));
    }
    else if( aURL.Path == "Bib/sdbsource" ||
             aURL.Path == "Bib/Mapping" ||
             aURL.Path == "Bib/autoFilter" ||
             aURL.Path == "Bib/standardFilter" )
    {
        aEvent.IsEnabled  = sal_True;
    }
    else if(aURL.Path == C2U("Bib/query"))
    {
        aEvent.IsEnabled  = sal_True;
        aEvent.State <<= pConfig->getQueryText();
    }
    else if (aURL.Path == C2U("Bib/removeFilter") )
    {
        rtl::OUString aFilterStr=pDatMan->getFilter();
        aEvent.IsEnabled  = !aFilterStr.isEmpty();
    }
    else if(aURL.Path == C2U("Cut"))
    {
        Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
        Edit* pEdit = dynamic_cast<Edit*>( pChild );
        if( pEdit )
            aEvent.IsEnabled  = !pEdit->IsReadOnly() && pEdit->GetSelection().Len();
    }
    if(aURL.Path == C2U("Copy"))
    {
        Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
        Edit* pEdit = dynamic_cast<Edit*>( pChild );
        if( pEdit )
            aEvent.IsEnabled  = pEdit->GetSelection().Len() > 0;
    }
    else if(aURL.Path == C2U("Paste") )
    {
        aEvent.IsEnabled  = sal_False;
        Window* pChild = lcl_GetFocusChild( VCLUnoHelper::GetWindow( xWindow ) );
        if(pChild)
        {
            uno::Reference< datatransfer::clipboard::XClipboard > xClip = pChild->GetClipboard();
            if(xClip.is())
            {
                uno::Reference< datatransfer::XTransferable > xDataObj;
                const sal_uInt32 nRef = Application::ReleaseSolarMutex();
                try
                {
                    xDataObj = xClip->getContents();
                }
                catch( const uno::Exception& )
                {
                }
                Application::AcquireSolarMutex( nRef );

                if ( xDataObj.is() )
                {
                    datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
                    try
                    {
                        uno::Any aData = xDataObj->getTransferData( aFlavor );
                        ::rtl::OUString aText;
                        aData >>= aText;
                        aEvent.IsEnabled  = !aText.isEmpty();
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }
            }
            uno::Reference< datatransfer::XTransferable > xContents = xClip->getContents(  );
        }
    }
    else if(aURL.Path == C2U("Bib/DeleteRecord"))
    {
        Reference< ::com::sun::star::sdbc::XResultSet >  xCursor(pDatMan->getForm(), UNO_QUERY);
        Reference< XResultSetUpdate >  xUpdateCursor(xCursor, UNO_QUERY);
        Reference< beans::XPropertySet >  xSet(pDatMan->getForm(), UNO_QUERY);
        sal_Bool  bIsNew  = ::comphelper::getBOOL(xSet->getPropertyValue(C2U("IsNew")));
        if(!bIsNew)
        {
            sal_uInt32 nCount = 0;
            xSet->getPropertyValue(C2U("RowCount")) >>= nCount;
            aEvent.IsEnabled  = nCount > 0;
        }
    }
    else if (aURL.Path == C2U("Bib/InsertRecord"))
    {
        Reference< beans::XPropertySet >  xSet(pDatMan->getForm(), UNO_QUERY);
        aEvent.IsEnabled = canInsertRecords(xSet);
    }
    aListener->statusChanged( aEvent );
}
//-----------------------------------------------------------------------------
void BibFrameController_Impl::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & aObject, const util::URL& aURL)
    throw (::com::sun::star::uno::RuntimeException)
{
    // search listener array for given listener
    // for checking equality always "cast" to XInterface
    if ( !bDisposing )
    {
        sal_uInt16 nCount = aStatusListeners.size();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            BibStatusDispatch *pObj = &aStatusListeners[n];
            sal_Bool bFlag=pObj->xListener.is();
            if (!bFlag || (pObj->xListener == aObject &&
                ( aURL.Complete.isEmpty() || pObj->aURL.Path == aURL.Path  )))
            {
                aStatusListeners.erase( aStatusListeners.begin() + n );
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
void BibFrameController_Impl::RemoveFilter()
{
    rtl::OUString aQuery;
    pDatMan->startQueryWith(aQuery);

    sal_uInt16 nCount = aStatusListeners.size();

    sal_Bool bRemoveFilter=sal_False;
    sal_Bool bQueryText=sal_False;

    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = &aStatusListeners[n];
        if ( pObj->aURL.Path == C2U("Bib/removeFilter") )
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_False;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;
            pObj->xListener->statusChanged( aEvent );
            bRemoveFilter=sal_True;
        }
        else if(pObj->aURL.Path == C2U("Bib/query"))
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_True;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;
            aEvent.State <<= aQuery;
            pObj->xListener->statusChanged( aEvent );
            bQueryText=sal_True;
        }

        if(bRemoveFilter && bQueryText)
            break;

    }
}
//-----------------------------------------------------------------------------
void BibFrameController_Impl::ChangeDataSource(const uno::Sequence< beans::PropertyValue >& aArgs)
{
    const beans::PropertyValue* pPropertyValue = aArgs.getConstArray();
    uno::Any aValue=pPropertyValue[0].Value;
    rtl::OUString aDBTableName;
    aValue >>= aDBTableName;


    if(aArgs.getLength() > 1)
    {
        uno::Any aDB = pPropertyValue[1].Value;
        rtl::OUString aURL;
        aDB >>= aURL;
        pDatMan->setActiveDataSource(aURL);
        aDBTableName = pDatMan->getActiveDataTable();
    }
    else
    {
        m_xDatMan->unload();
        pDatMan->setActiveDataTable(aDBTableName);
        pDatMan->updateGridModel();
        m_xDatMan->load();
    }


    sal_uInt16 nCount = aStatusListeners.size();

    sal_Bool bMenuFilter=sal_False;
    sal_Bool bQueryText=sal_False;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = &aStatusListeners[n];
        if(COMPARE_EQUAL == pObj->aURL.Path.compareToAscii("Bib/MenuFilter"))
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_True;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;
            aEvent.FeatureDescriptor=pDatMan->getQueryField();

            uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getQueryFields();
            aEvent.State  = makeAny( aStringSeq );

            pObj->xListener->statusChanged( aEvent );
            bMenuFilter=sal_True;
        }
        else if(COMPARE_EQUAL == pObj->aURL.Path.compareToAscii("Bib/query"))
        {
            FeatureStateEvent  aEvent;
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_True;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;
            BibConfig* pConfig = BibModul::GetConfig();
            aEvent.State <<= pConfig->getQueryText();
            pObj->xListener->statusChanged( aEvent );
            bQueryText=sal_True;
        }

        if (bMenuFilter && bQueryText)
            break;

    }
}

void BibFrameController_Impl::activate()
{
}
void BibFrameController_Impl::deactivate()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
