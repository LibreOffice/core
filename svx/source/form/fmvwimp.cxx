/*************************************************************************
 *
 *  $RCSfile: fmvwimp.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:45:14 $
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
#pragma hdrstop

#ifndef _SVX_FMCTRLER_HXX
#include <fmctrler.hxx>
#endif

#ifndef _SVX_FMVWIMP_HXX
#include "fmvwimp.hxx"
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SCROLLBARORIENTATION_HPP_
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLER_HPP_
#include <com/sun/star/awt/XTabController.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _SVX_FMMODEL_HXX
#include <fmmodel.hxx>
#endif

#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _SVX_FMVIEW_HXX
#include "fmview.hxx"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include "dataaccessdescriptor.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::style;
using namespace ::comphelper;
using namespace ::svxform;
using namespace ::svx;

namespace svxform
{
    //========================================================================
    class OAutoDispose
    {
    protected:
        Reference< XComponent > m_xComp;

    public:
        OAutoDispose( const Reference< XInterface > _rxObject );
        ~OAutoDispose();
    };

    //------------------------------------------------------------------------
    OAutoDispose::OAutoDispose( const Reference< XInterface > _rxObject )
        :m_xComp(_rxObject, UNO_QUERY)
    {
    }

    //------------------------------------------------------------------------
    OAutoDispose::~OAutoDispose()
    {
        if (m_xComp.is())
            m_xComp->dispose();
    }
}

//------------------------------------------------------------------------------
class FmXFormView::ObjectRemoveListener : public SfxListener
{
    FmXFormView* m_pParent;
public:
    ObjectRemoveListener( FmXFormView* pParent );
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

//========================================================================
DBG_NAME(FmXPageViewWinRec);
//------------------------------------------------------------------------
FmXPageViewWinRec::FmXPageViewWinRec(const Reference< XMultiServiceFactory >&   _xORB,
    //const SdrPageViewWinRec* pWinRec,
    const SdrPageViewWindow& rWindow,
    FmXFormView* _pViewImpl)
:   m_pViewImpl(_pViewImpl)
    ,m_pWindow( (Window*)(&rWindow.GetOutputDevice()) )
    ,m_xORB(_xORB)
{
    DBG_CTOR(FmXPageViewWinRec,NULL);

    // legt fuer jede ::com::sun::star::form ein FormController an
    FmFormPage* pP = NULL;
    if (m_pViewImpl)
        pP = PTR_CAST(FmFormPage,m_pViewImpl->getView()->GetPageViewPvNum(0)->GetPage());

    DBG_ASSERT(pP,"kein Page gefunden");

    if (pP)
    {
        Reference< XIndexAccess >  xForms(pP->GetForms(), UNO_QUERY);
        sal_uInt32 nLength = xForms->getCount();
        Any aElement;
        Reference< XForm >  xForm;
        for (sal_uInt32 i = 0; i < nLength; i++)
        {
            xForms->getByIndex(i) >>= xForm;
            setController(xForm, rWindow.GetControlContainerRef() );
        }
    }
}
// -----------------------------------------------------------------------------
FmXPageViewWinRec::~FmXPageViewWinRec()
{
    DBG_DTOR(FmXPageViewWinRec,NULL);
}

//------------------------------------------------------------------
void FmXPageViewWinRec::dispose()
{
    for (vector< Reference< XFormController > >::const_iterator i = m_aControllerList.begin();
            i != m_aControllerList.end(); i++)
    {
        // detaching the events
        Reference< XChild >  xChild((*i)->getModel(), UNO_QUERY);
        if (xChild.is())
        {
            Reference< XEventAttacherManager >  xEventManager(xChild->getParent(), UNO_QUERY);
            Reference< XInterface >  xIfc(*i, UNO_QUERY);
            xEventManager->detach( i - m_aControllerList.begin(), xIfc );
        }

        // dispose the formcontroller
        Reference< XComponent >  xComp(*i, UNO_QUERY);
        xComp->dispose();
    }
    m_aControllerList.clear(); // this call deletes the formcontrollers
    m_xORB = NULL;
}


//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXPageViewWinRec::hasElements(void) throw( RuntimeException )
{
    return getCount() != 0;
}

//------------------------------------------------------------------------------
Type SAL_CALL  FmXPageViewWinRec::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((const Reference< XFormController>*)0);
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference< XEnumeration >  SAL_CALL FmXPageViewWinRec::createEnumeration(void) throw( RuntimeException )
{
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FmXPageViewWinRec::getCount(void) throw( RuntimeException )
{
    return m_aControllerList.size();
}

//------------------------------------------------------------------------------
Any SAL_CALL FmXPageViewWinRec::getByIndex(sal_Int32 nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if (nIndex < 0 ||
        nIndex >= getCount())
        throw IndexOutOfBoundsException();

    Any aElement;
    aElement <<= m_aControllerList[nIndex];
    return aElement;
}

//------------------------------------------------------------------------
Reference< XFormController >  getControllerSearchChilds( const Reference< XIndexAccess > & xIndex, const Reference< XTabControllerModel > & xModel)
{
    if (xIndex.is() && xIndex->getCount())
    {
        Reference< XFormController >  xController;

        for (sal_Int32 n = xIndex->getCount(); n-- && !xController.is(); )
        {
            xIndex->getByIndex(n) >>= xController;
            if ((XTabControllerModel*)xModel.get() == (XTabControllerModel*)xController->getModel().get())
                return xController;
            else
            {
                xController = getControllerSearchChilds(Reference< XIndexAccess > (xController, UNO_QUERY), xModel);
                if ( xController.is() )
                    return xController;
            }
        }
    }
    return Reference< XFormController > ();
}

// Search the according controller
//------------------------------------------------------------------------
Reference< XFormController >  FmXPageViewWinRec::getController( const Reference< XForm > & xForm )
{
    Reference< XTabControllerModel >  xModel(xForm, UNO_QUERY);
    for (::std::vector< Reference< XFormController > >::const_iterator i = m_aControllerList.begin();
         i != m_aControllerList.end(); i++)
    {
        if ((XTabControllerModel*)(*i)->getModel().get() == (XTabControllerModel*)xModel.get())
            return *i;

        // the current-round controller isn't the right one. perhaps one of it's children ?
        Reference< XFormController >  xChildSearch = getControllerSearchChilds(Reference< XIndexAccess > (*i, UNO_QUERY), xModel);
        if (xChildSearch.is())
            return xChildSearch;
    }
    return Reference< XFormController > ();
}

//------------------------------------------------------------------------
void FmXPageViewWinRec::setController(const Reference< XForm > & xForm,
                                     const Reference< XControlContainer > & xCC,
                                     FmXFormController* pParent)
{
    DBG_ASSERT(xForm.is(), "Kein Formular angegeben");
    Reference< XIndexAccess >  xFormCps(xForm, UNO_QUERY);
    if (!xFormCps.is())
        return;

    Reference< XTabControllerModel >  xTabOrder(xForm, UNO_QUERY);

    // Anlegen des Tabcontrollers
    FmXFormController* pController = new FmXFormController( m_xORB,m_pViewImpl->getView(), m_pWindow );
    Reference< XFormController > xController( pController );

    pController->setModel(xTabOrder);
    pController->setContainer(xCC);
    pController->activateTabOrder();
    pController->addActivateListener(m_pViewImpl);

    if (pParent)
        pParent->addChild(pController);
    else
    {
        //  Reference< XFormController >  xController(pController);
        m_aControllerList.push_back(xController);

        pController->setParent(*this);

        // attaching the events
        Reference< XEventAttacherManager >  xEventManager(xForm->getParent(), UNO_QUERY);
        Reference< XInterface >  xIfc(xController, UNO_QUERY);
        xEventManager->attach(m_aControllerList.size() - 1, xIfc, makeAny(xController) );
    }



    // jetzt die Subforms durchgehen
    sal_uInt32 nLength = xFormCps->getCount();
    Reference< XForm >  xSubForm;
    for (sal_uInt32 i = 0; i < nLength; i++)
    {
        xFormCps->getByIndex(i) >>= xSubForm;
        if (xSubForm.is())
            setController(xSubForm, xCC, pController);
    }
}

//------------------------------------------------------------------------
void FmXPageViewWinRec::updateTabOrder( const Reference< XControl > & xControl,
                                       const Reference< XControlContainer > & xCC )
{
    // Das TabControllerModel der ::com::sun::star::form ermitteln, in der das Control
    // enthalten ist ...
    Reference< XFormComponent >  xFormComp(xControl->getModel(), UNO_QUERY);
    if (!xFormComp.is())
        return;

    Reference< XForm >  xForm(xFormComp->getParent(), UNO_QUERY);
    if (!xForm.is())
        return;

    Reference< XTabController >  xTabCtrl(getController( xForm ), UNO_QUERY);
    // Wenn es fuer dieses Formular noch keinen Tabcontroller gibt,
    // dann einen neuen anlegen
    if (!xTabCtrl.is())
    {
        // ist es ein Unterformular?
        // dann muss ein Tabcontroller fuer den Parent existieren
        // wichtig da ein hierarchischer Aufbau vorliegt
        Reference< XForm >  xParentForm(Reference< XChild > (xForm, UNO_QUERY)->getParent(), UNO_QUERY);
        FmXFormController* pFormController = NULL;
        // zugehoerigen controller suchen
        if (xParentForm.is())
            xTabCtrl = Reference< XTabController >(getController(xParentForm), UNO_QUERY);

        if (xTabCtrl.is())
        {
            Reference< XUnoTunnel > xTunnel(xTabCtrl,UNO_QUERY);
            DBG_ASSERT(xTunnel.is(), "FmPropController::ChangeFormatProperty : xTunnel is invalid!");
            if(xTunnel.is())
            {
                pFormController = reinterpret_cast<FmXFormController*>(xTunnel->getSomething(FmXFormController::getUnoTunnelImplementationId()));
            }
            //  ::comphelper::getImplementation(pFormController, xTunnel);
        }

        // Es gibt noch keinen TabController fuer das Formular, also muss
        // ein neuer angelegt werden.
        setController(xForm, xCC, pFormController);
    }
}

//------------------------------------------------------------------------
Reference< XControlContainer >  FmXPageViewWinRec::getControlContainer() const
{
    Reference< XControlContainer >  xCC;
    if (m_aControllerList.size())
        xCC = m_aControllerList[0]->getContainer();
    return xCC;
}

//------------------------------------------------------------------------
FmXFormView::FmXFormView(const Reference< XMultiServiceFactory >&   _xORB,
            FmFormView* _pView)
    :m_pView(_pView)
    ,m_xORB( _xORB )
    ,m_nActivationEvent(0)
    ,m_nErrorMessageEvent( 0 )
    ,m_nAutoFocusEvent( 0 )
    ,m_pWatchStoredList( NULL )
    ,m_bFirstActivation( sal_True )
    ,m_pMarkedGrid(NULL)
{
}

//------------------------------------------------------------------------
void FmXFormView::cancelEvents()
{
    if ( m_nActivationEvent )
    {
        Application::RemoveUserEvent( m_nActivationEvent );
        m_nActivationEvent = 0;
    }

    if ( m_nErrorMessageEvent )
    {
        Application::RemoveUserEvent( m_nErrorMessageEvent );
        m_nErrorMessageEvent = 0;
    }

    if ( m_nAutoFocusEvent )
    {
        Application::RemoveUserEvent( m_nAutoFocusEvent );
        m_nAutoFocusEvent = 0;
    }
}

//------------------------------------------------------------------------
void FmXFormView::notifyViewDying( )
{
    DBG_ASSERT( m_pView, "FmXFormView::notifyViewDying: my view already died!" );
    m_pView = NULL;
    cancelEvents();
}

//------------------------------------------------------------------------
FmXFormView::~FmXFormView()
{
    DBG_ASSERT(m_aWinList.size() == 0, "Liste nicht leer");

    cancelEvents();

    delete m_pWatchStoredList;
    m_pWatchStoredList = NULL;
}

//      EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::disposing(const EventObject& Source) throw( RuntimeException )
{
    if ( m_xWindow.is() && Source.Source == m_xWindow )
        removeGridWindowListening();
}

// XFormControllerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formActivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( m_pView && m_pView->GetFormShell() && m_pView->GetFormShell()->GetImpl() )
        m_pView->GetFormShell()->GetImpl()->formActivated( rEvent );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formDeactivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( m_pView && m_pView->GetFormShell() && m_pView->GetFormShell()->GetImpl() )
        m_pView->GetFormShell()->GetImpl()->formDeactivated( rEvent );
}

// XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementInserted(const ContainerEvent& evt) throw( RuntimeException )
{
    Reference< XControlContainer >  xCC(evt.Source, UNO_QUERY);
    if( xCC.is() )
    {
        FmWinRecList::iterator i = findWindow( xCC );

        if ( i != m_aWinList.end() )
        {
            Reference< XControl >  xControl;
            evt.Element >>= xControl;
            if( xControl.is() )
                (*i)->updateTabOrder( xControl, xCC );
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementReplaced(const ContainerEvent& evt) throw( RuntimeException )
{
    elementInserted(evt);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementRemoved(const ContainerEvent& evt) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
FmWinRecList::const_iterator FmXFormView::findWindow( const Reference< XControlContainer > & rCC )  const
{
    for (FmWinRecList::const_iterator i = m_aWinList.begin();
            i != m_aWinList.end(); i++)
    {
        if (rCC == (*i)->getControlContainer())
            return i;
    }
    return m_aWinList.end();
}

//------------------------------------------------------------------------------
FmWinRecList::iterator FmXFormView::findWindow( const Reference< XControlContainer > & rCC )
{
    for (FmWinRecList::iterator i = m_aWinList.begin();
            i != m_aWinList.end(); i++)
    {
        if (rCC == (*i)->getControlContainer())
            return i;
    }
    return m_aWinList.end();
}

//------------------------------------------------------------------------------
//void FmXFormView::addWindow(const SdrPageViewWinRec* pRec)
void FmXFormView::addWindow(const SdrPageViewWindow& rWindow)
{
    // Wird gerufen, wenn
    // - in den aktiven Modus geschaltet wird
    // - ein Window hinzugefuegt wird, waehrend man im Design-Modus ist
    // - der Control-Container fuer ein Window angelegt wird, waehrend
    //   der aktive Modus eingeschaltet ist.

    // Wenn es noch keinen Control-Container gibt oder am Control-Container
    // noch keine Peer Erzeugt wurde, dann ist es noch zu frueh, um die
    // Tab-Order einzustellen ...
    if( rWindow.GetOutputDevice().GetOutDevType() == OUTDEV_WINDOW  )
    {
        const Reference< XControlContainer > & rCC = rWindow.GetControlContainerRef();
        if ( rCC.is() && findWindow( rCC ) == m_aWinList.end())
        {
            FmXPageViewWinRec *pFmRec = new FmXPageViewWinRec(m_xORB, rWindow, this);
            pFmRec->acquire();

            m_aWinList.push_back(pFmRec);

            // Am ControlContainer horchen um Aenderungen mitzbekommen
            Reference< XContainer >  xContainer(rCC, UNO_QUERY);
            if (xContainer.is())
                xContainer->addContainerListener(this);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormView::removeWindow( const Reference< XControlContainer > & rCC )
{
    // Wird gerufen, wenn
    // - in den Design-Modus geschaltet wird
    // - ein Window geloescht wird, waehrend man im Design-Modus ist
    // - der Control-Container fuer ein Window entfernt wird, waehrend
    //   der aktive Modus eingeschaltet ist.
    FmWinRecList::iterator i = findWindow( rCC );
    if (i != m_aWinList.end())
    {
        // Am ControlContainer horchen um Aenderungen mitzbekommen
        Reference< XContainer >  xContainer(rCC, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener(this);

        (*i)->dispose();
        (*i)->release();
        m_aWinList.erase(i);
    }
}

//------------------------------------------------------------------------------
void FmXFormView::displayAsyncErrorMessage( const SQLErrorEvent& _rEvent )
{
    DBG_ASSERT( 0 == m_nErrorMessageEvent, "FmXFormView::displayAsyncErrorMessage: not too fast, please!" );
        // This should not happen - usually, the PostUserEvent is faster than any possible user
        // interaction which could trigger a new error. If it happens, we need a queue for the events.
    m_aAsyncError = _rEvent;
    m_nErrorMessageEvent = Application::PostUserEvent( LINK( this, FmXFormView, OnDelayedErrorMessage ) );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnDelayedErrorMessage, void*, EMPTYTAG)
{
    m_nErrorMessageEvent = 0;
    displayException( m_aAsyncError );
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormView::onFirstViewActivation( const FmFormModel* _pDocModel )
{
    if ( _pDocModel && _pDocModel->GetAutoControlFocus() )
        Application::PostUserEvent( LINK( this, FmXFormView, OnAutoFocus ) );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnActivate, void*, EMPTYTAG)
{
    m_nActivationEvent = 0;

    if ( !m_pView )
    {
        DBG_ERROR( "FmXFormView::OnActivate: well .... seems we have a timing problem (the view already died)!" );
        return 0;
    }

    // setting the controller to activate
    if (m_pView->GetFormShell() && m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
    {
        Window* pWindow = const_cast<Window*>(static_cast<const Window*>(m_pView->GetActualOutDev()));
        FmXPageViewWinRec* pFmRec = m_aWinList.size() ? m_aWinList[0] : NULL;
        for (FmWinRecList::const_iterator i = m_aWinList.begin();
            i != m_aWinList.end(); i++)
        {
            if (pWindow == (*i)->getWindow())
                pFmRec =*i;
        }

        if (pFmRec)
        {
            for (vector< Reference< XFormController > >::const_iterator i = pFmRec->GetList().begin();
                i != pFmRec->GetList().end(); i++)
            {
                const Reference< XFormController > & xController = *i;
                if (xController.is())
                {
                    // Nur bei Datenbankformularen erfolgt eine aktivierung
                    Reference< XRowSet >  xForm(xController->getModel(), UNO_QUERY);
                    if (xForm.is() && OStaticDataAccessTools().getRowSetConnection(xForm).is())
                    {
                        Reference< XPropertySet >  xFormSet(xForm, UNO_QUERY);
                        if (xFormSet.is())
                        {
                            // wenn es eine Datenquelle gibt, dann als aktive ::com::sun::star::form setzen
                            ::rtl::OUString aSource = ::comphelper::getString(xFormSet->getPropertyValue(FM_PROP_COMMAND));
                            if (aSource.getLength())
                            {
                                // benachrichtigung der Shell
                                FmXFormShell* pShImpl =  m_pView->GetFormShell()->GetImpl();
                                if (pShImpl)
                                    pShImpl->setActiveController(xController);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
void FmXFormView::Activate(sal_Bool bSync)
{
    if (m_nActivationEvent)
    {
        Application::RemoveUserEvent(m_nActivationEvent);
        m_nActivationEvent = 0;
    }

    if (bSync)
    {
        LINK(this,FmXFormView,OnActivate).Call(NULL);
    }
    else
        m_nActivationEvent = Application::PostUserEvent(LINK(this,FmXFormView,OnActivate));
}

//------------------------------------------------------------------------------
void FmXFormView::Deactivate(BOOL bDeactivateController)
{
    if (m_nActivationEvent)
    {
        Application::RemoveUserEvent(m_nActivationEvent);
        m_nActivationEvent = 0;
    }

    FmXFormShell* pShImpl =  m_pView->GetFormShell() ? m_pView->GetFormShell()->GetImpl() : NULL;
    if (pShImpl && bDeactivateController)
        pShImpl->setActiveController( NULL );
}

//------------------------------------------------------------------------------
void FmXFormView::AttachControl( const Reference< XControl > & rControl, sal_Bool bDetach )
{
    /* wird im fmctrler gemacht */
}

