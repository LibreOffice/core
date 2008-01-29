/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmvwimp.cxx,v $
 *
 *  $Revision: 1.66 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 17:10:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_FMCTRLER_HXX
#include <fmctrler.hxx>
#endif

#ifndef _SVX_FMVWIMP_HXX
#include "fmvwimp.hxx"
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX //autogen
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX
#include "fmcontrollayout.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
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
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XBINDABLEVALUE_HPP_
#include <com/sun/star/form/binding/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XVALUEBINDING_HPP_
#include <com/sun/star/form/binding/XValueBinding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_SUBMISSION_XSUBMISSIONSUPPLIER_HPP_
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
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
/** === end UNO includes === **/

#ifndef _SVX_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include <svx/fmpage.hxx>
#endif
#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
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
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_XMLEXCHG_HXX_
#include "xmlexchg.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
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
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif
#ifndef _SDRPAINTWINDOW_HXX
#include "sdrpaintwindow.hxx"
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
using namespace ::com::sun::star::task;
using namespace ::comphelper;
using namespace ::svxform;
using namespace ::svx;
using com::sun::star::style::VerticalAlignment_MIDDLE;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::form::binding::XBindableValue;

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
DBG_NAME(FmXPageViewWinRec)
//------------------------------------------------------------------------
FmXPageViewWinRec::FmXPageViewWinRec(const Reference< XMultiServiceFactory >& _rxORB, const SdrPageWindow& _rWindow, FmXFormView* _pViewImpl )
:   m_xControlContainer( _rWindow.GetControlContainer() ),
    m_xORB( _rxORB ),
    m_pViewImpl( _pViewImpl ),
    m_pWindow( dynamic_cast< Window* >( &_rWindow.GetPaintWindow().GetOutputDevice() ) )
{
    DBG_CTOR(FmXPageViewWinRec,NULL);

    // create an XFormController for every form
    FmFormPage* pFormPage = PTR_CAST( FmFormPage, _rWindow.GetPageView().GetPage() );
    DBG_ASSERT( pFormPage, "FmXPageViewWinRec::FmXPageViewWinRec: no FmFormPage found!" );
    if ( pFormPage )
    {
        Reference< XIndexAccess > xForms( pFormPage->GetForms(), UNO_QUERY );
        sal_uInt32 nLength = xForms->getCount();
        Any aElement;
        Reference< XForm >  xForm;
        for (sal_uInt32 i = 0; i < nLength; i++)
        {
            if ( xForms->getByIndex(i) >>= xForm )
                setController( xForm );
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
    for (::std::vector< Reference< XFormController > >::const_iterator i = m_aControllerList.begin();
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
Reference< XFormController >  FmXPageViewWinRec::getController( const Reference< XForm > & xForm ) const
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
void FmXPageViewWinRec::setController(const Reference< XForm > & xForm,  FmXFormController* _pParent )
{
    DBG_ASSERT( xForm.is(), "FmXPageViewWinRec::setController: there should be a form!" );
    Reference< XIndexAccess >  xFormCps(xForm, UNO_QUERY);
    if (!xFormCps.is())
        return;

    Reference< XTabControllerModel >  xTabOrder(xForm, UNO_QUERY);

    // create a form controller
    FmXFormController* pController = new FmXFormController( m_xORB,m_pViewImpl->getView(), m_pWindow );
    Reference< XFormController > xController( pController );

    Reference< XInteractionHandler > xHandler;
    if ( _pParent )
        xHandler = _pParent->getInteractionHandler();
    else
        // TODO: should we create a default handler? Not really necessary, since the
        // FormController itself has a default fallback
        ;
    if ( xHandler.is() )
    {
        Reference< XInitialization > xInitController( xController, UNO_QUERY );
        DBG_ASSERT( xInitController.is(), "FmXPageViewWinRec::setController: can't initialize the controller!" );
        if ( xInitController.is() )
        {
            Sequence< Any > aInitArgs( 1 );
            aInitArgs[ 0 ] <<= xHandler;
            xInitController->initialize( aInitArgs );
        }
    }

    pController->setModel(xTabOrder);
    pController->setContainer( m_xControlContainer );
    pController->activateTabOrder();
    pController->addActivateListener(m_pViewImpl);

    if ( _pParent )
        _pParent->addChild(pController);
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
        if ( xFormCps->getByIndex(i) >>= xSubForm )
            setController(xSubForm, pController);
    }
}

//------------------------------------------------------------------------
void FmXPageViewWinRec::updateTabOrder( const Reference< XControl > & xControl,
                                       const Reference< XControlContainer >& /*_rxCC*/ )
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
        setController( xForm, pFormController );
    }
}

//------------------------------------------------------------------------
FmXFormView::FmXFormView(const Reference< XMultiServiceFactory >&   _xORB,
            FmFormView* _pView)
    :m_xORB( _xORB )
    ,m_pMarkedGrid(NULL)
    ,m_pView(_pView)
    ,m_nActivationEvent(0)
    ,m_nErrorMessageEvent( 0 )
    ,m_nAutoFocusEvent( 0 )
    ,m_pWatchStoredList( NULL )
    ,m_bFirstActivation( sal_True )
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
    DBG_ASSERT(m_aWinList.size() == 0, "FmXFormView::~FmXFormView: Window list not empty!");

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
    Reference< XControlContainer > xCC( evt.Source, UNO_QUERY );
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
void SAL_CALL FmXFormView::elementRemoved(const ContainerEvent& /*evt*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
FmWinRecList::const_iterator FmXFormView::findWindow( const Reference< XControlContainer >& _rxCC )  const
{
    for (FmWinRecList::const_iterator i = m_aWinList.begin();
            i != m_aWinList.end(); i++)
    {
        if ( _rxCC == (*i)->getControlContainer() )
            return i;
    }
    return m_aWinList.end();
}

//------------------------------------------------------------------------------
FmWinRecList::iterator FmXFormView::findWindow( const Reference< XControlContainer >& _rxCC )
{
    for (FmWinRecList::iterator i = m_aWinList.begin();
            i != m_aWinList.end(); i++)
    {
        if ( _rxCC == (*i)->getControlContainer() )
            return i;
    }
    return m_aWinList.end();
}

//------------------------------------------------------------------------------
void FmXFormView::addWindow(const SdrPageWindow& rWindow)
{
    FmFormPage* pFormPage = PTR_CAST( FmFormPage, rWindow.GetPageView().GetPage() );
    if ( !pFormPage )
        return;

    Reference< XControlContainer > xCC = rWindow.GetControlContainer();
    if ( xCC.is() && findWindow( xCC ) == m_aWinList.end())
    {
        FmXPageViewWinRec *pFmRec = new FmXPageViewWinRec(m_xORB, rWindow, this);
        pFmRec->acquire();

        m_aWinList.push_back(pFmRec);

        // Am ControlContainer horchen um Aenderungen mitzbekommen
        Reference< XContainer >  xContainer( xCC, UNO_QUERY );
        if (xContainer.is())
            xContainer->addContainerListener(this);
    }
}

//------------------------------------------------------------------------------
void FmXFormView::removeWindow( const Reference< XControlContainer >& _rxCC )
{
    // Wird gerufen, wenn
    // - in den Design-Modus geschaltet wird
    // - ein Window geloescht wird, waehrend man im Design-Modus ist
    // - der Control-Container fuer ein Window entfernt wird, waehrend
    //   der aktive Modus eingeschaltet ist.
    FmWinRecList::iterator i = findWindow( _rxCC );
    if (i != m_aWinList.end())
    {
        // Am ControlContainer horchen um Aenderungen mitzbekommen
        Reference< XContainer >  xContainer( _rxCC, UNO_QUERY );
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
IMPL_LINK(FmXFormView, OnDelayedErrorMessage, void*, /*EMPTYTAG*/)
{
    m_nErrorMessageEvent = 0;
    displayException( m_aAsyncError );
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormView::onFirstViewActivation( const FmFormModel* _pDocModel )
{
    if ( _pDocModel && _pDocModel->GetAutoControlFocus() )
        m_nAutoFocusEvent = Application::PostUserEvent( LINK( this, FmXFormView, OnAutoFocus ) );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnActivate, void*, /*EMPTYTAG*/)
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
            for (::std::vector< Reference< XFormController > >::const_iterator i = pFmRec->GetList().begin();
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
            (void)e;    // make compiler happy
        }

        if ( !xReturn.is() && _rControls.getLength() )
            xReturn = _rControls[0];
    }

    return xReturn;
}

