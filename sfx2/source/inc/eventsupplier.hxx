/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventsupplier.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:00:42 $
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

#ifndef _SFX_EVENTSUPPLIER_HXX_
#define _SFX_EVENTSUPPLIER_HXX_

#ifndef  _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef  _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef  _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif

#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef  _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif

#ifndef  _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef  _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASVECTOR_HXX_
#include <comphelper/sequenceasvector.hxx>
#endif

#ifndef  _SFX_SFXUNO_HXX
#include <sfxuno.hxx>
#endif

#include <vos/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <svtools/lstner.hxx>
#include <svtools/eventcfg.hxx>

//--------------------------------------------------------------------------------------------------------

#define NOSUCHELEMENTEXCEPTION      ::com::sun::star::container::NoSuchElementException
#define XNAMEREPLACE                ::com::sun::star::container::XNameReplace
#define DOCEVENTOBJECT              ::com::sun::star::document::EventObject
#define XEVENTBROADCASTER           ::com::sun::star::document::XEventBroadcaster
#define XDOCEVENTLISTENER           ::com::sun::star::document::XEventListener
#define XEVENTSSUPPLIER             ::com::sun::star::document::XEventsSupplier
#define XJOBEXECUTOR                ::com::sun::star::task::XJobExecutor
#define EVENTOBJECT                 ::com::sun::star::lang::EventObject
#define ILLEGALARGUMENTEXCEPTION    ::com::sun::star::lang::IllegalArgumentException
#define WRAPPEDTARGETEXCEPTION      ::com::sun::star::lang::WrappedTargetException
#define ANY                         ::com::sun::star::uno::Any
#define REFERENCE                   ::com::sun::star::uno::Reference
#define WEAKREFERENCE               ::com::sun::star::uno::WeakReference
#define RUNTIMEEXCEPTION            ::com::sun::star::uno::RuntimeException
#define SEQUENCE                    ::com::sun::star::uno::Sequence
#define UNOTYPE                     ::com::sun::star::uno::Type
#define OUSTRING                    ::rtl::OUString
#define OINTERFACECONTAINERHELPER   ::cppu::OInterfaceContainerHelper

//--------------------------------------------------------------------------------------------------------

class SfxObjectShell;
class SfxBaseModel;
class SvxMacro;

//--------------------------------------------------------------------------------------------------------

class SfxEvents_Impl : public ::cppu::WeakImplHelper2< ::com::sun::star::container::XNameReplace, ::com::sun::star::document::XEventListener  >
{
    SEQUENCE< OUSTRING >            maEventNames;
    SEQUENCE< ANY >                 maEventData;
    REFERENCE< XEVENTBROADCASTER >  mxBroadcaster;
    ::osl::Mutex                    maMutex;
    SfxObjectShell                 *mpObjShell;

    sal_Bool                    Warn_Impl( const String& );

public:
                                SfxEvents_Impl( SfxObjectShell* pShell,
                                                REFERENCE< XEVENTBROADCASTER > xBroadcaster );
                               ~SfxEvents_Impl();

    //  --- XNameReplace ---
    virtual void SAL_CALL       replaceByName( const OUSTRING & aName, const ANY & aElement )
                                    throw( ILLEGALARGUMENTEXCEPTION, NOSUCHELEMENTEXCEPTION,
                                           WRAPPEDTARGETEXCEPTION, RUNTIMEEXCEPTION );

    //  --- XNameAccess ( parent of XNameReplace ) ---
    virtual ANY SAL_CALL        getByName( const OUSTRING& aName )
                                    throw( NOSUCHELEMENTEXCEPTION, WRAPPEDTARGETEXCEPTION,
                                           RUNTIMEEXCEPTION );
    virtual SEQUENCE< OUSTRING > SAL_CALL getElementNames() throw ( RUNTIMEEXCEPTION );
    virtual sal_Bool SAL_CALL   hasByName( const OUSTRING& aName ) throw ( RUNTIMEEXCEPTION );

    //  --- XElementAccess ( parent of XNameAccess ) ---
    virtual UNOTYPE SAL_CALL    getElementType() throw ( RUNTIMEEXCEPTION );
    virtual sal_Bool SAL_CALL   hasElements() throw ( RUNTIMEEXCEPTION );