//------------------------------------------------------------------------------
void FmXFormView::AttachControls( const Reference< XControlContainer > & rCtrlContainer,
                                  sal_Bool bDetach )
{
}

//------------------------------------------------------------------------------
FmFormShell* FmXFormView::GetFormShell() const
{
    return m_pView ? m_pView->GetFormShell() : NULL;
}
// -----------------------------------------------------------------------------
void FmXFormView::AutoFocus( sal_Bool _bSync )
{
    if (m_nAutoFocusEvent)
          Application::RemoveUserEvent(m_nAutoFocusEvent);

    if ( _bSync )
        OnAutoFocus( NULL );
    else
        m_nAutoFocusEvent = Application::PostUserEvent(LINK(this, FmXFormView, OnAutoFocus));
}
// -----------------------------------------------------------------------------
static Reference< XControl > lcl_firstFocussableControl( const Sequence< Reference< XControl > >& _rControls )
{
    Reference< XControl > xReturn;

    // loop through all the controls
    const Reference< XControl >* pControls = _rControls.getConstArray();
    const Reference< XControl >* pControlsEnd = _rControls.getConstArray() + _rControls.getLength();
    for ( ; pControls != pControlsEnd; ++pControls )
    {
        try
        {
            // check the class id of the control model
            sal_Int16 nClassId = FormComponentType::CONTROL;

            Reference< XPropertySet > xModelProps;
            if ( pControls->is() )
                xModelProps = xModelProps.query( (*pControls)->getModel() );
            if ( xModelProps.is() )
                xModelProps->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;

            // controls which are not focussable
            if  (   ( FormComponentType::CONTROL != nClassId )
                &&  ( FormComponentType::IMAGEBUTTON != nClassId )
                &&  ( FormComponentType::GROUPBOX != nClassId )
                &&  ( FormComponentType::FIXEDTEXT != nClassId )
                &&  ( FormComponentType::HIDDENCONTROL != nClassId )
                &&  ( FormComponentType::IMAGECONTROL != nClassId )
                &&  ( FormComponentType::SCROLLBAR != nClassId )
                &&  ( FormComponentType::SPINBUTTON!= nClassId )
                )
            {
                xReturn = *pControls;
                break;
            }
        }
        catch( const Exception& e )
        {
            e;  // make compiler happy
        }

        if ( !xReturn.is() && _rControls.getLength() )
            xReturn = _rControls[0];
    }

    return xReturn;
}