// -----------------------------------------------------------------------------
namespace
{
    // .........................................................................
    void lcl_ensureControlsOfFormExist_nothrow( const SdrPage& _rPage, const SdrView& _rView, const Window& _rWindow, const Reference< XForm >& _rxForm )
    {
        try
        {
            Reference< XInterface > xNormalizedForm( _rxForm, UNO_QUERY_THROW );

            SdrObjListIter aSdrObjectLoop( _rPage, IM_DEEPNOGROUPS );
            while ( aSdrObjectLoop.IsMore() )
            {
                const SdrUnoObj* pUnoObject = dynamic_cast< const SdrUnoObj* >( aSdrObjectLoop.Next() );
                if ( !pUnoObject )
                    continue;

                Reference< XChild > xModel( pUnoObject->GetUnoControlModel(), UNO_QUERY_THROW );
                Reference< XInterface > xModelParent( xModel->getParent(), UNO_QUERY_THROW );

                if ( xNormalizedForm.get() != xModelParent.get() )
                    continue;

                pUnoObject->GetUnoControl( _rView, _rWindow );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

// -----------------------------------------------------------------------------
Reference< XFormController > FmXFormView::getFormController( const Reference< XForm >& _rxForm, const OutputDevice& _rDevice ) const
{
    Reference< XFormController > xController;

    for ( FmWinRecList::const_iterator rec = m_aWinList.begin(); rec != m_aWinList.end(); ++rec )
    {
        const FmXPageViewWinRec* pViewWinRec( *rec );
        OSL_ENSURE( pViewWinRec, "FmXFormView::getFormController: invalid PageViewWinRec!" );
        if ( !pViewWinRec || ( pViewWinRec->getWindow() != &_rDevice ) )
            // wrong device
            continue;

        xController = pViewWinRec->getController( _rxForm );
        if ( xController.is() )
            break;
    }
    return xController;
}

// -----------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnAutoFocus, void*, /*EMPTYTAG*/)
{
    m_nAutoFocusEvent = 0;

    // go to the first form of our page, examine it's TabController, go to it's first (in terms of the tab order)
    // control, give it the focus

    do
    {

    // get the forms collection of the page we belong to
    FmFormPage* pPage = m_pView ? PTR_CAST( FmFormPage, m_pView->GetSdrPageView()->GetPage() ) : NULL;
    Reference< XIndexAccess > xForms( pPage ? Reference< XIndexAccess >( pPage->GetForms(), UNO_QUERY ) : Reference< XIndexAccess >() );

    const FmXPageViewWinRec* pViewWinRec = m_aWinList.size() ? m_aWinList[0] : NULL;
    const Window* pWindow = pViewWinRec ? pViewWinRec->getWindow() : NULL;

    OSL_ENSURE( xForms.is() && pWindow, "FmXFormView::OnAutoFocus: could not collect all essentials!" );
    if ( !xForms.is() || !pWindow )
        return 0L;

    try
    {
        // go for the tab controller of the first form
        if ( !xForms->getCount() )
            break;
        Reference< XForm > xForm( xForms->getByIndex( 0 ), UNO_QUERY_THROW );
        Reference< XTabController > xTabController( pViewWinRec->getController( xForm ), UNO_QUERY_THROW );

        // go for the first control of the controller
        Sequence< Reference< XControl > > aControls( xTabController->getControls() );
        if ( aControls.getLength() == 0 )
        {
            Reference< XElementAccess > xFormElementAccess( xForm, UNO_QUERY_THROW );
            if ( xFormElementAccess->hasElements() )
            {
                // there are control models in the form, but no controls, yet.
                // Well, since some time controls are created on demand only. In particular,
                // they're normally created when they're first painted.
                // Unfortunately, the FormController does not have any way to
                // trigger the creation itself, so we must hack this ...
                lcl_ensureControlsOfFormExist_nothrow( *pPage, *m_pView, *pWindow, xForm );
                aControls = xTabController->getControls();
                OSL_ENSURE( aControls.getLength(), "FmXFormView::OnAutoFocus: no controls at all!" );
            }
        }

        // set the focus to this first control
        Reference< XWindow > xControlWindow( lcl_firstFocussableControl( aControls ), UNO_QUERY );
        if ( !xControlWindow.is() )
            break;

        xControlWindow->setFocus();

        // ensure that the control is visible
        // 80210 - 12/07/00 - FS
        const Window* pCurrentWindow = dynamic_cast< const Window* >( m_pView->GetActualOutDev() );
        if ( pCurrentWindow )
        {
            awt::Rectangle aRect = xControlWindow->getPosSize();
            ::Rectangle aNonUnoRect( aRect.X, aRect.Y, aRect.X + aRect.Width, aRect.Y + aRect.Height );
            m_pView->MakeVisible( pCurrentWindow->PixelToLogic( aNonUnoRect ), *const_cast< Window* >( pCurrentWindow ) );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    }   // do
    while ( false );

    return 1L;
}

// -----------------------------------------------------------------------------
namespace
{
    /*
        ATTENTION!
        Broken for solaris? It seems that the old used template argument TYPE was already
        defined as a macro ... which expand to ... "TYPE "!?
        All platforms are OK - excepting Solaris. There the line "template< class TYPE >"
        was expanded to "template < class TYPE " where the closing ">" was missing.
    */
    #ifdef MYTYPE
        #error "Who defines the macro MYTYPE, which is used as template argument here?"
    #endif

    //....................................................................
    template< class MYTYPE >
    Reference< MYTYPE > getTypedModelNode( const Reference< XInterface >& _rxModelNode )
    {
        Reference< MYTYPE > xTypedNode( _rxModelNode, UNO_QUERY );
        if ( xTypedNode.is() )
            return xTypedNode;
        else
        {
            Reference< XChild > xChild( _rxModelNode, UNO_QUERY );
            if ( xChild.is() )
                return getTypedModelNode< MYTYPE >( xChild->getParent() );
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
        DocumentType eDocumentType = GetFormShell() ? GetFormShell()->GetImpl()->getDocumentType() : eUnknownDocumentType;
        ControlLayouter::initializeControlLayout( _rxModel, eDocumentType );

        _rxModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;
        Reference< XPropertySetInfo> xPSI(_rxModel->getPropertySetInfo());
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
                    if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_MULTILINE ) )
                        _rxModel->setPropertyValue( FM_PROP_MULTILINE, makeAny( (sal_Bool)sal_True ) );
                }
            }
            break;

            case FormComponentType::RADIOBUTTON:
            case FormComponentType::CHECKBOX:
            case FormComponentType::FIXEDTEXT:
            {
                ::rtl::OUString sVertAlignPropertyName( RTL_CONSTASCII_USTRINGPARAM( "VerticalAlign" ) );
                if ( xPSI.is() && xPSI->hasPropertyByName( sVertAlignPropertyName ) )
                    _rxModel->setPropertyValue( sVertAlignPropertyName, makeAny( VerticalAlignment_MIDDLE ) );
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

    ::rtl::OUString sCommand, sFieldName;
    sal_Int32 nCommandType = CommandType::COMMAND;
    SharedConnection xConnection;

    ::rtl::OUString sDataSource = _rColumnDescriptor.getDataSource();
    _rColumnDescriptor[ daCommand ]     >>= sCommand;
    _rColumnDescriptor[ daColumnName ]  >>= sFieldName;
    _rColumnDescriptor[ daCommandType ] >>= nCommandType;
    {
        Reference< XConnection > xExternalConnection;
        _rColumnDescriptor[ daConnection ]  >>= xExternalConnection;
        xConnection.reset( xExternalConnection, SharedConnection::NoTakeOwnership );
    }

    if  (   !sCommand.getLength()
        ||  !sFieldName.getLength()
        ||  (   !sDataSource.getLength()
            &&  !xConnection.is()
            )
        )
    {
        DBG_ERROR( "FmXFormView::implCreateFieldControl: nonsense!" );
    }

    Reference< XDataSource > xDataSource;
    SQLErrorEvent aError;
    try
    {
        if ( xConnection.is() && !xDataSource.is() && !sDataSource.getLength() )
        {
            Reference< XChild > xChild( xConnection, UNO_QUERY );
            if ( xChild.is() )
                xDataSource = xDataSource.query( xChild->getParent() );
        }

        // obtain the data source
        if ( !xDataSource.is() )
            xDataSource = OStaticDataAccessTools().getDataSource(sDataSource, getORB());

        // and the connection, if necessary
        if ( !xConnection.is() )
            xConnection.reset( OStaticDataAccessTools().getConnection_withFeedback(
                sDataSource,
                ::rtl::OUString(),
                ::rtl::OUString(),
                getORB()
            ) );
    }
    catch(const SQLContext& e) { aError.Reason <<= e; }
    catch(const SQLWarning& e) { aError.Reason <<= e; }
    catch(const SQLException& e) { aError.Reason <<= e; }
    catch( const Exception& ) { /* will be asserted below */ }
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

    OStaticDataAccessTools aDBATools;
    Reference< XComponent > xKeepFieldsAlive;
    // go
    try
    {
        FmFormPage& rPage = *static_cast<FmFormPage*>(m_pView->GetSdrPageView()->GetPage());

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

        ::rtl::OUString sLabelPostfix;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* _pOutDev = NULL;
        if (m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
            _pOutDev = const_cast<OutputDevice*>(m_pView->GetActualOutDev());
        else
        {// OutDev suchen
            SdrPageView* pPageView = m_pView->GetSdrPageView();
            if( pPageView && !_pOutDev )
            {
                // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

                for( sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); i++ )
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);

                    if( rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        _pOutDev = &rPageWindow.GetPaintWindow().GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if (!_pOutDev)
            return NULL;

        sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));
        if ((DataType::BINARY == nDataType) || (DataType::VARBINARY == nDataType))
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // determine the control type by examining the data type of the bound column
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
                case DataType::BOOLEAN:
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
                    sLabelPostfix = String( SVX_RES( RID_STR_POSTFIX_DATE ) );
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

        SdrUnoObj* pLabel;
        SdrUnoObj* pControl;
        createControlLabelPair(m_pView,_pOutDev, 0,0, xField, xNumberFormats, nOBJID, sLabelPostfix, FmFormInventor,OBJ_FM_FIXEDTEXT,NULL,NULL,NULL,pLabel, pControl);
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
        Reference< XIndexContainer >  xContainer(rPage.GetImpl()->placeInFormComponentHierarchy(xContent, xDataSource, sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
        implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pLabel );

        xContent = Reference< XFormComponent > (pControl->GetUnoControlModel(), UNO_QUERY);
        xContainer = Reference< XIndexContainer > (rPage.GetImpl()->placeInFormComponentHierarchy(xContent, xDataSource,
            sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
        implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pLabel );

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
            createControlLabelPair(m_pView,_pOutDev, 0,1000, xField, xNumberFormats, OBJ_FM_TIMEFIELD,
                String( SVX_RES( RID_STR_POSTFIX_TIME ) ), FmFormInventor,OBJ_FM_FIXEDTEXT,
                NULL,NULL,NULL,
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

// -----------------------------------------------------------------------------
SdrObject* FmXFormView::implCreateXFormsControl( const ::svx::OXFormsDescriptor &_rDesc )
{
    // not if we're in design mode
    if ( !m_pView->IsDesignMode() )
        return NULL;

    Reference< XComponent > xKeepFieldsAlive;

    // go
    try
    {
        FmFormPage& rPage = *static_cast<FmFormPage*>(m_pView->GetSdrPageView()->GetPage());

        // determine the table/query field which we should create a control for
        Reference< XPropertySet >   xField;
        Reference< XNumberFormats > xNumberFormats;
        ::rtl::OUString sLabelPostfix = _rDesc.szName;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* _pOutDev = NULL;
        if (m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
            _pOutDev = const_cast<OutputDevice*>(m_pView->GetActualOutDev());
        else
        {// OutDev suchen
            SdrPageView* pPageView = m_pView->GetSdrPageView();
            if( pPageView && !_pOutDev )
            {
                // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

                for( sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); i++ )
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);

                    if( rPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType() == OUTDEV_WINDOW)
                    {
                        _pOutDev = &rPageWindow.GetPaintWindow().GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if (!_pOutDev)
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // The service name decides which control should be created
        sal_uInt16 nOBJID = OBJ_FM_EDIT;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_SUN_COMPONENT_NUMERICFIELD))
            nOBJID = OBJ_FM_NUMERICFIELD;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_SUN_COMPONENT_CHECKBOX))
            nOBJID = OBJ_FM_CHECKBOX;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_COMPONENT_COMMANDBUTTON))
            nOBJID = OBJ_FM_BUTTON;

        typedef ::com::sun::star::form::submission::XSubmission XSubmission_t;
        Reference< XSubmission_t > xSubmission(_rDesc.xPropSet, UNO_QUERY);

        // xform control or submission button?
        if ( !xSubmission.is() )
        {

            SdrUnoObj* pLabel;
            SdrUnoObj* pControl;
            createControlLabelPair(m_pView,_pOutDev, 0,0, xField, xNumberFormats, nOBJID, sLabelPostfix, FmFormInventor,OBJ_FM_FIXEDTEXT,NULL,NULL,NULL,pLabel, pControl);
            if (!pLabel || !pControl) {
                delete pLabel;
                delete pControl;
                return NULL;
            }

            //////////////////////////////////////////////////////////////////////
            // Now build the connection between the control and the data item.
            Reference< XValueBinding > xValueBinding(_rDesc.xPropSet,UNO_QUERY);
            Reference< XBindableValue > xBindableValue(pControl->GetUnoControlModel(),UNO_QUERY);

            DBG_ASSERT( xBindableValue.is(), "FmXFormView::implCreateXFormsControl: control's not bindable!" );
            if ( xBindableValue.is() )
                xBindableValue->setValueBinding(xValueBinding);

            //////////////////////////////////////////////////////////////////////
            // Feststellen ob eine ::com::sun::star::form erzeugt werden muss
            // Dieses erledigt die Page fuer uns bzw. die PageImpl
            Reference< XFormComponent >  xContent(pLabel->GetUnoControlModel(), UNO_QUERY);
            Reference< XIndexContainer >  xContainer(rPage.GetImpl()->placeInFormComponentHierarchy( xContent ), UNO_QUERY);
            if (xContainer.is())
                xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
            implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pControl );

            xContent = Reference< XFormComponent > (pControl->GetUnoControlModel(), UNO_QUERY);
            xContainer = Reference< XIndexContainer > (rPage.GetImpl()->placeInFormComponentHierarchy( xContent ), UNO_QUERY);
            if (xContainer.is())
                xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));
            implInitializeNewControlModel( Reference< XPropertySet >( xContent, UNO_QUERY ), pControl );

            //////////////////////////////////////////////////////////////////////
            // Objekte gruppieren
            SdrObjGroup* pGroup  = new SdrObjGroup();
            SdrObjList* pObjList = pGroup->GetSubList();
            pObjList->InsertObject(pLabel);
            pObjList->InsertObject(pControl);

            return pGroup;
        }
        else {

            // create a button control
            const MapMode eTargetMode(_pOutDev->GetMapMode());
            const MapMode eSourceMode(MAP_100TH_MM);
            const sal_uInt16 nObjID = OBJ_FM_BUTTON;
            ::Size controlSize(4000, 500);
            FmFormObj *pControl = static_cast<FmFormObj*>(SdrObjFactory::MakeNewObject( FmFormInventor, nObjID, NULL, NULL ));
            controlSize.Width() = Fraction(controlSize.Width(), 1) * eTargetMode.GetScaleX();
            controlSize.Height() = Fraction(controlSize.Height(), 1) * eTargetMode.GetScaleY();
            ::Point controlPos(_pOutDev->LogicToLogic(Point(controlSize.Width(),0),eSourceMode,eTargetMode));
            ::Rectangle controlRect(controlPos,_pOutDev->LogicToLogic(controlSize, eSourceMode, eTargetMode));
            pControl->SetLogicRect(controlRect);

            // set the button label
            Reference< XPropertySet > xControlSet(pControl->GetUnoControlModel(), UNO_QUERY);
            xControlSet->setPropertyValue(FM_PROP_LABEL, makeAny(::rtl::OUString(_rDesc.szName)));

            // connect the submission with the submission supplier (aka the button)
            xControlSet->setPropertyValue( FM_PROP_BUTTON_TYPE,
                                           makeAny( FormButtonType_SUBMIT ) );
            typedef ::com::sun::star::form::submission::XSubmissionSupplier XSubmissionSupplier_t;
            Reference< XSubmissionSupplier_t > xSubmissionSupplier(pControl->GetUnoControlModel(), UNO_QUERY);
            xSubmissionSupplier->setSubmission(xSubmission);

            return pControl;
        }
    }
    catch(const Exception&)
    {
        DBG_ERROR("FmXFormView::implCreateXFormsControl: caught an exception while creating the control !");
    }


    return NULL;
}

//------------------------------------------------------------------------
void FmXFormView::createControlLabelPair(SdrView* /*_pView*/,OutputDevice* _pOutDev, sal_Int32 _nXOffsetMM, sal_Int32 _nYOffsetMM,
    const Reference< XPropertySet >& _rxField, const Reference< XNumberFormats >& _rxNumberFormats,
    sal_uInt16 _nObjID, const ::rtl::OUString& _rFieldPostfix,UINT32 _nInventor,UINT16 _nIndent
    ,SdrPage* _pLabelPage,SdrPage* _pPage,SdrModel* _pModel,
    SdrUnoObj*& _rpLabel, SdrUnoObj*& _rpControl)
{
    sal_Int32 nDataType = 0;
    sal_Int32 nFormatKey = 0;
    ::rtl::OUString sFieldName;
    Any aFieldName;
    if ( _rxField.is() )
    {
        nDataType = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FIELDTYPE));
        Reference< XPropertySetInfo > xPSI( _rxField->getPropertySetInfo() );
        if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_FORMATKEY ) )
            nFormatKey = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FORMATKEY));
        else
            nFormatKey = OStaticDataAccessTools().getDefaultNumberFormat(
                _rxField,
                Reference< XNumberFormatTypes >( _rxNumberFormats, UNO_QUERY ),
                SvtSysLocale().GetLocaleData().getLocale()
            );

        aFieldName = Any(_rxField->getPropertyValue(FM_PROP_NAME));
        aFieldName >>= sFieldName;
    }

    // das Label
    _rpLabel = static_cast<SdrUnoObj*>(SdrObjFactory::MakeNewObject( _nInventor, _nIndent, _pLabelPage,_pModel ));
    Reference< XPropertySet > xLabelSet(_rpLabel->GetUnoControlModel(), UNO_QUERY);
    xLabelSet->setPropertyValue(FM_PROP_LABEL, makeAny(sFieldName + _rFieldPostfix));

    // positionieren unter Beachtung der Einstellungen des Ziel-Output-Devices
    ::Size aTextSize(_pOutDev->GetTextWidth(sFieldName + _rFieldPostfix), _pOutDev->GetTextHeight());

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
    aRealSize.Width() = long(Fraction(aRealSize.Width(), 1) * eTargetMode.GetScaleX());
    aRealSize.Height() = long(Fraction(aRealSize.Height(), 1) * eTargetMode.GetScaleY());
    _rpLabel->SetLogicRect(
        ::Rectangle(    _pOutDev->LogicToLogic(Point(_nXOffsetMM, _nYOffsetMM), eSourceMode, eTargetMode),
                    _pOutDev->LogicToLogic(aRealSize, eSourceMode, eTargetMode)
        ));

    // jetzt das Control
    _rpControl = static_cast<SdrUnoObj*>(SdrObjFactory::MakeNewObject( _nInventor, _nObjID, _pPage,_pModel ));
    Reference< XPropertySet > xControlSet( _rpControl->GetUnoControlModel(), UNO_QUERY );

    // positionieren
    ::Size szControlSize;
    if (DataType::BIT == nDataType || nDataType == DataType::BOOLEAN )
        szControlSize = aDefSize;
    else if (OBJ_FM_IMAGECONTROL == _nObjID || DataType::LONGVARCHAR == nDataType || DataType::LONGVARBINARY == nDataType )
        szControlSize = aDefImageSize;
    else
        szControlSize = aDefSize;

    // normieren wie oben
    szControlSize.Width() = long(Fraction(szControlSize.Width(), 1) * eTargetMode.GetScaleX());
    szControlSize.Height() = long(Fraction(szControlSize.Height(), 1) * eTargetMode.GetScaleY());
    _rpControl->SetLogicRect(
        ::Rectangle(    _pOutDev->LogicToLogic(Point(aRealSize.Width() + _nXOffsetMM, _nYOffsetMM), eSourceMode, eTargetMode),
                    _pOutDev->LogicToLogic(szControlSize, eSourceMode, eTargetMode)
        ));

    // ein paar initiale Einstellungen am ControlModel
    if (xControlSet.is())
    {
        Reference< XPropertySetInfo > xControlPropInfo = xControlSet->getPropertySetInfo();
        // ein paar numersiche Eigenschaften durchschleifen
        if (xControlPropInfo->hasPropertyByName(FM_PROP_DECIMAL_ACCURACY))
        {
            // Number braucht eine Scale
            Any aScaleVal(::comphelper::getNumberFormatDecimals(_rxNumberFormats, nFormatKey));
            xControlSet->setPropertyValue(FM_PROP_DECIMAL_ACCURACY, aScaleVal);
        }
        if (xControlPropInfo->hasPropertyByName(FM_PROP_VALUEMIN) && xControlPropInfo->hasPropertyByName(FM_PROP_VALUEMAX))
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

        if (xControlPropInfo->hasPropertyByName(FM_PROP_STRICTFORMAT))
        {   // Formatueberpruefung fue numeric fields standardmaessig sal_True
            sal_Bool bB(sal_True);
            Any aVal(&bB,getBooleanCppuType());
            xControlSet->setPropertyValue(FM_PROP_STRICTFORMAT, aVal);
        }

        if ( aFieldName.hasValue() )
        {
            xControlSet->setPropertyValue(FM_PROP_CONTROLSOURCE, aFieldName);
            xControlSet->setPropertyValue(FM_PROP_NAME, aFieldName);
        }

        if (nDataType == DataType::LONGVARCHAR && xControlPropInfo->hasPropertyByName(FM_PROP_MULTILINE) )
        {
            sal_Bool bB(sal_True);
            xControlSet->setPropertyValue(FM_PROP_MULTILINE,Any(&bB,getBooleanCppuType()));
        }

        if (_nObjID == OBJ_FM_CHECKBOX)
        {
            sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
            if( _rxField.is() )
                _rxField->getPropertyValue( FM_PROP_ISNULLABLE ) >>= nNullable;
            xControlSet->setPropertyValue( FM_PROP_TRISTATE, makeAny( sal_Bool( ColumnValue::NULLABLE == nNullable ) ) );
        }

        // announce the label to the control
        if (xControlPropInfo->hasPropertyByName(FM_PROP_CONTROLLABEL))
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
}

