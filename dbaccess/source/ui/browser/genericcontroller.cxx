/*************************************************************************
 *
 *  $RCSfile: genericcontroller.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-27 15:11:05 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace ::com::sun::star::uno;
//  using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::task;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;

::rtl::OUString URL_CONFIRM_DELETION(::rtl::OUString::createFromAscii(".uno:FormSlots/ConfirmDeletion"));

// -------------------------------------------------------------------------
OGenericUnoController::OGenericUnoController(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_xMultiServiceFacatory(_rM)
    ,m_aDisposeListeners(m_aPropertyMutex)
    ,m_bCurrentlyModified(sal_False)
    ,m_bFrameUiActive(sal_False)
    ,m_pView(NULL)
{
    try
    {
        m_xUrlTransformer = Reference< ::com::sun::star::util::XURLTransformer > (_rM->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::Construct(Window* pParent)
{
    OSL_ENSURE(m_pView,"the view is NULL!");
    // want to have a toolbox ?
    ToolBox* pTB = CreateToolBox(getView());
    getView()->setToolBox(pTB);
    if (pTB) // we want to handle the select
        pTB->SetSelectHdl(LINK(this, OGenericUnoController, OnToolBoxSelected));

    AddSupportedFeatures();

    // create the database context
    DBG_ASSERT(m_xMultiServiceFacatory.is(), "SbaTableQueryBrowser::Construct need a service factory!");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(m_xMultiServiceFacatory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::Construct: could not create (or start listening at) the database context!");
    }

    if (!m_xDatabaseContext.is())
    {       // at least notify the user. Though the whole component does not make any sense without the database context ...
        ShowServiceNotAvailableError(getView(), String(SERVICE_SDB_DATABASECONTEXT), sal_True);
    }
    return sal_True;
}
//------------------------------------------------------------------------------
IMPL_LINK(OGenericUnoController, OnAsyncInvalidateAll, void*, EMPTYARG)
{
    InvalidateFeature_Impl();
    return 0L;
}
// -----------------------------------------------------------------------------
VCLXWindow* OGenericUnoController::getWindowPeer()
{
    return getView()->GetWindowPeer();
}
// -------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow >     xParent;
    Reference< ::com::sun::star::frame::XFrame >    xFrame;

    PropertyValue aValue;
    const Any* pBegin   = aArguments.getConstArray();
    const Any* pEnd     = pBegin + aArguments.getLength();

    ::rtl::OUString aFrameName = ::rtl::OUString::createFromAscii("Frame");
    for(;pBegin != pEnd;++pBegin)
    {
        if((*pBegin >>= aValue) && aValue.Name == aFrameName)
        {
            if((aValue.Value >>= xFrame) && xFrame.is())
            {
                xParent = xFrame->getContainerWindow();
                VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xParent);
                Window* pParentWin = pParentComponent ? pParentComponent->GetWindow() : NULL;
                if (!pParentWin)
                {
                    throw Exception(::rtl::OUString::createFromAscii("Parent window is null"),*this);
                }

                if(xFrame.is() && Construct(pParentWin))
                {
                    //  xFrame->setComponent(getView()->GetWindowPeer(), this);
                    xFrame->setComponent(getWindowPeer(), this);
                    attachFrame(xFrame);
                    pParentComponent->setVisible(sal_True);
                }
                break; // no more needed here
            }
            else
            {
                OSL_ENSHURE(0,"OGenericUnoController::initialize: Frame is null!");
            }
        }
    }
}
// -------------------------------------------------------------------------
void OGenericUnoController::disposing(const EventObject& Source) throw( RuntimeException )
{
    // our frame ?
    Reference< ::com::sun::star::frame::XFrame >  xSourceFrame(Source.Source, UNO_QUERY);
    if (((::com::sun::star::frame::XFrame*)xSourceFrame.get() == (::com::sun::star::frame::XFrame*)m_xCurrentFrame.get()) && m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
}
//------------------------------------------------------------------------
void OGenericUnoController::modified(const EventObject& aEvent) throw( RuntimeException )
{
    m_bCurrentlyModified = sal_True;
    InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
    InvalidateFeature(ID_BROWSER_UNDO);
}
// -----------------------------------------------------------------------
void OGenericUnoController::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( RuntimeException )
{
    if (m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

    m_xCurrentFrame = xFrame;
    if (m_xCurrentFrame.is())
    {
        m_xCurrentFrame->addFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
        m_bFrameUiActive = m_xCurrentFrame->isActive();
    }
}
// -----------------------------------------------------------------------
sal_Bool OGenericUnoController::ImplInvalidateTBItem(sal_uInt16 nId, const FeatureState& rState)
{
    ToolBox* pTB = getView() ? getView()->getToolBox() : NULL;
    if (!pTB || (pTB->GetItemPos(nId) == TOOLBOX_ITEM_NOTFOUND))
        return sal_False;

    pTB->EnableItem(nId, rState.bEnabled);
    switch (rState.aState.getValueTypeClass())
    {
        case TypeClass_BOOLEAN:
            pTB->CheckItem(nId, ::comphelper::getBOOL(rState.aState));
            break;

        case TypeClass_STRING:
            if (pTB->GetItemWindow(nId))
                pTB->GetItemWindow(nId)->SetText(::comphelper::getString(rState.aState));
            break;

        case TypeClass_VOID:
            break;

        default:
            DBG_WARNING("OGenericUnoController::ImplInvalidateTBItem : don't know what to do with the item state !");
    }
    return sal_True;
}

// -----------------------------------------------------------------------
void OGenericUnoController::ImplBroadcastFeatureState(const ::rtl::OUString& _rFeature, const Reference< ::com::sun::star::frame::XStatusListener > & xListener, sal_Bool _bIgnoreCache)
{
    sal_Int32 nFeat = m_aSupportedFeatures[_rFeature];
    FeatureState aFeatState( GetState(nFeat));

    FeatureState& rCachedState = m_aStateCache[nFeat];  // creates if neccessary

    if(!_bIgnoreCache)
    {
        // check if we really need to notify the listeners : this method may be called much more often than needed, so check
        // the cached state of the feature
        sal_Bool bAlreadyCached = (m_aStateCache.find(nFeat) != m_aStateCache.end());
        if (bAlreadyCached && (rCachedState.bEnabled == aFeatState.bEnabled))
        {   // the enabled flag hasn't changed, maybe the state ?
            if (rCachedState.aState.getValueTypeClass() == aFeatState.aState.getValueTypeClass())
            {   // at least the type of the state hasn't
                sal_Bool bEqualValue = sal_False;
                switch (rCachedState.aState.getValueTypeClass())
                {
                    case TypeClass_VOID:
                        bEqualValue = !aFeatState.aState.hasValue();
                        break;
                    case TypeClass_BOOLEAN:
                        bEqualValue = ::comphelper::getBOOL(rCachedState.aState) == ::comphelper::getBOOL(aFeatState.aState);
                        break;
                    case TypeClass_SHORT:
                        bEqualValue = ::comphelper::getINT16(rCachedState.aState) == ::comphelper::getINT16(aFeatState.aState);
                        break;
                    case TypeClass_LONG:
                        bEqualValue = ::comphelper::getINT32(rCachedState.aState) == ::comphelper::getINT32(aFeatState.aState);
                        break;
                    case TypeClass_STRING:
                        bEqualValue = ::comphelper::getString(rCachedState.aState).equals(::comphelper::getString(aFeatState.aState));
                        break;
                    default:
                        DBG_ERROR("OGenericUnoController::ImplBroadcastFeatureState : unknown state type (not implemented yet) !");
                        break;
                }
                if (bEqualValue)
                    return;
            }
        }
    }
    rCachedState = aFeatState;

    ::com::sun::star::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = _rFeature;
    if (m_xUrlTransformer.is())
        m_xUrlTransformer->parseStrict(aEvent.FeatureURL);
    aEvent.Source       = (::com::sun::star::frame::XDispatch*)this;
    aEvent.IsEnabled    = aFeatState.bEnabled;
    aEvent.Requery      = aFeatState.bRequery;
    aEvent.State        = aFeatState.aState;

    // a special listener ?
    if (xListener.is())
        xListener->statusChanged(aEvent);
    else
    {   // no -> iterate through all listeners responsible for the ::com::sun::star::util::URL
        DispatchIterator iterSearch = m_arrStatusListener.begin();
        DispatchIterator iterEnd = m_arrStatusListener.end();

        while (iterSearch != iterEnd)
        {
            DispatchTarget& rCurrent = *iterSearch;
            if (rCurrent.aURL.Complete.equals(_rFeature))
                rCurrent.xListener->statusChanged(aEvent);
            ++iterSearch;
        }
    }

    // give the TB a chance
    ImplInvalidateTBItem(nFeat, aFeatState);
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(const ::rtl::OUString& _rURLPath, const Reference< ::com::sun::star::frame::XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    FeaturePair aPair;
    aPair.nId               = m_aSupportedFeatures[_rURLPath];
    aPair.xListener         = _xListener;
    aPair.bForceBroadcast   = _bForceBroadcast;

    sal_Bool bWasEmpty = sal_False;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back(aPair);
    }
    if(bWasEmpty)
    {
        m_aAsyncInvalidateAll.Call();
        return;
    }

    //  ImplBroadcastFeatureState(rURLPath, xListener, _bForceBroadcast);
}
// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature_Impl()
{
#ifdef DBG_UTIL
    static sal_Int32 s_nRecursions = 0;
    ++s_nRecursions;
#endif

    sal_Bool bEmpty = sal_True;
    FeaturePair aNextFeature;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }
    while(!bEmpty)
    {
        if(aNextFeature.nId == -1)
        {
            InvalidateAll_Impl();
            break;
        }
        else
        {
            sal_Bool bFound = sal_False;
            SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            for(;aIter != m_aSupportedFeatures.end();++aIter)
            {
                if(aIter->second == aNextFeature.nId)
                {
                    ImplBroadcastFeatureState(aIter->first,aNextFeature.xListener, aNextFeature.bForceBroadcast);
                    bFound = sal_True;
                    break;
                }
            }
            if(!bFound)
                ImplInvalidateTBItem(aNextFeature.nId, GetState(aNextFeature.nId));
        }

        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aFeaturesToInvalidate.pop_front();
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }

#ifdef DBG_UTIL
    --s_nRecursions;
#endif
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(sal_uInt16 _nId, const Reference< ::com::sun::star::frame::XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    FeaturePair aPair;
    aPair.nId               = _nId;
    aPair.xListener         = _xListener;
    aPair.bForceBroadcast   = _bForceBroadcast;

    sal_Bool bWasEmpty = sal_False;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back(aPair);
    }
    if(bWasEmpty)
    {
        m_aAsyncInvalidateAll.Call();
        return;
    }
//  sal_Bool bFound = sal_False;
//  SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
//  for(;aIter != m_aSupportedFeatures.end();++aIter)
//  {
//      if(aIter->second == _nId)
//      {
//          ImplBroadcastFeatureState(aIter->first,_xListener, _bForceBroadcast);
//          bFound = sal_True;
//          break;
//      }
//  }
//  if(!bFound)
//      ImplInvalidateTBItem(_nId, GetState(_nId));
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateAll()
{
    FeaturePair aPair;
    aPair.nId               = -1;
    aPair.xListener         = NULL;
    aPair.bForceBroadcast   = sal_True;
    sal_Bool bWasEmpty = sal_False;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back(aPair);
    }
    if(bWasEmpty)
    {
        m_aAsyncInvalidateAll.Call();
        return;
    }
}
// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateAll_Impl()
{
    sal_uInt16 i;
    // ---------------------------------
    // invalidate all aupported features
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
    for(;aIter != m_aSupportedFeatures.end();++aIter)
    {
        ImplBroadcastFeatureState(aIter->first, Reference< ::com::sun::star::frame::XStatusListener > (), sal_True);
    }

    // ------------------------------
    // invalidate all slots in the TB (if any)
    ToolBox* pTB = getView() ? getView()->getToolBox() : NULL;
    if (pTB)
    {
        for (i=0; i<pTB->GetItemCount(); ++i)
            if (pTB->GetItemId(i))
                InvalidateFeature(pTB->GetItemId(i));
    }

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        DBG_ASSERT(m_aFeaturesToInvalidate.size(), "OGenericUnoController::InvalidateAll_Impl: to be called from within InvalidateFeature_Impl only!");
        m_aFeaturesToInvalidate.pop_front();
        if(!m_aFeaturesToInvalidate.empty())
            m_aAsyncInvalidateAll.Call();
    }
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatch >  OGenericUnoController::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    if (aURL.Complete.equals(URL_CONFIRM_DELETION))
        return (::com::sun::star::frame::XDispatch*)this;

    if (m_aSupportedFeatures.find(aURL.Complete) != m_aSupportedFeatures.end())
        return (::com::sun::star::frame::XDispatch*)this;

    if (m_xSlaveDispatcher.is())
        return m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return Reference< ::com::sun::star::frame::XDispatch > ();
}

// -----------------------------------------------------------------------
Sequence< Reference< ::com::sun::star::frame::XDispatch > > OGenericUnoController::queryDispatches(const Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw( RuntimeException )
{
    Sequence< Reference< ::com::sun::star::frame::XDispatch > > aReturn;
    if (!aDescripts.getLength())
        return aReturn;

    aReturn.realloc(aDescripts.getLength());
    Reference< ::com::sun::star::frame::XDispatch > * pReturn = aReturn.getArray();

    const ::com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_uInt16 i=0; i<aDescripts.getLength(); ++i, ++pDescripts, ++pReturn)
        *pReturn = queryDispatch(pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags);

    return aReturn;
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider >  OGenericUnoController::getSlaveDispatchProvider(void) throw( RuntimeException )
{
    return m_xSlaveDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setSlaveDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xSlaveDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider >  OGenericUnoController::getMasterDispatchProvider(void) throw( RuntimeException )
{
    return m_xMasterDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setMasterDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xMasterDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
void OGenericUnoController::dispatch(const ::com::sun::star::util::URL& aURL, const Sequence< PropertyValue >& aArgs)
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(aURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
        Execute(aIter->second);
}

// -----------------------------------------------------------------------
void OGenericUnoController::addStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL)
{
    // remeber the listener together with the ::com::sun::star::util::URL
    m_arrStatusListener.insert(m_arrStatusListener.end(), DispatchTarget(aURL, aListener));

    // initially broadcast the state
    InvalidateFeature(aURL.Complete, aListener, sal_True);
        // force the new state to be broadcasted to the new listener
}

// -----------------------------------------------------------------------
void OGenericUnoController::removeStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL)
{
    DispatchIterator iterSearch = m_arrStatusListener.begin();
    DispatchIterator iterEnd    = m_arrStatusListener.end();

    sal_Bool bRemoveForAll = (aURL.Complete.getLength() == 0);
    while (iterSearch != iterEnd)
    {
        DispatchTarget& rCurrent = *iterSearch;
        if  (   (rCurrent.xListener == aListener)
            &&  (   bRemoveForAll
                ||  (rCurrent.aURL.Complete.equals(aURL.Complete))
                )
            )
        {
            if (!bRemoveForAll)
            {   // remove the listener only for the given ::com::sun::star::util::URL, so we can exit the loop after deletion
                m_arrStatusListener.erase(iterSearch);
                break;
            }
            else
            {   // we have to remove the listener for all URLs, so a simple erase isn't sufficient (as the iterator may be invalid then)
                sal_Int32 nOffset = iterSearch - m_arrStatusListener.begin();
                m_arrStatusListener.erase(iterSearch);
                iterSearch  = m_arrStatusListener.begin() + nOffset;
                iterEnd     = m_arrStatusListener.end();
            }
        }
        else
            ++iterSearch;
    }

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(aURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
    {   // clear the cache for that feature
        StateCacheIterator aCachePos = m_aStateCache.find(aIter->second);
        if (aCachePos != m_aStateCache.end())
            m_aStateCache.erase(aCachePos);
    }

    // now remove the listener from the deque
    ::osl::MutexGuard aGuard( m_aFeatureMutex);
    ::std::deque< FeaturePair >::iterator aFeatIter = m_aFeaturesToInvalidate.begin();
    for(;aFeatIter != m_aFeaturesToInvalidate.end();)
    {
        //  FeaturePair aPair = *aIter;
        if(aFeatIter->xListener == aListener)
            aFeatIter = m_aFeaturesToInvalidate.erase(aFeatIter);
        else
            ++aFeatIter;
    }
}

// -----------------------------------------------------------------------
void OGenericUnoController::dispose()
{
    // say our dispose listeners goodbye
    EventObject aEvt;
    aEvt.Source = (XWeak*)(::cppu::OWeakObject*)this;
    m_aDisposeListeners.disposeAndClear(aEvt);

    // our status listeners, too
    while (m_arrStatusListener.size() > 0)
    {
        DispatchIterator iterCurrent = m_arrStatusListener.begin();

        DispatchTarget& rCurrent = *iterCurrent;
        EventObject aDisposeEvent;
        aDisposeEvent.Source = (XComponent*)this;

#ifdef DBG_UTIL
        sal_Int32 nSize = m_arrStatusListener.size();
#endif
        rCurrent.xListener->disposing(aDisposeEvent);
        DBG_ASSERT(nSize > m_arrStatusListener.size(), "OGenericUnoController::dispose : the listener did not call removeStatusListener !");
            // in disposing the status listener should remove itself via removeStatusListener, therein we remove it from
            // m_arrStatusListener, so the size should have decreased.
    }

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aAsyncInvalidateAll.CancelCall();
        m_aFeaturesToInvalidate.clear();
    }

    // check out from all the objects we are listening
    // the frame
    if (m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
}

// -----------------------------------------------------------------------
void OGenericUnoController::addEventListener(const Reference< XEventListener > & aListener)
{
    m_aDisposeListeners.addInterface(aListener);
}

// -----------------------------------------------------------------------
void OGenericUnoController::removeEventListener(const Reference< XEventListener > & aListener)
{
    m_aDisposeListeners.removeInterface(aListener);
}

//------------------------------------------------------------------------------
void OGenericUnoController::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
}
//------------------------------------------------------------------------------
void OGenericUnoController::EmptyWindow()
{
    // dispatch en empty ::com::sun::star::util::URL so we will be cleaned up
    Reference< ::com::sun::star::frame::XDispatchProvider >  xProvider(m_xCurrentFrame, UNO_QUERY);
    if (xProvider.is())
    {
        Reference< ::com::sun::star::frame::XDispatch >  xDispatcher = xProvider->queryDispatch(::com::sun::star::util::URL(), m_xCurrentFrame->getName(), 0);
        if (xDispatcher.is())
            xDispatcher->dispatch(::com::sun::star::util::URL(), Sequence< PropertyValue >());
    }
}

//------------------------------------------------------------------------------
void OGenericUnoController::AddSupportedFeatures()
{
    // add all supported features
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/copyContent")] = ID_BROWSER_COPY;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/cutContent")] = ID_BROWSER_CUT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/pasteContent")] = ID_BROWSER_PASTE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/EditDoc")] = ID_BROWSER_EDITDOC;

//  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Redo")] = ID_BROWSER_REDO;
//  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Save")] = ID_BROWSER_SAVEDOC;
//  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Undo")] = ID_BROWSER_UNDO;
}

//------------------------------------------------------------------------------
FeatureState OGenericUnoController::GetState(sal_uInt16 nId)
{
    FeatureState aReturn;
        // (disabled automatically)

    try
    {
        switch (nId)
        {
            case ID_BROWSER_REDO:
                aReturn.bEnabled = sal_False;   // simply forget it ;). no redo possible.
                break;

            case ID_BROWSER_UNDO:
            case ID_BROWSER_SAVEDOC:
                aReturn.bEnabled = sal_True;
            break;
        }
    }
    catch(Exception& e)
    {
#if DBG_UTIL
        String sMessage("OGenericUnoController::GetState(", RTL_TEXTENCODING_ASCII_US);
        sMessage += String::CreateFromInt32(nId);
        sMessage.AppendAscii(") : catched an exception ! message : ");
        sMessage += (const sal_Unicode*)e.Message;
        DBG_ERROR(ByteString(sMessage, gsl_getSystemTextEncoding()).GetBuffer());
#else
        e;  // make compiler happy
#endif
    }

    return aReturn;
}

//------------------------------------------------------------------------------
sal_Bool OGenericUnoController::SaveModified(sal_Bool bCommit)
{
    sal_Bool bResult = sal_True;
    InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
    InvalidateFeature(ID_BROWSER_UNDO);

    return bResult;
}
//------------------------------------------------------------------------------
IMPL_LINK(OGenericUnoController, OnToolBoxSelected, ToolBox*, pToolBox)
{
    Execute(pToolBox->GetCurItemId());
    return 0L;
}
//------------------------------------------------------------------
sal_uInt16 OGenericUnoController::SaveData(sal_Bool bUI, sal_Bool bForBrowsing)
{
    return (sal_uInt16)SaveModified();
}
// -------------------------------------------------------------------------
URL OGenericUnoController::getURLForId(sal_Int32 _nId) const
{
    URL aReturn;
    for (   ConstSupportedFeaturesIterator aLoop = m_aSupportedFeatures.begin();
            aLoop != m_aSupportedFeatures.end();
            ++aLoop
        )
        if (aLoop->second == _nId)
        {
            aReturn.Complete = aLoop->first;
            break;
        }

    if (m_xUrlTransformer.is())
        m_xUrlTransformer->parseStrict(aReturn);

    return aReturn;
}
//-------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoController::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pArray = aSupported.getConstArray();
    for (sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray)
        if (pArray->equals(ServiceName))
            return sal_True;
    return sal_False;
}
// -----------------------------------------------------------------------------
Reference<XConnection> OGenericUnoController::connect(const ::rtl::OUString& _rsDataSourceName)
{
    Reference<XConnection> xConnection;

    Any aValue;
    try
    {
        aValue = m_xDatabaseContext->getByName(_rsDataSourceName);
    }
    catch(Exception&)
    {
    }

    Reference<XPropertySet> xProp;
    aValue >>= xProp;
    if (!xProp.is())
    {
        DBG_ERROR("OGenericUnoController::connect: coult not retrieve the data source!");
        return xConnection;
    }

    ::rtl::OUString sPwd, sUser;
    sal_Bool bPwdReq = sal_False;
    try
    {
        xProp->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
        bPwdReq = cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED));
        xProp->getPropertyValue(PROPERTY_USER) >>= sUser;
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: error while retrieving data source properties!");
    }

    SQLExceptionInfo aInfo;
    try
    {
        if(bPwdReq && !sPwd.getLength())
        {   // password required, but empty -> connect using an interaction handler
            Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
            if (!xConnectionCompletion.is())
            {
                DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: missing an interface ... need an error message here!");
            }
            else
            {   // instantiate the default SDB interaction handler
                Reference< XInteractionHandler > xHandler(m_xMultiServiceFacatory->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
                if (!xHandler.is())
                {
                    ShowServiceNotAvailableError(getView(), String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
                        // TODO: a real parent!
                }
                else
                {
                    xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                }
            }
        }
        else
        {
            Reference<XDataSource> xDataSource(xProp,UNO_QUERY);
            xConnection = xDataSource->getConnection(sUser, sPwd);
        }
        // be notified when connection is in disposing
        Reference< XComponent >  xComponent(xConnection, UNO_QUERY);
        if (xComponent.is())
        {
            Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
            xComponent->addEventListener(xEvtL);
        }
    }
    catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
    catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
    catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
    catch(Exception&) { DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: could not connect - unknown exception!"); }

    showError(aInfo);

    return xConnection;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::showError(const SQLExceptionInfo& _rInfo)
{
    ::dbaui::showError(_rInfo,getView(),getORB());
}
// -----------------------------------------------------------------------------