// -----------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnAutoFocus, void*, EMPTYTAG)
{
    m_nAutoFocusEvent = 0;

    // go to the first form of our page, examine it's TabController, go to it's first (in terms of the tab order)
    // control, give it the focus

    // get the forms collection of the page we belong to
    FmFormPage* pPage = m_pView ? PTR_CAST(FmFormPage, m_pView->GetPageViewPvNum(0)->GetPage()) : NULL;
    Reference< XIndexAccess > xForms;
    if (pPage)
        xForms = Reference< XIndexAccess >(pPage->GetForms(), UNO_QUERY);
    FmXPageViewWinRec* pViewWinRec = m_aWinList.size() ? m_aWinList[0] : NULL;
    if (pViewWinRec)
    {
        try
        {
            // go for the tab controller of the first form
            sal_Int32 nObjects = xForms->getCount();
            Reference< XForm > xForm;
            if (nObjects)
                ::cppu::extractInterface(xForm, xForms->getByIndex(0));

            Reference< XTabController > xTabControllerModel(pViewWinRec->getController( xForm ), UNO_QUERY);
            // go for the first control of the controller
            Sequence< Reference< XControl > > aControls;
            if (xTabControllerModel.is())
                aControls = xTabControllerModel->getControls();

            // set the focus to this first control
            Reference< XWindow > xControlWindow( lcl_firstFocussableControl( aControls ), UNO_QUERY );
            if (xControlWindow.is())
                xControlWindow->setFocus();

            // ensure that the control is visible
            // 80210 - 12/07/00 - FS
            if (xControlWindow.is() && m_pView->GetActualOutDev() && (OUTDEV_WINDOW == m_pView->GetActualOutDev()->GetOutDevType()))
            {
                const Window* pWindow = static_cast<const Window*>(m_pView->GetActualOutDev());
                awt::Rectangle aRect = xControlWindow->getPosSize();
                ::Rectangle aNonUnoRect(aRect.X, aRect.Y, aRect.X + aRect.Width, aRect.Y + aRect.Height);
                m_pView->MakeVisible(pWindow->PixelToLogic(aNonUnoRect), *const_cast<Window*>(pWindow));
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("FmXFormView::OnAutoFocus: could not activate the first control!");
        }
    }
    return 0L;
}

// -----------------------------------------------------------------------------
namespace
{
    //....................................................................
    template< class TYPE >
    Reference< TYPE > getTypedModelNode( const Reference< XInterface >& _rxModelNode )
    {
        Reference< TYPE > xTypedNode( _rxModelNode, UNO_QUERY );
        if ( xTypedNode.is() )
            return xTypedNode;
        else
        {
            Reference< XChild > xChild( _rxModelNode, UNO_QUERY );
            if ( xChild.is() )
                return getTypedModelNode< TYPE >( xChild->getParent() );
            else
                return NULL;
        }
    }

    //....................................................................
    static bool lcl_getDocumentDefaultStyleAndFamily( const Reference< XInterface >& _rxDocument, ::rtl::OUString& _rFamilyName, ::rtl::OUString& _rStyleName ) SAL_THROW(( Exception ))
    {
        bool bSuccess = true;
        Reference< XServiceInfo > xDocumentSI( _rxDocument, UNO_QUERY );
        if ( xDocumentSI.is() )
        {
            if (  xDocumentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ) ) )
               || xDocumentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.WebDocument" ) ) )
               )
            {
                _rFamilyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParagraphStyles" ) );
                _rStyleName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
            }
            else if ( xDocumentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" ) ) ) )
            {
                _rFamilyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CellStyles" ) );
                _rStyleName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Default" ) );
            }
            else if (  xDocumentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) )
                    || xDocumentSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) )
                    )
            {
                _rFamilyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "graphics" ) );
                _rStyleName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "standard" ) );
            }
            else
                bSuccess = false;
        }
        return bSuccess;
    }

    static const sal_Char* aCharacterAndParagraphProperties[] =
    {
        "CharFontName",
        "CharFontStyleName",
        "CharFontFamily",
        "CharFontCharSet",
        "CharFontPitch",
        "CharColor",
        "CharEscapement",
        "CharHeight",
        "CharUnderline",
        "CharWeight",
        "CharPosture",
        "CharAutoKerning",
        "CharBackColor",
        "CharBackTransparent",
        "CharCaseMap",
        "CharCrossedOut",
        "CharFlash",
        "CharStrikeout",
        "CharWordMode",
        "CharKerning",
        "CharLocale",
        "CharKeepTogether",
        "CharNoLineBreak",
        "CharShadowed",
        "CharFontType",
        "CharStyleName",
        "CharContoured",
        "CharCombineIsOn",
        "CharCombinePrefix",
        "CharCombineSuffix",
        "CharEmphasize",
        "CharRelief",
        "RubyText",
        "RubyAdjust",
        "RubyCharStyleName",
        "RubyIsAbove",
        "CharRotation",
        "CharRotationIsFitToLine",
        "CharScaleWidth",
        "HyperLinkURL",
        "HyperLinkTarget",
        "HyperLinkName",
        "VisitedCharStyleName",
        "UnvisitedCharStyleName",
        "CharEscapementHeight",
        "CharNoHyphenation",
        "CharUnderlineColor",
        "CharUnderlineHasColor",
        "CharStyleNames",
        "CharHeightAsian",
        "CharWeightAsian",
        "CharFontNameAsian",
        "CharFontStyleNameAsian",
        "CharFontFamilyAsian",
        "CharFontCharSetAsian",
        "CharFontPitchAsian",
        "CharPostureAsian",
        "CharLocaleAsian",
        "ParaIsCharacterDistance",
        "ParaIsForbiddenRules",
        "ParaIsHangingPunctuation",
        "CharHeightComplex",
        "CharWeightComplex",
        "CharFontNameComplex",
        "CharFontStyleNameComplex",
        "CharFontFamilyComplex",
        "CharFontCharSetComplex",
        "CharFontPitchComplex",
        "CharPostureComplex",
        "CharLocaleComplex",
        "ParaAdjust",
        "ParaLineSpacing",
        "ParaBackColor",
        "ParaBackTransparent",
        "ParaBackGraphicURL",
        "ParaBackGraphicFilter",
        "ParaBackGraphicLocation",
        "ParaLastLineAdjust",
        "ParaExpandSingleWord",
        "ParaLeftMargin",
        "ParaRightMargin",
        "ParaTopMargin",
        "ParaBottomMargin",
        "ParaLineNumberCount",
        "ParaLineNumberStartValue",
        "PageDescName",
        "PageNumberOffset",
        "ParaRegisterModeActive",
        "ParaTabStops",
        "ParaStyleName",
        "DropCapFormat",
        "DropCapWholeWord",
        "ParaKeepTogether",
        "Setting",
        "ParaSplit",
        "Setting",
        "NumberingLevel",
        "NumberingRules",
        "NumberingStartValue",
        "ParaIsNumberingRestart",
        "NumberingStyleName",
        "ParaOrphans",
        "ParaWidows",
        "ParaShadowFormat",
        "LeftBorder",
        "RightBorder",
        "TopBorder",
        "BottomBorder",
        "BorderDistance",
        "LeftBorderDistance",
        "RightBorderDistance",
        "TopBorderDistance",
        "BottomBorderDistance",
        "BreakType",
        "DropCapCharStyleName",
        "ParaFirstLineIndent",
        "ParaIsAutoFirstLineIndent",
        "ParaIsHyphenation",
        "ParaHyphenationMaxHyphens",
        "ParaHyphenationMaxLeadingChars",
        "ParaHyphenationMaxTrailingChars",
        "ParaVertAlignment",
        "ParaUserDefinedAttributes",
        "NumberingIsNumber",
        "ParaIsConnectBorder",
        NULL
    };

    //....................................................................
    static void lcl_initializeCharacterAttributes( const Reference< XPropertySet >& _rxModel )
    {
        // need to initialize the attributes from the "Default" style of the document we live in

        try
        {
            // the style family collection
            Reference< XStyleFamiliesSupplier > xSuppStyleFamilies = getTypedModelNode< XStyleFamiliesSupplier >( _rxModel.get() );
            Reference< XNameAccess > xStyleFamilies;
            if ( xSuppStyleFamilies.is() )
                xStyleFamilies = xSuppStyleFamilies->getStyleFamilies();
            OSL_ENSURE( xStyleFamilies.is(), "lcl_initializeCharacterAttributes: could not obtain the style families!" );
            if ( !xStyleFamilies.is() )
                return;

            // the names of the family, and the style - depends on the document type we live in
            ::rtl::OUString sFamilyName, sStyleName;
            bool bKnownDocumentType = lcl_getDocumentDefaultStyleAndFamily( xSuppStyleFamilies.get(), sFamilyName, sStyleName );
            OSL_ENSURE( bKnownDocumentType, "lcl_initializeCharacterAttributes: Huh? What document type is this?" );
            if ( !bKnownDocumentType )
                return;

            // the concrete style
            Reference< XNameAccess > xStyleFamily( xStyleFamilies->getByName( sFamilyName ), UNO_QUERY );
            Reference< XPropertySet > xStyle;
            if ( xStyleFamily.is() )
                xStyleFamily->getByName( sStyleName ) >>= xStyle;
            OSL_ENSURE( xStyle.is(), "lcl_initializeCharacterAttributes: could not retrieve the style!" );
            if ( !xStyle.is() )
                return;

            // transfer all properties which are described by the com.sun.star.style.
            Reference< XPropertySetInfo > xSourcePropInfo( xStyle->getPropertySetInfo() );
            Reference< XPropertySetInfo > xDestPropInfo( _rxModel->getPropertySetInfo() );
            OSL_ENSURE( xSourcePropInfo.is() && xDestPropInfo.is(), "lcl_initializeCharacterAttributes: no property set info!" );
            if ( !xSourcePropInfo.is() || !xDestPropInfo.is() )
                return;

            ::rtl::OUString sPropertyName;
            const sal_Char** pCharacterProperty = aCharacterAndParagraphProperties;
            while ( *pCharacterProperty )
            {
                sPropertyName = ::rtl::OUString::createFromAscii( *pCharacterProperty );

                if ( xSourcePropInfo->hasPropertyByName( sPropertyName ) && xDestPropInfo->hasPropertyByName( sPropertyName ) )
                    _rxModel->setPropertyValue( sPropertyName, xStyle->getPropertyValue( sPropertyName ) );

                ++pCharacterProperty;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_initializeCharacterAttributes: caught an exception!" );
        }
    }
}

