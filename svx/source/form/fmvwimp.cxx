/*************************************************************************
 *
 *  $RCSfile: fmvwimp.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hjs $ $Date: 2001-09-12 18:10:47 $
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

#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
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

//========================================================================
DBG_NAME(FmXPageViewWinRec);
//------------------------------------------------------------------------
FmXPageViewWinRec::FmXPageViewWinRec(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xORB,
                                     const SdrPageViewWinRec* pWinRec,
                                   FmXFormView* _pViewImpl)
                 :m_pViewImpl(_pViewImpl)
                 ,m_pWindow( (Window*)pWinRec->GetOutputDevice() )
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
            setController(xForm, pWinRec->GetControlContainerRef() );
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
    for (vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > >::const_iterator i = m_aControllerList.begin();
            i != m_aControllerList.end(); i++)
    {
        // detaching the events
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xChild((*i)->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xChild.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xIfc(*i, ::com::sun::star::uno::UNO_QUERY);
            xEventManager->detach( i - m_aControllerList.begin(), xIfc );
        }

        // dispose the formcontroller
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >  xComp(*i, ::com::sun::star::uno::UNO_QUERY);
        xComp->dispose();
    }
    m_aControllerList.clear(); // this call deletes the formcontrollers
    m_xORB = NULL;
}


//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXPageViewWinRec::hasElements(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return getCount() != 0;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Type SAL_CALL  FmXPageViewWinRec::getElementType(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>*)0);
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL FmXPageViewWinRec::createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return new ::comphelper::OEnumerationByIndex(this);
}

// ::com::sun::star::container::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FmXPageViewWinRec::getCount(void) throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aControllerList.size();
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXPageViewWinRec::getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException )
{
    if (nIndex < 0 ||
        nIndex >= getCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    ::com::sun::star::uno::Any aElement;
    aElement <<= m_aControllerList[nIndex];
    return aElement;
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  getControllerSearchChilds( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > & xIndex, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > & xModel)
{
    if (xIndex.is() && xIndex->getCount())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  xController;
        for (sal_Int32 n = xIndex->getCount(); n--; )
        {
            xIndex->getByIndex(n) >>= xController;
            if ((::com::sun::star::awt::XTabControllerModel*)xModel.get() == (::com::sun::star::awt::XTabControllerModel*)xController->getModel().get())
                return xController;
            else
                return getControllerSearchChilds(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (xController, ::com::sun::star::uno::UNO_QUERY), xModel);
        }
    }
    return ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > ();
}

// Search the according controller
//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  FmXPageViewWinRec::getController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xForm )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  xModel(xForm, ::com::sun::star::uno::UNO_QUERY);
    for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > >::const_iterator i = m_aControllerList.begin();
         i != m_aControllerList.end(); i++)
    {
        if ((::com::sun::star::awt::XTabControllerModel*)(*i)->getModel().get() == (::com::sun::star::awt::XTabControllerModel*)xModel.get())
            return *i;

        // the current-round controller isn't the right one. perhaps one of it's children ?
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  xChildSearch = getControllerSearchChilds(::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > (*i, ::com::sun::star::uno::UNO_QUERY), xModel);
        if (xChildSearch.is())
            return xChildSearch;
    }
    return ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController > ();
}

//------------------------------------------------------------------------
void FmXPageViewWinRec::setController(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > & xForm,
                                     const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & xCC,
                                     FmXFormController* pParent, sal_Bool)
{
    DBG_ASSERT(xForm.is(), "Kein Formular angegeben");
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xFormCps(xForm, ::com::sun::star::uno::UNO_QUERY);
    if (!xFormCps.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  xTabOrder(xForm, ::com::sun::star::uno::UNO_QUERY);

    FmFormPage* pCurrentPage = PTR_CAST(FmFormPage, m_pViewImpl->getView()->GetPageViewPvNum(0)->GetPage());
    DBG_ASSERT(pCurrentPage, "FmXFormController::queryDispatch : could not retrieve my page !");
    String sPageId = pCurrentPage ? pCurrentPage->GetImpl()->GetPageId() : String::CreateFromAscii("no page");
        // this "no page" should result in a empty ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  provided by the controller's external dispatcher

    // Anlegen des Tabcontrollers
    FmXFormController* pController = new FmXFormController(m_xORB,m_pViewImpl->getView(), m_pWindow, sPageId);
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  xController(pController);

    pController->setModel(xTabOrder);
    pController->setContainer(xCC);
    pController->activateTabOrder();
    pController->addActivateListener(m_pViewImpl);

    if (pParent)
        pParent->addChild(pController);
    else
    {
        //  ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  xController(pController);
        m_aControllerList.push_back(xController);

        pController->setParent(*this);

        // attaching the events
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager >  xEventManager(xForm->getParent(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xIfc(xController, ::com::sun::star::uno::UNO_QUERY);
        xEventManager->attach(m_aControllerList.size() - 1, xIfc, ::com::sun::star::uno::makeAny(xController) );
    }



    // jetzt die Subforms durchgehen
    sal_uInt32 nLength = xFormCps->getCount();
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xSubForm;
    for (sal_uInt32 i = 0; i < nLength; i++)
    {
        xFormCps->getByIndex(i) >>= xSubForm;
        if (xSubForm.is())
            setController(xSubForm, xCC, pController);
    }
}

//------------------------------------------------------------------------
void FmXPageViewWinRec::updateTabOrder( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & xControl,
                                       const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & xCC )
{
    // Das TabControllerModel der ::com::sun::star::form ermitteln, in der das Control
    // enthalten ist ...
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComp(xControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
    if (!xFormComp.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm(xFormComp->getParent(), ::com::sun::star::uno::UNO_QUERY);
    if (!xForm.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >  xTabCtrl(getController( xForm ), ::com::sun::star::uno::UNO_QUERY);
    // Wenn es fuer dieses Formular noch keinen Tabcontroller gibt,
    // dann einen neuen anlegen
    if (!xTabCtrl.is())
    {
        // ist es ein Unterformular?
        // dann muss ein Tabcontroller fuer den Parent existieren
        // wichtig da ein hierarchischer Aufbau vorliegt
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xParentForm(::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > (xForm, ::com::sun::star::uno::UNO_QUERY)->getParent(), ::com::sun::star::uno::UNO_QUERY);
        FmXFormController* pFormController = NULL;
        // zugehoerigen controller suchen
        if (xParentForm.is())
            xTabCtrl = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >(getController(xParentForm), ::com::sun::star::uno::UNO_QUERY);

        if (xTabCtrl.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(xTabCtrl,::com::sun::star::uno::UNO_QUERY);
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
::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  FmXPageViewWinRec::getControlContainer() const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  xCC;
    if (m_aControllerList.size())
        xCC = m_aControllerList[0]->getContainer();
    return xCC;
}

//------------------------------------------------------------------------
FmXFormView::~FmXFormView()
{
    DBG_ASSERT(m_aWinList.size() == 0, "Liste nicht leer");
    if (m_nEvent)
        Application::RemoveUserEvent(m_nEvent);
    if (m_nErrorMessageEvent)
        Application::RemoveUserEvent(m_nErrorMessageEvent);
    if (m_nAutoFocusEvent)
        Application::RemoveUserEvent(m_nAutoFocusEvent);
}

//      EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException )
{
}

// ::com::sun::star::form::XFormControllerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formActivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >  xController(rEvent.Source, ::com::sun::star::uno::UNO_QUERY);
    // benachrichtigung der Shell
    if (m_pView && m_pView->GetFormShell())
    {
        FmXFormShell* pShImpl =  m_pView->GetFormShell()->GetImpl();
        if (pShImpl)
            pShImpl->setActiveController(xController);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formDeactivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    // deaktivierung wird nicht registriert
}

// ::com::sun::star::container::XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementInserted(const ::com::sun::star::container::ContainerEvent& evt) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  xCC(evt.Source, ::com::sun::star::uno::UNO_QUERY);
    if( xCC.is() )
    {
        FmWinRecList::iterator i = findWindow( xCC );

        if ( i != m_aWinList.end() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  xControl;
            evt.Element >>= xControl;
            if( xControl.is() )
                (*i)->updateTabOrder( xControl, xCC );
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt) throw( ::com::sun::star::uno::RuntimeException )
{
    elementInserted(evt);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt) throw( ::com::sun::star::uno::RuntimeException )
{
}

//------------------------------------------------------------------------------
FmWinRecList::const_iterator FmXFormView::findWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & rCC )  const
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
FmWinRecList::iterator FmXFormView::findWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & rCC )
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
void FmXFormView::addWindow(const SdrPageViewWinRec* pRec)
{
    // Wird gerufen, wenn
    // - in den aktiven Modus geschaltet wird
    // - ein Window hinzugefuegt wird, waehrend man im Design-Modus ist
    // - der Control-Container fuer ein Window angelegt wird, waehrend
    //   der aktive Modus eingeschaltet ist.

    // Wenn es noch keinen Control-Container gibt oder am Control-Container
    // noch keine Peer Erzeugt wurde, dann ist es noch zu frueh, um die
    // Tab-Order einzustellen ...
    if( pRec->GetOutputDevice()->GetOutDevType() == OUTDEV_WINDOW  )
    {
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & rCC = pRec->GetControlContainerRef();
        if ( rCC.is() && findWindow( rCC ) == m_aWinList.end())
        {
            FmXPageViewWinRec *pFmRec = new FmXPageViewWinRec(m_xORB,pRec, this);
            pFmRec->acquire();

            m_aWinList.push_back(pFmRec);

            // Am ControlContainer horchen um Aenderungen mitzbekommen
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(rCC, ::com::sun::star::uno::UNO_QUERY);
            if (xContainer.is())
                xContainer->addContainerListener(this);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormView::removeWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & rCC )
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
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >  xContainer(rCC, ::com::sun::star::uno::UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener(this);

        (*i)->dispose();
        (*i)->release();
        m_aWinList.erase(i);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnDelayedErrorMessage, void*, EMPTYTAG)
{
    m_nErrorMessageEvent = 0;
    Window* pParentWindow = Application::GetDefDialogParent();
    ErrorBox(pParentWindow, WB_OK, m_sErrorMessage).Execute();
//  ErrorBox(Application::GetDefDialogParent(), WB_OK, m_sErrorMessage).Execute();
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnActivate, void*, EMPTYTAG)
{
    m_nEvent = 0;

    if (m_pPageViewForActivation)
    {
        FmFormModel* pModel = PTR_CAST(FmFormModel, m_pView->GetModel());
        if (pModel)
            // lock the undo env so the forms can change non-transient properties while loading
                // (without this my doc's modified flag would be set)
            pModel->GetUndoEnv().Lock();

        // Load all forms
        FmFormPage* pPage = static_cast<FmFormPage*>(m_pPageViewForActivation->GetPage());
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xForms(pPage->GetForms(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Any aElement;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >  xForm;
        for (sal_Int32 i = 0, nCount = xForms->getCount(); i < nCount; i++)
        {
            xForms->getByIndex(i) >>= xForm;
            // a database form must be loaded for
            if (::isLoadable(xForm) && !xForm->isLoaded())
                xForm->load();
        }

        if (pModel && pModel->GetAutoControlFocus())
            LINK(this, FmXFormView, OnAutoFocus).Call(NULL);

        if (pModel)
            // unlock the environment
            pModel->GetUndoEnv().UnLock();
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
                    if (xForm.is() && getRowsetConnection(xForm).is())
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
void FmXFormView::Activate(SdrPageView* pPageView, sal_Bool bSync)
{
    if (m_nEvent)
    {
        Application::RemoveUserEvent(m_nEvent);
        m_nEvent = 0;
    }

    m_pPageViewForActivation = pPageView;
    if (bSync)
    {
        LINK(this,FmXFormView,OnActivate).Call(NULL);
    }
    else
        m_nEvent = Application::PostUserEvent(LINK(this,FmXFormView,OnActivate));
}

//------------------------------------------------------------------------------
void FmXFormView::Deactivate(SdrPageView* pPageView, BOOL bDeactivateController)
{
    if (m_nEvent)
    {
        Application::RemoveUserEvent(m_nEvent);
        m_nEvent = 0;
        m_pPageViewForActivation = NULL;
    }

    FmXFormShell* pShImpl =  m_pView->GetFormShell() ? m_pView->GetFormShell()->GetImpl() : NULL;
    if (pShImpl && bDeactivateController)
        pShImpl->setActiveController();
}

//------------------------------------------------------------------------------
void FmXFormView::AttachControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > & rControl, sal_Bool bDetach )
{
    /* wird im fmctrler gemacht */
}