//------------------------------------------------------------------------------
FmXFormView::ObjectRemoveListener::ObjectRemoveListener( FmXFormView* pParent )
    :m_pParent( pParent )
{
}

//------------------------------------------------------------------------------
void FmXFormView::ObjectRemoveListener::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
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

    ULONG nCount = m_aMark.GetMarkCount();
    for (ULONG i = 0; i < nCount; ++i)
    {
        SdrMark* pMark = m_aMark.GetMark(i);
        SdrObject* pCurrent = pMark->GetMarkedSdrObj();
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
            ULONG nCount = m_aMark.GetMarkCount( );
            for ( ULONG i = 0; i < nCount; ++i )
            {
                SdrMark*   pMark = m_aMark.GetMark(i);
                SdrObject* pObj  = pMark->GetMarkedSdrObj();

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
                            m_pView->MarkObj( pMark->GetMarkedSdrObj(), pMark->GetPageView(), sal_True /* unmark! */ );
                        }
                    }
                    else
                    {
                        if ( pObj->GetObjInventor() == FmFormInventor )
                        {   // this is a form layer object
                            m_pView->MarkObj( pMark->GetMarkedSdrObj(), pMark->GetPageView(), sal_True /* unmark! */ );
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
            ULONG nCurrentCount = rCurrentList.GetMarkCount();
            for ( ULONG i=0; i<nCurrentCount&& !bMisMatch; ++i )
            {
                const SdrObject* pCurrentMarked = rCurrentList.GetMark( i )->GetMarkedSdrObj();

                // loop through all saved marks, check for equality
                sal_Bool bFound = sal_False;
                ULONG nSavedCount = m_aMark.GetMarkCount();
                for ( ULONG j=0; j<nSavedCount && !bFound; ++j )
                {
                    if ( m_aMark.GetMark( j )->GetMarkedSdrObj() == pCurrentMarked )
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
        SdrPageView* pCurPageView = m_pView->GetSdrPageView();
        SdrObjListIter aPageIter( *pPage );
        sal_Bool bFound = sal_True;

        // gibt es noch alle Objecte
        ULONG nCount = m_aMark.GetMarkCount();
        for (ULONG i = 0; i < nCount && bFound; i++)
        {
            SdrMark*   pMark = m_aMark.GetMark(i);
            SdrObject* pObj  = pMark->GetMarkedSdrObj();
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
                for (ULONG i = 0; i < nCount; i++)
                {
                    SdrMark* pMark = m_aMark.GetMark(i);
                    SdrObject* pObj = pMark->GetMarkedSdrObj();
                    if ( pObj->GetObjInventor() == FmFormInventor )
                        if ( !m_pView->IsObjMarked( pObj ) )
                            m_pView->MarkObj( pObj, pMark->GetPageView() );
                }

                _rRestoredMarkList = m_aMark;
            }
        }
        m_aMark.Clear();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusGained( const FocusEvent& /*e*/ ) throw (RuntimeException)
{
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside(TRUE);
        //OLMm_pView->RefreshAllIAOManagers();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusLost( const FocusEvent& /*e*/ ) throw (RuntimeException)
{
    // when switch the focus outside the office the mark didn't change
    // so we can not remove us as focus listener
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside(FALSE);
        //OLMm_pView->RefreshAllIAOManagers();
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
            //OLMm_pView->RefreshAllIAOManagers();
        }
        m_xWindow = NULL;
    }
}
// -----------------------------------------------------------------------------