// -----------------------------------------------------------------------------
sal_Int16 FmXFormView::implInitializeNewControlModel( const Reference< XPropertySet >& _rxModel, const SdrObject* _pObject ) const
{
    OSL_ENSURE( _rxModel.is() && _pObject, "FmXFormView::implInitializeNewControlModel: invalid model!" );

    sal_Int16 nClassId = FormComponentType::CONTROL;
    if ( !_rxModel.is() || !_pObject )
        return nClassId;

    try
    {
        // if the control lives in an eForm or database, give it some nicer layout
        // Notice that in such documents, and FmXFormController will additionally enable
        // dynamic control border colors (i.e. the color changes when the mouse hovers over
        // the control, or the control has the focus)
        Reference< XPropertySetInfo > xPSI( _rxModel->getPropertySetInfo() );
        if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_BORDER ) )
        {
            sal_Int16 nCurrentBorder = VisualEffect::NONE;
            OSL_VERIFY( _rxModel->getPropertyValue( FM_PROP_BORDER ) >>= nCurrentBorder );
            if ( nCurrentBorder != VisualEffect::NONE )
            {
                DocumentType eDocType = DocumentClassification::classifyHostDocument( _rxModel.get() );
                if ( ( eDocType == eElectronicForm ) || ( eDocType == eDatabaseForm ) )
                {
                    _rxModel->setPropertyValue( FM_PROP_BORDER, makeAny( VisualEffect::FLAT ) );
                    OSL_ENSURE( xPSI->hasPropertyByName( FM_PROP_BORDERCOLOR ), "FmXFormView::implInitializeNewControlModel: Border, but no border color?" );
                    if ( xPSI->hasPropertyByName( FM_PROP_BORDERCOLOR ) )
                        _rxModel->setPropertyValue( FM_PROP_BORDERCOLOR, makeAny( (sal_Int32)0x00C0C0C0 ) );  // light gray
                }
            }
        }

        _rxModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;

        switch ( nClassId )
        {
            case FormComponentType::SCROLLBAR:
                _rxModel->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LiveScroll" ) ), makeAny( (sal_Bool)sal_True ) );
                // NO break!
            case FormComponentType::SPINBUTTON:
            {
                const ::Rectangle& rBoundRect = _pObject->GetCurrentBoundRect();
                sal_Int32 eOrientation = ScrollBarOrientation::HORIZONTAL;
                if ( rBoundRect.GetWidth() < rBoundRect.GetHeight() )
                    eOrientation = ScrollBarOrientation::VERTICAL;
                _rxModel->setPropertyValue( FM_PROP_ORIENTATION, makeAny( eOrientation ) );
            }
            break;

            case FormComponentType::LISTBOX:
            case FormComponentType::COMBOBOX:
            {
                const ::Rectangle& rBoundRect = _pObject->GetCurrentBoundRect();
                sal_Bool bDropDown = ( rBoundRect.GetWidth() >= 3 * rBoundRect.GetHeight() );
                _rxModel->setPropertyValue( FM_PROP_DROPDOWN, makeAny( (sal_Bool)bDropDown ) );
            }
            break;

            case FormComponentType::TEXTFIELD:
            {
                initializeTextFieldLineEnds( _rxModel, m_xORB );
                lcl_initializeCharacterAttributes( _rxModel );

                const ::Rectangle& rBoundRect = _pObject->GetCurrentBoundRect();
                if ( !( rBoundRect.GetWidth() > 4 * rBoundRect.GetHeight() ) )  // heuristics
                {
                    Reference< XPropertySetInfo > xProps( _rxModel->getPropertySetInfo() );
                    if ( xProps.is() && xProps->hasPropertyByName( FM_PROP_MULTILINE ) )
                        _rxModel->setPropertyValue( FM_PROP_MULTILINE, makeAny( (sal_Bool)sal_True ) );
                }
            }
            break;
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormView::implInitializeNewControlModel: caught an exception!" );
    }
    return nClassId;
}