//------------------------------------------------------------------------------
void FmXFormView::AttachControls( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & rCtrlContainer,
                                  sal_Bool bDetach )
{
}

//------------------------------------------------------------------------------
void FmXFormView::smartControlReset( const Reference< XIndexAccess >& _rxModels )
{
    if (!_rxModels.is())
    {
        DBG_ERROR("FmXFormView::smartControlReset: invalid container!");
        return;
    }

    static const ::rtl::OUString sClassIdPropertyName = FM_PROP_CLASSID;
    static const ::rtl::OUString sBoundFieldPropertyName = FM_PROP_BOUNDFIELD;
    sal_Int32 nCount = _rxModels->getCount();
    Reference< XPropertySet > xCurrent;
    Reference< XPropertySetInfo > xCurrentInfo;
    Reference< XPropertySet > xBoundField;

    for (sal_Int32 i=0; i<nCount; ++i)
    {
        _rxModels->getByIndex(i) >>= xCurrent;
        if (xCurrent.is())
            xCurrentInfo = xCurrent->getPropertySetInfo();
        else
            xCurrentInfo.clear();
        if (!xCurrentInfo.is())
            continue;

        if (xCurrentInfo->hasPropertyByName(sClassIdPropertyName))
        {   // it's a control model

            // check if this control is bound to a living database field
            if (xCurrentInfo->hasPropertyByName(sBoundFieldPropertyName))
                xCurrent->getPropertyValue(sBoundFieldPropertyName) >>= xBoundField;
            else
                xBoundField.clear();

            if (!xBoundField.is())
            {   // no, not valid bound -> reset it
                Reference< XReset > xControlReset(xCurrent, UNO_QUERY);
                if (xControlReset.is())
                    xControlReset->reset();
            }
        }
        else
        {
            Reference< XIndexAccess > xContainer(xCurrent, UNO_QUERY);
            if (xContainer.is())
                smartControlReset(xContainer);
        }
    }
}

