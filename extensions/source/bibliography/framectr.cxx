/*************************************************************************
 *
 *  $RCSfile: framectr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-04-05 15:49:29 $
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
//#ifndef _USR_ITERHLP_HXX
//#include <usr/iterhlp.hxx>
//#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
//#ifndef _TOOLKIT_UNOIFACE_HXX
//#include <toolkit/unoiface.hxx>
//#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BIB_FRAMECTR_HXX
#include "framectr.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef _BIB_FIELDWIN_HXX
#include "fieldwin.hxx"
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

using namespace osl;
using namespace cppu;
using namespace rtl;
using namespace com::sun::star::frame;
using namespace com::sun::star::uno;
using namespace com::sun::star;

#define C2U(cChar) OUString::createFromAscii(cChar)
extern rtl::OUString gGridModelCommand;

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
    virtual void                        SAL_CALL disposing( const lang::EventObject& Source );
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
{
    if ( pController )
        pController->getFrame()->removeFrameActionListener( this );
}

BibFrameController_Impl::BibFrameController_Impl( const uno::Reference< awt::XWindow > & xComponent):
    xWindow( xComponent ),
    pDatMan(NULL),
    pFieldWin(NULL)

{
    pBibMod = OpenBibModul();
    pDatMan = (*pBibMod)->createDataManager();
    xDatman = pDatMan;
    bDisposing=sal_False;
    bHierarchical=sal_True;
    pImp = new BibFrameCtrl_Impl;
    pImp->pController = this;
    pImp->acquire();
}

BibFrameController_Impl::BibFrameController_Impl( const uno::Reference< awt::XWindow > & xComponent,
                                                BibDataManager* pDataManager):
    xWindow( xComponent ),
    pDatMan(pDataManager),
    pFieldWin(NULL),
    pBibMod(NULL)
{
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

void BibFrameController_Impl::attachFrame( const uno::Reference< XFrame > & xArg )
{
    xFrame = xArg;
    xFrame->addFrameActionListener( pImp );
}

sal_Bool BibFrameController_Impl::attachModel( const uno::Reference< XModel > & xModel )
{
    return sal_False;
}

sal_Bool BibFrameController_Impl::suspend( sal_Bool bSuspend )
{
    if ( bSuspend )
        getFrame()->removeFrameActionListener( pImp );
    else
        getFrame()->addFrameActionListener( pImp );
    return sal_True;
}

uno::Any BibFrameController_Impl::getViewData()
{
    return uno::Any();
}

void BibFrameController_Impl::restoreViewData( const uno::Any& Value )
{
}

uno::Reference< XFrame >  BibFrameController_Impl::getFrame()
{
    return xFrame;
}

uno::Reference< XModel >  BibFrameController_Impl::getModel()
{
    return uno::Reference< XModel > ();
}

void BibFrameController_Impl::dispose()
{
    util::URL aURL;
    aURL.Complete = C2U("0");
    uno::Reference< XDispatchProvider >  xProv( xFrame, UNO_QUERY );

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


    delete pFieldWin;
    bDisposing = sal_True;
    lang::EventObject aObject;
    aObject.Source = (XController*)this;
    pImp->aLC.disposeAndClear(aObject);
    xDatman = 0;
    pDatMan = 0;
     aStatusListeners.DeleteAndDestroy( 0, aStatusListeners.Count() );
 }

void BibFrameController_Impl::addEventListener( const uno::Reference< lang::XEventListener > & aListener )
{
    pImp->aLC.addInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

void BibFrameController_Impl::removeEventListener( const uno::Reference< lang::XEventListener > & aListener )
{
    pImp->aLC.removeInterface( ::getCppuType((const Reference< lang::XEventListener >*)0), aListener );
}

uno::Reference< frame::XDispatch >  BibFrameController_Impl::queryDispatch( const util::URL& aURL, const rtl::OUString& aTarget, sal_Int32 nSearchFlags )
{
    if ( !bDisposing )
    {
        String aCommand( aURL.Path );
        if (    aCommand.EqualsAscii("Undo") || aCommand.EqualsAscii("Cut") ||
                aCommand.EqualsAscii("Copy") || aCommand.EqualsAscii("Paste") ||
                aCommand.EqualsAscii("SelectAll") || aCommand.Copy(0,4).EqualsAscii("Bib/")||
                aCommand.EqualsAscii("Title"))

            return (frame::XDispatch*) this;
    }

    return uno::Reference< frame::XDispatch > ();
}

uno::Sequence<uno::Reference< XDispatch > > BibFrameController_Impl::queryDispatches( const uno::Sequence<DispatchDescriptor>& aDescripts )
{
    return uno::Sequence<uno::Reference< XDispatch > >();
}

//class XDispatch
void BibFrameController_Impl::dispatch(const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs)
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
                uno::Sequence< beans::PropertyValue >   aArgs(2);
                beans::PropertyValue* pProps = aArgs.getArray();
                pProps[0].Value <<= rtl::OUString();
                pProps[1].Value <<= aURL;
                ChangeDataSource(aArgs);
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

            uno::Reference< sdb::XSQLQueryComposer >  xParser=pDatMan->getParser();
//          uno::Reference< data::XDatabaseDialogs >  xDlgs(xParser, UNO_QUERY);
//          if (xDlgs.is())
//          {
//              uno::Reference< container::XNamed >  xField;
//              xDlgs->executeFilter(xField);
//              pDatMan->setFilter(xParser->getFilter());
//          }
        }
        else if(aCommand.EqualsAscii("Bib/removeFilter"))
        {
            RemoveFilter();
        }
    }
}
//-----------------------------------------------------------------------------
void BibFrameController_Impl::addStatusListener(
    const uno::Reference< frame::XStatusListener > & aListener,
    const util::URL& aURL)
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
        aEvent.IsEnabled  = (aFilterStr.len() > 0);
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aListener->statusChanged( aEvent );
        //break; because there are more than one
    }
    else
    {
        FeatureStateEvent aEvent;
        aEvent.FeatureURL = aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.Source     = (XDispatch *) this;
        aEvent.State <<= OUString(String(BibResId(RID_BIB_STR_FRAME_TITLE)));
        aListener->statusChanged( aEvent );
    }
}
//-----------------------------------------------------------------------------
void BibFrameController_Impl::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & aObject, const util::URL& aURL)
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
                ( !aURL.Complete.len() || pObj->aURL.Path == aURL.Path  )))
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

    pDatMan->unloadDatabase();

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
    uno::Reference< awt::XControlModel >  xNewModel=pDatMan->createGridModel();

    sal_uInt16 nCount = aStatusListeners.Count();
    FeatureStateEvent  aEvent;

    sal_Bool bGridMod=sal_False;
    sal_Bool bMenuFilter=sal_False;
    sal_Bool bQueryText=sal_False;
    for ( sal_uInt16 n=0; n<nCount; n++ )
    {
        BibStatusDispatch *pObj = aStatusListeners[n];
        if ( COMPARE_EQUAL == pObj->aURL.Path.compareToAscii("Bib/newGridModel"))
        {
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_True;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;

            aEvent.State.setValue(&xNewModel, ::getCppuType((uno::Reference<awt::XControlModel>*)0));
            pObj->xListener->statusChanged( aEvent );
            bGridMod=sal_True;
        }
        else if(COMPARE_EQUAL == pObj->aURL.Path.compareToAscii("Bib/MenuFilter"))
        {
            aEvent.FeatureURL = pObj->aURL;
            aEvent.IsEnabled  = sal_True;
            aEvent.Requery    = sal_False;
            aEvent.Source     = (XDispatch *) this;
            aEvent.FeatureDescriptor=pDatMan->getQueryField();

            uno::Sequence<rtl::OUString> aStringSeq=pDatMan->getQueryFields();
            aEvent.State.setValue(&aStringSeq,::getCppuType((uno::Sequence<rtl::OUString>*)0));

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

        if(bGridMod && bMenuFilter && bQueryText) break;

    }
    pDatMan->loadDatabase();
    if(pFieldWin)
        pFieldWin->Update(pDatMan->getDatabaseForm());
}

void BibFrameController_Impl::activate()
{
    if(pFieldWin) pFieldWin->Show();
}
void BibFrameController_Impl::deactivate()
{
    if(pFieldWin) pFieldWin->Hide();
}