// -----------------------------------------------------------------------------
SdrObject* FmXFormView::implCreateFieldControl( const ::svx::ODataAccessDescriptor& _rColumnDescriptor )
{
    // not if we're in design mode
    if ( !m_pView->IsDesignMode() )
        return NULL;

    // the very basic information
    ::rtl::OUString sCommand, sFieldName;
    sal_Int32 nCommandType = CommandType::COMMAND;

    ::rtl::OUString sDataSource = _rColumnDescriptor.getDataSource();
    _rColumnDescriptor[ daCommand ]     >>= sCommand;
    _rColumnDescriptor[ daColumnName ]  >>= sFieldName;
    _rColumnDescriptor[ daCommandType ] >>= nCommandType;

    if ( !sDataSource.getLength() || !sCommand.getLength() || !sFieldName.getLength() )
        return NULL;

    // additional (and optional) information
    Reference< XConnection > xConnection;
    if ( _rColumnDescriptor.has( daConnection ) )
        _rColumnDescriptor[ daConnection ]  >>= xConnection;

    // did we get the connection from outside (no need to dispose it then)?
    sal_Bool bForeignConnection = xConnection.is();

    // obtain the data source
    Reference< XDataSource > xDataSource;
    SQLErrorEvent aError;
    try
    {
        xDataSource = OStaticDataAccessTools().getDataSource(sDataSource, getORB());
        // and the connection, if necessary
        if ( !bForeignConnection )
            xConnection = OStaticDataAccessTools().getConnection_withFeedback(sDataSource,::rtl::OUString(),::rtl::OUString(), getORB());
    }
    catch(const SQLContext& e) { aError.Reason <<= e; }
    catch(const SQLWarning& e) { aError.Reason <<= e; }
    catch(const SQLException& e) { aError.Reason <<= e; }
    if (aError.Reason.hasValue())
    {
        displayAsyncErrorMessage( aError );
        return NULL;
    }

    // need a data source and a connection here
    if (!xDataSource.is() || !xConnection.is())
    {
        DBG_ERROR("FmXFormView::implCreateFieldControl : could not retrieve the data source or the connection!");
        return NULL;
    }

    // from now on, if something goes wrong, automatically dispose the connection
    Reference< XInterface > xAutoDisposee;
    if ( !bForeignConnection )
        xAutoDisposee = xConnection.get();
    OAutoDispose aDisposeConnection( xAutoDisposee );


    OStaticDataAccessTools aDBATools;
    Reference< XComponent > xKeepFieldsAlive;
    // go
    try
    {
        FmFormPage& rPage = *static_cast<FmFormPage*>(m_pView->GetPageViewPvNum(0)->GetPage());

        // determine the table/query field which we should create a control for
        Reference< XPropertySet >   xField;

        Reference< XNameAccess >    xFields = aDBATools.getFieldsByCommandDescriptor(
            xConnection, nCommandType, sCommand, xKeepFieldsAlive );

        if (xFields.is() && xFields->hasByName(sFieldName))
            xFields->getByName(sFieldName) >>= xField;

        Reference< XNumberFormatsSupplier >  xSupplier = aDBATools.getNumberFormats(xConnection, sal_False);
        if (!xSupplier.is() || !xField.is())
            return NULL;

        Reference< XNumberFormats >  xNumberFormats(xSupplier->getNumberFormats());
        if (!xNumberFormats.is())
            return NULL;

        // Vom Feld werden nun zwei Informationen benoetigt:
        // a.) Name des Feldes fuer Label und ControlSource
        // b.) FormatKey, um festzustellen, welches Feld erzeugt werden soll
        sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));
        sal_Int32 nFormatKey = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FORMATKEY));

        ::rtl::OUString sLabelPostfix;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* _pOutDev = NULL;
        if (m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
            _pOutDev = const_cast<OutputDevice*>(m_pView->GetActualOutDev());
        else
        {// OutDev suchen
            SdrPageView* pPageView = m_pView->GetPageViewPvNum(0);
            if( pPageView && !_pOutDev )
            {
                // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

                for( sal_uInt32 i = 0L; i < pPageView->WindowCount(); i++ )
                {
                    const SdrPageViewWindow& rPageViewWindow = *pPageView->GetWindow(i);

                    if( rPageViewWindow.GetOutputDevice().GetOutDevType() == OUTDEV_WINDOW)
                    {
                        _pOutDev = &rPageViewWindow.GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if (!_pOutDev)
            return NULL;

        if ((DataType::BINARY == nDataType) || (DataType::VARBINARY == nDataType))
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // Anhand des FormatKeys wird festgestellt, welches Feld benoetigt wird
        sal_uInt16 nOBJID = 0;
        sal_Bool bDateNTimeField = sal_False;

        sal_Bool bIsCurrency = sal_False;
        if (::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField))
            bIsCurrency = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY));

        if (bIsCurrency)
            nOBJID = OBJ_FM_CURRENCYFIELD;
        else
            switch (nDataType)
            {
                case DataType::LONGVARBINARY:
                    nOBJID = OBJ_FM_IMAGECONTROL;
                    break;
                case DataType::LONGVARCHAR:
                    nOBJID = OBJ_FM_EDIT;
                    break;
                case DataType::BINARY:
                case DataType::VARBINARY:
                    return NULL;
                case DataType::BIT:
                    nOBJID = OBJ_FM_CHECKBOX;
                    break;
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                    nOBJID = OBJ_FM_NUMERICFIELD;
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                    nOBJID = OBJ_FM_FORMATTEDFIELD;
                    break;
                case DataType::TIMESTAMP:
                    bDateNTimeField = sal_True;
                    sLabelPostfix = UniString(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX)).GetToken(0, ';');
                    // DON'T break !
                case DataType::DATE:
                    nOBJID = OBJ_FM_DATEFIELD;
                    break;
                case DataType::TIME:
                    nOBJID = OBJ_FM_TIMEFIELD;
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                default:
                    nOBJID = OBJ_FM_EDIT;
                    break;
            }
        if (!nOBJID)
            return NULL;

        FmFormObj* pLabel;
        FmFormObj* pControl;
        createControlLabelPair(_pOutDev, 0, xField, xNumberFormats, nOBJID, sLabelPostfix, pLabel, pControl);
        if (!pLabel || !pControl)
        {
            delete pLabel;
            delete pControl;
            return NULL;
        }

        //////////////////////////////////////////////////////////////////////
        // Feststellen ob eine form erzeugt werden muss
        // Dieses erledigt die Page fuer uns bzw. die PageImpl
        Reference< XFormComponent >  xContent(pLabel->GetUnoControlModel(), UNO_QUERY);
        Reference< XIndexContainer >  xContainer(rPage.GetImpl()->SetDefaults(xContent, xDataSource, sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
        implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pLabel );

        xContent = Reference< XFormComponent > (pControl->GetUnoControlModel(), UNO_QUERY);
        xContainer = Reference< XIndexContainer > (rPage.GetImpl()->SetDefaults(xContent, xDataSource, sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
        implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pControl );

        //////////////////////////////////////////////////////////////////////
        // Objekte gruppieren
        SdrObjGroup* pGroup  = new SdrObjGroup();
        SdrObjList* pObjList = pGroup->GetSubList();
        pObjList->InsertObject(pLabel);
        pObjList->InsertObject(pControl);


        if (bDateNTimeField)
        {   // wir haben bis jetzt nur ein Datums-Feld eingefuegt, brauchen aber noch ein extra Feld fuer
            // die Zeit-Komponente
            pLabel = pControl = NULL;
            createControlLabelPair(_pOutDev, 1000, xField, xNumberFormats, OBJ_FM_TIMEFIELD,
                UniString(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX)).GetToken(1, ';'),
                pLabel, pControl);

            if (pLabel && pControl)
            {
                pObjList->InsertObject(pLabel);
                pObjList->InsertObject(pControl);
            }
            else
            {
                delete pLabel;
                delete pControl;
            }
        }

        return pGroup; // und fertig
    }
    catch(const Exception&)
    {
        DBG_ERROR("FmXFormView::implCreateFieldControl: caught an exception while creating the control !");
    }


    return NULL;
}