//------------------------------------------------------------------------------
FmFormShell* FmXFormView::GetFormShell() const
{
    return m_pView ? m_pView->GetFormShell() : NULL;
}
// -----------------------------------------------------------------------------
void FmXFormView::AutoFocus()
{
    if (m_nAutoFocusEvent)
          Application::RemoveUserEvent(m_nAutoFocusEvent);
    m_nAutoFocusEvent = Application::PostUserEvent(LINK(this, FmXFormView, OnAutoFocus));
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
            ::com::sun::star::uno::Reference< XForm > xForm;
            if (nObjects)
                ::cppu::extractInterface(xForm, xForms->getByIndex(0));

            Reference< XTabController > xTabControllerModel(pViewWinRec->getController( xForm ), UNO_QUERY);
            // go for the first control of the controller
            Sequence< Reference< XControl > > aControls;
            if (xTabControllerModel.is())
                aControls = xTabControllerModel->getControls();
            Reference< XControl > xFirstControl;
            if (aControls.getLength())
                xFirstControl = aControls[0];

            // set the focus to this first control
            Reference< XWindow > xControlWindow(xFirstControl, UNO_QUERY);
            if (xControlWindow.is())
                xControlWindow->setFocus();

            // ensure that the control is visible
            // 80210 - 12/07/00 - FS
            if (xControlWindow.is() && m_pView->GetActualOutDev() && (OUTDEV_WINDOW == m_pView->GetActualOutDev()->GetOutDevType()))
            {
                const Window* pWindow = static_cast<const Window*>(m_pView->GetActualOutDev());
                ::com::sun::star::awt::Rectangle aRect = xControlWindow->getPosSize();
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
SdrObject* FmXFormView::implCreateFieldControl( const ::svx::ODataAccessDescriptor& _rColumnDescriptor )
{
    // not if we're in design mode
    if (!m_pView->IsDesignMode())
        return NULL;

    // the very basic information
    ::rtl::OUString sDataSource, sCommand, sFieldName;
    sal_Int32 nCommandType = CommandType::COMMAND;

    _rColumnDescriptor[ daDataSource ]  >>= sDataSource;
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
        xDataSource = getDatasourceObject(sDataSource, getORB());
        // and the connection, if necessary
        if ( !bForeignConnection )
            xConnection = getDatasourceConnection(sDataSource, getORB());
    }
    catch(const SQLContext& e) { aError.Reason <<= e; }
    catch(const SQLWarning& e) { aError.Reason <<= e; }
    catch(const SQLException& e) { aError.Reason <<= e; }
    if (aError.Reason.hasValue())
    {
        displayException(aError);
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


    Reference< XPreparedStatement >     xStatement;
    // go
    try
    {
        FmFormPage& rPage = *static_cast<FmFormPage*>(m_pView->GetPageViewPvNum(0)->GetPage());
        // Festellen des Feldes
        Reference< XNameAccess >    xFields;
        Reference< XPropertySet >       _rxField;
        switch (nCommandType)
        {
            case 0: // old : DataSelectionType_TABLE:
            {
                Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
                Reference< XColumnsSupplier >  xSupplyColumns;
                xSupplyTables->getTables()->getByName(sCommand) >>= xSupplyColumns;
                xFields = xSupplyColumns->getColumns();
            }
            break;
            case 1: // old : DataSelectionType_QUERY:
            {
                Reference< XQueriesSupplier >  xSupplyQueries(xConnection, UNO_QUERY);
                Reference< XColumnsSupplier >  xSupplyColumns;
                xSupplyQueries->getQueries()->getByName(sCommand) >>= xSupplyColumns;
                xFields  = xSupplyColumns->getColumns();
            }
            break;
            default:
            {
                xStatement = xConnection->prepareStatement(sCommand);
                // not interested in any results
                Reference< XPropertySet > (xStatement,UNO_QUERY)->setPropertyValue(::rtl::OUString::createFromAscii("MaxRows"),makeAny(sal_Int32(0)));
                Reference< XColumnsSupplier >  xSupplyCols(xStatement->executeQuery(), UNO_QUERY);
                if (xSupplyCols.is())
                    xFields = xSupplyCols->getColumns();
            }
        }

        if (xFields.is() && xFields->hasByName(sFieldName))
            xFields->getByName(sFieldName) >>= _rxField;

        Reference< XNumberFormatsSupplier >  xSupplier = OStaticDataAccessTools().getNumberFormats(xConnection, sal_False);
        if (!xSupplier.is() || !_rxField.is())
            return NULL;

        Reference< XNumberFormats >  xNumberFormats(xSupplier->getNumberFormats());
        if (!xNumberFormats.is())
            return NULL;

        // Vom Feld werden nun zwei Informationen benoetigt:
        // a.) Name des Feldes fuer Label und ControlSource
        // b.) FormatKey, um festzustellen, welches Feld erzeugt werden soll
        sal_Int32 nDataType = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FIELDTYPE));
        sal_Int32 nFormatKey = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FORMATKEY));

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
                const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                for( sal_uInt16 i = 0; i < rWinList.GetCount(); i++ )
                {
                    if( rWinList[i].GetOutputDevice()->GetOutDevType() == OUTDEV_WINDOW)
                    {
                        _pOutDev = rWinList[i].GetOutputDevice();
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
        if (::comphelper::hasProperty(FM_PROP_ISCURRENCY, _rxField))
            bIsCurrency = ::comphelper::getBOOL(_rxField->getPropertyValue(FM_PROP_ISCURRENCY));

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
        createControlLabelPair(_pOutDev, 0, _rxField, xNumberFormats, nOBJID, sLabelPostfix, pLabel, pControl);
        if (!pLabel || !pControl)
        {
            delete pLabel;
            delete pControl;
            return NULL;
        }

        //////////////////////////////////////////////////////////////////////
        // Feststellen ob eine ::com::sun::star::form erzeugt werden muss
        // Dieses erledigt die Page fuer uns bzw. die PageImpl
        Reference< XFormComponent >  xContent(pLabel->GetUnoControlModel(), UNO_QUERY);
        Reference< XIndexContainer >  xContainer(rPage.GetImpl()->SetDefaults(xContent, xDataSource, sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));

        xContent = Reference< XFormComponent > (pControl->GetUnoControlModel(), UNO_QUERY);
        xContainer = Reference< XIndexContainer > (rPage.GetImpl()->SetDefaults(xContent, xDataSource,
            sDataSource, sCommand, nCommandType), UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), makeAny(xContent));

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
            createControlLabelPair(_pOutDev, 1000, _rxField, xNumberFormats, OBJ_FM_TIMEFIELD,
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
        ::comphelper::disposeComponent(xStatement);
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
    Reference< ::com::sun::star::beans::XPropertySet >  xLabelSet(_rpLabel->GetUnoControlModel(), UNO_QUERY);
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
    aRealSize.Width() = max(aRealSize.Width(), aDefTxtSize.Width()) + aDelta.Width();
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
    Reference< ::com::sun::star::beans::XPropertySet >  xControlSet = Reference< ::com::sun::star::beans::XPropertySet > (_rpControl->GetUnoControlModel(), UNO_QUERY);
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