    // --- ::document::XEventListener ---
    virtual void SAL_CALL       notifyEvent( const DOCEVENTOBJECT& aEvent )
                                    throw( RUNTIMEEXCEPTION );

    // --- ::lang::XEventListener ---
    virtual void SAL_CALL       disposing( const EVENTOBJECT& Source )
                                    throw( RUNTIMEEXCEPTION );

    static SvxMacro*            ConvertToMacro( const ANY& rElement, SfxObjectShell* pDoc, BOOL bBlowUp );
    static void                 BlowUpMacro( const ANY& rIn, ANY& rOut, SfxObjectShell* pDoc );
};

//=============================================================================
struct ModelCollectionMutexBase
{
    public:
        ::osl::Mutex m_aLock;
};

//=============================================================================
typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > > TModelList;

//=============================================================================
class ModelCollectionEnumeration : public ModelCollectionMutexBase
                                 , public ::cppu::WeakImplHelper1< ::com::sun::star::container::XEnumeration >
{

    //-------------------------------------------------------------------------
    // member
    //-------------------------------------------------------------------------
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xSMGR;
        TModelList m_lModels;
        TModelList::iterator m_pEnumerationIt;

    //-------------------------------------------------------------------------
    // native interface
    //-------------------------------------------------------------------------
    public:
        ModelCollectionEnumeration(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMGR);
        virtual ~ModelCollectionEnumeration();
        void setModelList(const TModelList& rList);

    //-------------------------------------------------------------------------
    // uno interface
    //-------------------------------------------------------------------------
    public:

        // css.container.XEnumeration
        virtual sal_Bool SAL_CALL hasMoreElements()
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
            throw(::com::sun::star::container::NoSuchElementException,
                  ::com::sun::star::lang::WrappedTargetException     ,
                  ::com::sun::star::uno::RuntimeException            );
};

//=============================================================================
class SfxGlobalEvents_Impl : public ModelCollectionMutexBase
                           , public ::cppu::WeakImplHelper5< ::com::sun::star::lang::XServiceInfo
                                                           , ::com::sun::star::document::XEventsSupplier
                                                           , ::com::sun::star::document::XEventBroadcaster
                                                           , ::com::sun::star::document::XEventListener
                                                           , ::com::sun::star::container::XSet >
                           , public SfxListener
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xSMGR;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > m_xEvents;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::task::XJobExecutor > m_xJobsBinding;
    OINTERFACECONTAINERHELPER m_aInterfaceContainer;
    TModelList m_lModels;
    GlobalEventConfig* pImp;

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint );

public:
    SfxGlobalEvents_Impl(const com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xSMGR);
    virtual ~SfxGlobalEvents_Impl();

    SFX_DECL_XSERVICEINFO

    // css.document.XEventBroadcaster
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents()
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& xListener)
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& xListener)
        throw(::com::sun::star::uno::RuntimeException);

    // css.document.XEventListener
    virtual void SAL_CALL notifyEvent(const ::com::sun::star::document::EventObject& aEvent)
        throw(::com::sun::star::uno::RuntimeException);

    // css.container.XSet
    virtual sal_Bool SAL_CALL has(const ::com::sun::star::uno::Any& aElement)
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL insert(const ::com::sun::star::uno::Any& aElement)
        throw(::com::sun::star::lang::IllegalArgumentException  ,
              ::com::sun::star::container::ElementExistException,
              ::com::sun::star::uno::RuntimeException           );

    virtual void SAL_CALL remove(const ::com::sun::star::uno::Any& aElement)
        throw(::com::sun::star::lang::IllegalArgumentException   ,
              ::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException            );

    // css.container.XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration()
        throw(::com::sun::star::uno::RuntimeException);

    // css.container.XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException);

    // css.lang.XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& aEvent)
        throw(::com::sun::star::uno::RuntimeException);

private:

    // threadsafe
    void implts_notifyJobExecution(const ::com::sun::star::document::EventObject& aEvent);
    void implts_checkAndExecuteEventBindings(const ::com::sun::star::document::EventObject& aEvent);
    void implts_notifyListener(const ::com::sun::star::document::EventObject& aEvent);

    // not threadsafe
    TModelList::iterator impl_searchDoc(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel);
};

#endif