//------------------------------------------------------------------------
void FmXFormView::createControlLabelPair(OutputDevice* _pOutDev, sal_Int32 _nYOffsetMM,
    const Reference< XPropertySet >& _rxField, const Reference< XNumberFormats >& _rxNumberFormats,
    sal_uInt16 _nObjID, const ::rtl::OUString& _rFieldPostfix,
    FmFormObj*& _rpLabel, FmFormObj*& _rpControl) const
{
    sal_Int32 nDataType = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FIELDTYPE));
    sal_Int32 nFormatKey = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FORMATKEY));

    Any aFieldName(_rxField->getPropertyValue(FM_PROP_NAME));
    ::rtl::OUString sFieldName;
    aFieldName >>= sFieldName;

    // das Label
    _rpLabel = (FmFormObj*)SdrObjFactory::MakeNewObject( FmFormInventor, OBJ_FM_FIXEDTEXT, NULL, NULL );
    Reference< XPropertySet > xLabelSet(_rpLabel->GetUnoControlModel(), UNO_QUERY);
    xLabelSet->setPropertyValue(FM_PROP_LABEL, makeAny(sFieldName + _rFieldPostfix));

    // positionieren unter Beachtung der Einstellungen des Ziel-Output-Devices
    ::Size aTextSize(_pOutDev->GetTextWidth(sFieldName + _rFieldPostfix), _pOutDev->GetTextHeight());

    SdrModel* pModel    = m_pView->GetModel();
    MapMode   eTargetMode(_pOutDev->GetMapMode()),
              eSourceMode(MAP_100TH_MM);

    // Textbreite ist mindestens 5cm
    // Texthoehe immer halber cm
    ::Size aDefTxtSize(3000, 500);
    ::Size aDefSize(4000, 500);
    ::Size aDefImageSize(4000, 4000);
    // Abstand zwischen Text und Control
    ::Size aDelta(500, 0);

    ::Size aRealSize = _pOutDev->LogicToLogic(aTextSize, eTargetMode, eSourceMode);
    aRealSize.Width() = std::max(aRealSize.Width(), aDefTxtSize.Width()) + aDelta.Width();
    aRealSize.Height()= aDefSize.Height();

    // je nach Skalierung des Zieldevices muss die Groesse noch normiert werden (#53523#)
    aRealSize.Width() = sal_Int32(Fraction(aRealSize.Width(), 1) * eTargetMode.GetScaleX());
    aRealSize.Height() = sal_Int32(Fraction(aRealSize.Height(), 1) * eTargetMode.GetScaleY());
    _rpLabel->SetLogicRect(
        ::Rectangle(    _pOutDev->LogicToLogic(Point(0, _nYOffsetMM), eSourceMode, eTargetMode),
                    _pOutDev->LogicToLogic(aRealSize, eSourceMode, eTargetMode)
        ));

    // jetzt das Control
    _rpControl = static_cast<FmFormObj*>(SdrObjFactory::MakeNewObject( FmFormInventor, _nObjID, NULL, NULL ));
    Reference< XPropertySet > xControlSet = Reference< XPropertySet > ( _rpControl->GetUnoControlModel(), UNO_QUERY );

    // positionieren
    ::Size szControlSize;
    if (DataType::BIT == nDataType)
        szControlSize = aDefSize;
    else if (OBJ_FM_IMAGECONTROL == _nObjID || DataType::LONGVARCHAR == nDataType)
        szControlSize = aDefImageSize;
    else
        szControlSize = aDefSize;

    // normieren wie oben
    szControlSize.Width() = sal_Int32(Fraction(szControlSize.Width(), 1) * eTargetMode.GetScaleX());
    szControlSize.Height() = sal_Int32(Fraction(szControlSize.Height(), 1) * eTargetMode.GetScaleY());
    _rpControl->SetLogicRect(
        ::Rectangle(    _pOutDev->LogicToLogic(Point(aRealSize.Width(), _nYOffsetMM), eSourceMode, eTargetMode),
                    _pOutDev->LogicToLogic(szControlSize, eSourceMode, eTargetMode)
        ));

    // ein paar initiale Einstellungen am ControlModel
    if (xControlSet.is())
    {
        // ein paar numersiche Eigenschaften durchschleifen
        if (::comphelper::hasProperty(FM_PROP_DECIMAL_ACCURACY, xControlSet))
        {
            // Number braucht eine Scale
            Any aScaleVal(::comphelper::getNumberFormatDecimals(_rxNumberFormats, nFormatKey));
            xControlSet->setPropertyValue(FM_PROP_DECIMAL_ACCURACY, aScaleVal);
        }
        if (::comphelper::hasProperty(FM_PROP_VALUEMIN, xControlSet) && ::comphelper::hasProperty(FM_PROP_VALUEMAX, xControlSet))
        {
            // die minimale/maximale Zahl in diesem Feld
            sal_Int32 nMinValue = -1000000000, nMaxValue = 1000000000;
            switch (nDataType)
            {
                case DataType::TINYINT  : nMinValue = 0; nMaxValue = 255; break;
                case DataType::SMALLINT : nMinValue = -32768; nMaxValue = 32767; break;
                case DataType::INTEGER  : nMinValue = 0x80000000; nMaxValue = 0x7FFFFFFF; break;
                    // um die doubles/singles kuemmere ich mich nicht, da es ein wenig sinnlos ist
            }

            Reference< XPropertySetInfo > xControlPropInfo = xControlSet->getPropertySetInfo();
            Any aVal;

            Property aMinProp = xControlPropInfo->getPropertyByName(FM_PROP_VALUEMIN);
            if (aMinProp.Type.getTypeClass() == TypeClass_DOUBLE)
                aVal <<= (double)nMinValue;
            else if (aMinProp.Type.getTypeClass() == TypeClass_LONG)
                aVal <<= (sal_Int32)nMinValue;
            else
                DBG_ERROR("FmXFormView::createControlLabelPair: unexpected property type (MinValue)!");
            xControlSet->setPropertyValue(FM_PROP_VALUEMIN,aVal);

            Property aMaxProp = xControlPropInfo->getPropertyByName(FM_PROP_VALUEMAX);
            if (aMaxProp.Type.getTypeClass() == TypeClass_DOUBLE)
                aVal <<= (double)nMaxValue;
            else if (aMaxProp.Type.getTypeClass() == TypeClass_LONG)
                aVal <<= (sal_Int32)nMaxValue;
            else
                DBG_ERROR("FmXFormView::createControlLabelPair: unexpected property type (MaxValue)!");
            xControlSet->setPropertyValue(FM_PROP_VALUEMAX,aVal);
        }

        if (::comphelper::hasProperty(FM_PROP_STRICTFORMAT, xControlSet))
        {   // Formatueberpruefung fue numeric fields standardmaessig sal_True
            sal_Bool bB(sal_True);
            Any aVal(&bB,getBooleanCppuType());
            xControlSet->setPropertyValue(FM_PROP_STRICTFORMAT, aVal);
        }

        xControlSet->setPropertyValue(FM_PROP_CONTROLSOURCE, aFieldName);
        xControlSet->setPropertyValue(FM_PROP_NAME, aFieldName);

        if (nDataType == DataType::LONGVARCHAR)
        {
            sal_Bool bB(sal_True);
            xControlSet->setPropertyValue(FM_PROP_MULTILINE,Any(&bB,getBooleanCppuType()));
        }

        if (_nObjID == OBJ_FM_CHECKBOX)
        {
            sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
            _rxField->getPropertyValue( FM_PROP_ISNULLABLE ) >>= nNullable;
            xControlSet->setPropertyValue( FM_PROP_TRISTATE, makeAny( sal_Bool( ColumnValue::NULLABLE == nNullable ) ) );
        }
    }

    // announce the label to the control
    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xControlSet))
    {
        // (try-catch as the control may refuse a model without the right service name - which we don't know
        // usually a fixed text we use as label should be accepted, but to be sure ....)
        try
        {
            xControlSet->setPropertyValue(FM_PROP_CONTROLLABEL, makeAny(xLabelSet));
        }
        catch(Exception&)
        {
            DBG_ERROR("FmXFormView::createControlLabelPair : could not marry the control and the label !");
        }
    }
}

