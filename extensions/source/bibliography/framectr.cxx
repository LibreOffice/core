/*************************************************************************
 *
 *  $RCSfile: framectr.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:01 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_WAITOBJ_HXX //autogen wg. WaitObject
#include <vcl/waitobj.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _BIB_FRAMECTR_HXX
#include "framectr.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef ADRRESID_HXX
#include "bibresid.hxx"
#endif
#ifndef BIB_HRC
#include "bib.hrc"
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _BIBCONFIG_HXX
#include "bibconfig.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include <extensio.hrc>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_XCONFIRMDELETELISTENER_HPP_
#include <com/sun/star/form/XConfirmDeleteListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif

using namespace osl;
using namespace cppu;
using namespace rtl;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star;

#define C2U(cChar) OUString::createFromAscii(cChar)

SV_IMPL_PTRARR( BibStatusDispatchArr, BibStatusDispatchPtr );

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

void BibFrameCtrl_Impl::disposing( const lang::EventObject& Source )
    throw (::com::sun::star::uno::RuntimeException)
{
    util::URL aURL;
    aURL.Complete = C2U("0");
    uno::Reference< XDispatchProvider >  xProv( pController->xFrame, UNO_QUERY );

    if ( xProv.is() )
    {
        uno::Sequence<beans::PropertyValue> aArgs( 1 );
        Any aValue;
        aValue <<= C2U( "remove" );
        aArgs[0].Value  = aValue;
        aArgs[0].Name   = C2U( "command" );

        uno::Reference< XDispatch >  aDisp = xProv->queryDispatch( aURL,  C2U("_menubar"), 0 );
        if ( aDisp.is() )
            aDisp->dispatch( aURL, aArgs );
    }

    if ( pController )
        pController->getFrame()->removeFrameActionListener( this );
}

BibFrameController_Impl::BibFrameController_Impl( const uno::Reference< awt::XWindow > & xComponent):
    xWindow( xComponent ),
    pDatMan(NULL)

{
    Window* pParent = VCLUnoHelper::GetWindow( xWindow );
    pParent->SetUniqueId(UID_BIB_FRAME_WINDOW);
    pBibMod = OpenBibModul();
    pDatMan = (*pBibMod)->createDataManager();
    m_xDatMan = pDatMan;
    bDisposing=sal_False;
    bHierarchical=sal_True;
    pImp = new BibFrameCtrl_Impl;
    pImp->pController = this;
    pImp->acquire();
}

BibFrameController_Impl::BibFrameController_Impl( const uno::Reference< awt::XWindow > & xComponent,
                                                BibDataManager* pDataManager)
    :xWindow( xComponent )
    ,pDatMan( pDataManager )
    ,m_xDatMan( pDataManager )
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

BibDataManager* BibFrameController_Impl::GetDataManager()
{
    return pDatMan;
}

BibFrameController_Impl::~BibFrameController_Impl()
{
    pImp->pController = NULL;
    pImp->release();
    delete pDatMan;
    if(pBibMod)
        CloseBibModul(pBibMod);
}

void BibFrameController_Impl::attachFrame( const uno::Reference< XFrame > & xArg ) throw (::com::sun::star::uno::RuntimeException)
{
    xFrame = xArg;
    xFrame->addFrameActionListener( pImp );
}

sal_Bool BibFrameController_Impl::attachModel( const uno::Reference< XModel > & xModel ) throw (::com::sun::star::uno::RuntimeException)
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

void BibFrameController_Impl::restoreViewData( const uno::Any& Value ) throw (::com::sun::star::uno::RuntimeException)
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
    aStatusListeners.DeleteAndDestroy( 0, aStatusListeners.Count() );
 }

void BibFrameController_Impl::addEventListener( const uno::Reference< lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    pImp->aLC.addInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

void BibFrameController_Impl::removeEventListener( const uno::Reference< lang::XEventListener > & aListener ) throw (::com::sun::star::uno::RuntimeException)
{
    pImp->aLC.removeInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

uno::Reference< frame::XDispatch >  BibFrameController_Impl::queryDispatch( const util::URL& aURL, const rtl::OUString& aTarget, sal_Int32 nSearchFlags ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !bDisposing )
    {
        String aCommand( aURL.Path );
        if (    aCommand.EqualsAscii("Undo") || aCommand.EqualsAscii("Cut") ||
                aCommand.EqualsAscii("Copy") || aCommand.EqualsAscii("Paste") ||
                aCommand.EqualsAscii("SelectAll") || aCommand.Copy(0,4).EqualsAscii("Bib/")||
                aURL.Complete.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("slot:5503")) ||
                aCommand.EqualsAscii("CloseDoc"))

            return (frame::XDispatch*) this;
    }

    return uno::Reference< frame::XDispatch > ();
}

uno::Sequence<uno::Reference< XDispatch > > BibFrameController_Impl::queryDispatches( const uno::Sequence<DispatchDescriptor>& aDescripts ) throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Sequence<uno::Reference< XDispatch > >();
}
sal_Bool canInsertRecords(const Reference< beans::XPropertySet>& _rxCursorSet)
{
    sal_Int32 nPriv = 0;
    _rxCursorSet->getPropertyValue(C2U("Privileges")) >>= nPriv;
    return ((_rxCursorSet.is() && (nPriv & sdbcx::Privilege::INSERT) != 0));
}
/* -----------------------------08.05.2002 08:58------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool BibFrameController_Impl::SaveModified(const Reference< form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;
    sal_Bool bInserted = sal_False;

    Reference< XResultSetUpdate> _xCursor = Reference< XResultSetUpdate>(xController->getModel(), UNO_QUERY);

    if (!_xCursor.is())
        return sal_False;

    Reference< beans::XPropertySet> _xSet = Reference< beans::XPropertySet>(_xCursor, UNO_QUERY);
    if (!_xSet.is())
        return sal_False;

    // muß gespeichert werden ?
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
        catch(Exception&)
        {
            DBG_ERROR("SaveModified: Exception occured!");
        }

        bInserted = bIsNew && bResult;
    }
    return bResult;
}

//class XDispatch
void BibFrameController_Impl::dispatch(const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs) throw (::com::sun::star::uno::RuntimeException)
{
    if ( !bDisposing )
    {
        WaitObject aWaitObject;

        String aCommand( aURL.Path);
        if(aCommand.EqualsAscii("Bib/Mapping"))
        {
            Window* pParent = VCLUnoHelper::GetWindow( xWindow );
            pDatMan->CreateMappingDialog(pParent);
        }
        else if(aCommand.EqualsAscii("Bib/source"))
        {
            ChangeDataSource(aArgs);
        }
        else if(aCommand.EqualsAscii("Bib/sdbsource"))
        {
            Window* pParent = VCLUnoHelper::GetWindow( xWindow );
            rtl::OUString aURL = pDatMan->CreateDBChangeDialog(pParent);
            if(aURL.getLength())
            {
                try
                {
                    uno::Sequence< beans::PropertyValue >   aArgs(2);
                    beans::PropertyValue* pProps = aArgs.getArray();
                    pProps[0].Value <<= rtl::OUString();
                    pProps[1].Value <<= aURL;
                    ChangeDataSource(aArgs);
                }
                catch(const Exception&)
                {
                    DBG_ERROR("Exception catched while changing the data source");
                }
            }
        }
        else if(aCommand.EqualsAscii("Bib/autoFilter"))
        {
            sal_uInt16 nCount = aStatusListeners.Count();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                BibStatusDispatch *pObj = aStatusListeners[n];
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
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString::createFromAscii( "QueryComposer" ),
                                                        -1,
                                                        makeAny( pDatMan->getParser() ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );

                // the rowset
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString::createFromAscii( "RowSet" ),
                                                        -1,
                                                        makeAny( pDatMan->getForm() ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );
                // the parent window for the dialog
                *pDialogCreationArgs++ <<= beans::PropertyValue( ::rtl::OUString::createFromAscii( "ParentWindow" ),
                                                        -1,
                                                        makeAny( xWindow ),
                                                        beans::PropertyState_DIRECT_VALUE
                                                      );

                // create the dialog object
                const ::rtl::OUString sDialogServiceName = ::rtl::OUString::createFromAscii( "com.sun.star.uno.sdb.FilterDialog" );
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
                DBG_ERROR( "BibFrameController_Impl::dispatch: caught an exception!" );
            }

            sal_uInt16 nCount = aStatusListeners.Count();
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                BibStatusDispatch *pObj = aStatusListeners[n];
                if ( pObj->aURL.Path == C2U("Bib/removeFilter") )
                {
                    FeatureStateEvent  aEvent;
                    aEvent.FeatureURL = pObj->aURL;
                    aEvent.IsEnabled  = 0 != pDatMan->getParser()->getFilter().getLength();
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
        else if(aURL.Complete.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("slot:5503")) ||
                aCommand.EqualsAscii("CloseDoc"))
        {
            Application::PostUserEvent( STATIC_LINK( this, BibFrameController_Impl,
                                        DisposeHdl ), 0 );

        }
        else if(aCommand.EqualsAscii("Bib/InsertRecord"))
        {
            Reference<form::XFormController > xFormCtrl = pDatMan->GetFormController();
            if(SaveModified(xFormCtrl))
            {
                try
                {
                    Reference< sdbc::XResultSet >  xCursor( pDatMan->getForm(), UNO_QUERY );
                    xCursor->last();

                    Reference< XResultSetUpdate >  xUpdateCursor( pDatMan->getForm(), UNO_QUERY );
                    xUpdateCursor->moveToInsertRow();
                }
                catch(Exception&)
                {
                    DBG_ERROR("Exception in last() or moveToInsertRow()")
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
                sal_Bool bLeft = xCursor->isLast() && nCount > 1;
                sal_Bool bRight= !xCursor->isLast();
                sal_Bool bSuccess = sal_False;
                try
                {
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
                catch(Exception&)
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
                        catch(Exception&)
                        {
                            DBG_ERROR("DeleteRecord : exception caught !");
                        }
                    }
                }
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
    aStatusListeners.Insert( new BibStatusDispatch( aURL, aListener ), aStatusListeners.Count() );

    // den ersten Status synchron zusenden
    if ( aURL.Path == C2U("Bib/hierarchical") )
    {
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        const char*  pHier = bHierarchical? "" : "*" ;
        aEvent.State <<= rtl::OUString::createFromAscii(pHier);
        aListener->statusChanged( aEvent );
    }
    else if(aURL.Path == C2U("Bib/MenuFilter"))
    {
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aEvent.FeatureDescriptor=pDatMan->getQueryField();

        uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getQueryFields();
        aEvent.State.setValue(&aStringSeq,::getCppuType((uno::Sequence<rtl::OUString>*)0));

        aListener->statusChanged( aEvent );
    }
    else if ( aURL.Path == C2U("Bib/source"))
    {
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aEvent.FeatureDescriptor=pDatMan->getActiveDataTable();

        uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getDataSources();
        aEvent.State.setValue(&aStringSeq,::getCppuType((uno::Sequence<rtl::OUString>*)0));
        aListener->statusChanged( aEvent );
    }
    else if(aURL.Path == C2U("Bib/query"))
    {
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aEvent.State <<= pConfig->getQueryText();
        aListener->statusChanged( aEvent );
    }
    else if (aURL.Path == C2U("Bib/removeFilter") )
    {
        rtl::OUString aFilterStr=pDatMan->getFilter();
        FeatureStateEvent  aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = (aFilterStr.getLength() > 0);
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aListener->statusChanged( aEvent );
        //break; because there are more than one
    }
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
        sal_uInt16 nCount = aStatusListeners.Count();
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            BibStatusDispatch *pObj = aStatusListeners[n];
            sal_Bool bFlag=pObj->xListener.is();
            if (!bFlag || (pObj->xListener == aObject &&
                ( !aURL.Complete.getLength() || pObj->aURL.Path == aURL.Path  )))
            {
                aStatusListeners.DeleteAndDestroy( n );
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

    sal_uInt16 nCount = aStatusListeners.Count();
    FeatureStateEvent  aEvent;

    sal_Bool bRemoveFilter=sal_False;
    sal_Bool bQueryText=sal_False;

    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = aStatusListeners[n];
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
            FeatureStateEvent aEvent;
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

    m_xDatMan->unload();

    if(aArgs.getLength() > 1)
    {
        uno::Any aDB = pPropertyValue[1].Value;
        rtl::OUString aURL;
        aDB >>= aURL;
        pDatMan->setActiveDataSource(aURL);
        aDBTableName = pDatMan->getActiveDataTable();
    }
    else
        pDatMan->setActiveDataTable(aDBTableName);

    pDatMan->updateGridModel();

    sal_uInt16 nCount = aStatusListeners.Count();
    FeatureStateEvent  aEvent;

    sal_Bool bMenuFilter=sal_False;
    sal_Bool bQueryText=sal_False;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = aStatusListeners[n];
        if(COMPARE_EQUAL == pObj->aURL.Path.compareToAscii("Bib/MenuFilter"))
        {
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
            FeatureStateEvent aEvent;
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
    m_xDatMan->load();
}

void BibFrameController_Impl::activate()
{
}
void BibFrameController_Impl::deactivate()
{
}