//------------------------------------------------------------------------------
FmXFormView::ObjectRemoveListener::ObjectRemoveListener( FmXFormView* pParent )
    :m_pParent( pParent )
{
}

//------------------------------------------------------------------------------
void FmXFormView::ObjectRemoveListener::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA(SdrHint) && (((SdrHint&)rHint).GetKind() == HINT_OBJREMOVED))
        m_pParent->ObjectRemovedInAliveMode(((SdrHint&)rHint).GetObject());
}

//------------------------------------------------------------------------------
void FmXFormView::ObjectRemovedInAliveMode( const SdrObject* pObject )
{
    // wenn das entfernte Objekt in meiner MarkList, die ich mir beim Umschalten in den Alive-Mode gemerkt habe, steht,
    // muss ich es jetzt da rausnehmen, da ich sonst beim Zurueckschalten versuche, die Markierung wieder zu setzen
    // (interesanterweise geht das nur bei gruppierten Objekten schief (beim Zugriff auf deren ObjList GPF), nicht bei einzelnen)

    sal_uInt32 nCount = m_aMark.GetMarkCount();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        SdrMark* pMark = m_aMark.GetMark(i);
        SdrObject* pCurrent = pMark->GetObj();
        if (pObject == pCurrent)
        {
            m_aMark.DeleteMark(i);
            return;
        }
        // ich brauche nicht in GroupObjects absteigen : wenn dort unten ein Objekt geloescht wird, dann bleibt der
        // Zeiger auf das GroupObject, den ich habe, trotzdem weiter gueltig bleibt ...
    }
}

//------------------------------------------------------------------------------
void FmXFormView::stopMarkListWatching()
{
    if ( m_pWatchStoredList )
    {
        m_pWatchStoredList->EndListeningAll();
        delete m_pWatchStoredList;
        m_pWatchStoredList = NULL;
    }
}

//------------------------------------------------------------------------------
void FmXFormView::startMarkListWatching()
{
    if ( !m_pWatchStoredList )
    {
        m_pWatchStoredList = new ObjectRemoveListener( this );
        FmFormModel* pModel = GetFormShell() ? GetFormShell()->GetFormModel() : NULL;
        DBG_ASSERT( pModel != NULL, "FmXFormView::startMarkListWatching: shell has no model!" );
        m_pWatchStoredList->StartListening( *static_cast< SfxBroadcaster* >( pModel ) );
    }
    else
        DBG_ERROR( "FmXFormView::startMarkListWatching: already listening!" );
}

//------------------------------------------------------------------------------
void FmXFormView::saveMarkList( sal_Bool _bSmartUnmark )
{
    if ( m_pView )
    {
        m_aMark = m_pView->GetMarkedObjectList();
        if ( _bSmartUnmark )
        {
            sal_uInt32 nCount = m_aMark.GetMarkCount( );
            for ( sal_uInt32 i = 0; i < nCount; ++i )
            {
                SdrMark*   pMark = m_aMark.GetMark(i);
                SdrObject* pObj  = pMark->GetObj();

                if ( m_pView->IsObjMarked( pObj ) )
                {
                    if ( pObj->IsGroupObject() )
                    {
                        SdrObjListIter aIter( *pObj->GetSubList() );
                        sal_Bool bMixed = sal_False;
                        while ( aIter.IsMore() && !bMixed )
                            bMixed = ( aIter.Next()->GetObjInventor() != FmFormInventor );

                        if ( !bMixed )
                        {
                            // all objects in the group are form objects
                            m_pView->MarkObj( pMark->GetObj(), pMark->GetPageView(), sal_True /* unmark! */ );
                        }
                    }
                    else
                    {
                        if ( pObj->GetObjInventor() == FmFormInventor )
                        {   // this is a form layer object
                            m_pView->MarkObj( pMark->GetObj(), pMark->GetPageView(), sal_True /* unmark! */ );
                        }
                    }
                }
            }
        }
    }
    else
    {
        DBG_ERROR( "FmXFormView::saveMarkList: invalid view!" );
        m_aMark = SdrMarkList();
    }
}

//--------------------------------------------------------------------------
static sal_Bool lcl_hasObject( SdrObjListIter& rIter, SdrObject* pObj )
{
    sal_Bool bFound = sal_False;
    while (rIter.IsMore() && !bFound)
        bFound = pObj == rIter.Next();

    rIter.Reset();
    return bFound;
}

//------------------------------------------------------------------------------
void FmXFormView::restoreMarkList( SdrMarkList& _rRestoredMarkList )
{
    if ( !m_pView )
        return;

    _rRestoredMarkList.Clear();

    const SdrMarkList& rCurrentList = m_pView->GetMarkedObjectList();
    FmFormPage* pPage = GetFormShell() ? GetFormShell()->GetCurPage() : NULL;
    if (pPage)
    {
        if (rCurrentList.GetMarkCount())
        {   // there is a current mark ... hmm. Is it a subset of the mark we remembered in saveMarkList?
            sal_Bool bMisMatch = sal_False;

            // loop through all current marks
            sal_uInt32 nCurrentCount = rCurrentList.GetMarkCount();
            for ( sal_uInt32 i=0; i<nCurrentCount&& !bMisMatch; ++i )
            {
                const SdrObject* pCurrentMarked = rCurrentList.GetMark( i )->GetObj();

                // loop through all saved marks, check for equality
                sal_Bool bFound = sal_False;
                sal_uInt32 nSavedCount = m_aMark.GetMarkCount();
                for ( sal_uInt32 j=0; j<nSavedCount && !bFound; ++j )
                {
                    if ( m_aMark.GetMark( j )->GetObj() == pCurrentMarked )
                        bFound = sal_True;
                }

                // did not find a current mark in the saved marks
                if ( !bFound )
                    bMisMatch = sal_True;
            }

            if ( bMisMatch )
            {
                m_aMark.Clear();
                _rRestoredMarkList = rCurrentList;
                return;
            }
        }
        // wichtig ist das auf die Objecte der markliste nicht zugegriffen wird
        // da diese bereits zerstoert sein koennen
        SdrPageView* pCurPageView = m_pView->GetPageViewPvNum(0);
        SdrObjListIter aPageIter( *pPage );
        sal_Bool bFound = sal_True;

        // gibt es noch alle Objecte
        sal_uInt32 nCount = m_aMark.GetMarkCount();
        for (sal_uInt32 i = 0; i < nCount && bFound; i++)
        {
            SdrMark*   pMark = m_aMark.GetMark(i);
            SdrObject* pObj  = pMark->GetObj();
            if (pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj->GetSubList());
                while (aIter.IsMore() && bFound)
                    bFound = lcl_hasObject(aPageIter, aIter.Next());
            }
            else
                bFound = lcl_hasObject(aPageIter, pObj);

            bFound = bFound && pCurPageView == pMark->GetPageView();
        }

        if (bFound)
        {
            // Das LastObject auswerten
            if (nCount) // Objecte jetzt Markieren
            {
                for (sal_uInt32 i = 0; i < nCount; i++)
                {
                    SdrMark* pMark = m_aMark.GetMark(i);
                    SdrObject* pObj = pMark->GetObj();
                    if ( pObj->GetObjInventor() == FmFormInventor )
                        if ( !m_pView->IsObjMarked( pObj ) )
                            m_pView->MarkObj( pObj, pMark->GetPageView() );
                }

                _rRestoredMarkList = m_aMark;
            }
            // This one is from the times where the code was part of the FmXFormShell instead of the FmXFormView.
            // I do not see the deeper sense - it's not fully clear what it does, and everything works as
            // expected without it. Fine, so I omit it :).
            // 02.05.2002 - fs@openoffice.org
//          else
//          {
//              Reference< XIndexAccess> xCont(pPage->GetForms(), UNO_QUERY);
//
//              // Ist das aktuelle Object ein Element eines SelectionSuppliers?
//              Reference< XChild> xChild(m_xCurControl,UNO_QUERY);
//              Reference< XSelectionSupplier> xSelectionSupplier;
//              if (xChild.is())
//                  xSelectionSupplier = Reference< XSelectionSupplier>(xChild->getParent(), UNO_QUERY);
//              if (xSelectionSupplier.is())
//              {
//                  // suchen der Zugehoreigen Form
//                  Reference< XForm> xForm(GetForm(m_xCurControl));
//                  Reference< XInterface> xIface(xForm, UNO_QUERY);
//                  if (xForm.is() && searchElement(xCont, xIface))
//                  {
//                      setCurForm(xForm);
//                      setCurControl(m_xCurControl);
//
//                      // jetzt noch die Selection vornehmen
//                      xSelectionSupplier->select(makeAny(m_xCurControl));
//                  }
//              }
//              else
//              {
//                  // Auswerten des letzen Objects
//                  Reference< XForm> xForm(m_xCurForm, UNO_QUERY);
//                  Reference< XInterface> xIface(xForm, UNO_QUERY);
//                  if (xForm.is() && searchElement(xCont, xIface))
//                  {
//                      setCurForm(xForm);
//                  }
//                  else
//                  {
//                      if (pPage->GetImpl()->getCurForm().is())
//                          xForm = pPage->GetImpl()->getCurForm();
//                      else if (xCont.is() && xCont->getCount())
//                          xForm = pPage->GetImpl()->getDefaultForm();
//
//                      if (xForm.is())
//                          setCurForm(xForm);
//                  }
//              }
//
//              Reference< XInterface> xPreviousObject(getSelObject());
//
//              // wurde vorher Form angezeigt, dann wieder die Form anzeigen
//              Reference< XForm> xOldForm(xPreviousObject, UNO_QUERY);
//              if (xOldForm.is())
//                  setSelObject(m_xCurForm);
//              else
//                  setSelObject(m_xCurControl);
//
//              if (IsPropBrwOpen() && m_xSelObject != xPreviousObject)
//                  ShowProperties(m_xSelObject, sal_True);
//          }
        }
        m_aMark.Clear();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusGained( const FocusEvent& e ) throw (RuntimeException)
{
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside(TRUE);
        m_pView->RefreshAllIAOManagers();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusLost( const FocusEvent& e ) throw (RuntimeException)
{
    // when switch the focus outside the office the mark didn't change
    // so we can not remove us as focus listener
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside(FALSE);
        m_pView->RefreshAllIAOManagers();
    }
}
// -----------------------------------------------------------------------------
void FmXFormView::removeGridWindowListening()
{
    if ( m_xWindow.is() )
    {
        m_xWindow->removeFocusListener(this);
        if ( m_pView )
        {
            m_pView->SetMoveOutside(FALSE);
            m_pView->RefreshAllIAOManagers();
        }
        m_xWindow = NULL;
    }
}
// -----------------------------------------------------------------------------


