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

#include "fmscriptingenv.hxx"
#include "svx/fmmodel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <tools/diagnose_ex.h>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/implementationreference.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/app.hxx>
#include <basic/basmgr.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace svxform
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::script::XEventAttacherManager;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::script::XScriptListener;
    using ::com::sun::star::script::ScriptEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::reflection::InvocationTargetException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::container::XHierarchicalNameAccess;
    using ::com::sun::star::reflection::XInterfaceMethodTypeDescription;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::beans::XPropertySet;
    /** === end UNO using === **/

    class FormScriptingEnvironment;

    //====================================================================
    //= FormScriptListener
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   XScriptListener
                                    >   FormScriptListener_Base;

    /** implements the XScriptListener interface, is used by FormScriptingEnvironment
    */
    class FormScriptListener    :public FormScriptListener_Base
    {
    private:
        ::osl::Mutex m_aMutex;
        FormScriptingEnvironment *m_pScriptExecutor;

    public:
        FormScriptListener( FormScriptingEnvironment * pScriptExecutor );

        // XScriptListener
        virtual void SAL_CALL firing( const ScriptEvent& aEvent ) throw (RuntimeException);
        virtual Any SAL_CALL approveFiring( const ScriptEvent& aEvent ) throw (InvocationTargetException, RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

        // lifetime control
        void SAL_CALL dispose();

    protected:
        ~FormScriptListener();

    private:
        /** determines whether calling a given method at a given listener interface can be done asynchronously

            @param _rListenerType
                the name of the UNO type whose method is to be checked
            @param _rMethodName
                the name of the method at the interface determined by _rListenerType

            @return
                <TRUE/> if and only if the method is declared <code>oneway</code>, i.e. can be called asynchronously
        */
        bool    impl_allowAsynchronousCall_nothrow( const ::rtl::OUString& _rListenerType, const ::rtl::OUString& _rMethodName ) const;

        /** determines whether the instance is already disposed
        */
        bool    impl_isDisposed_nothrow() const { return !m_pScriptExecutor; }

        /** fires the given script event in a thread-safe manner

            This methods calls our script executor's doFireScriptEvent, with previously releasing the given mutex guard,
            but ensuring that our script executor is not deleted between this release and the actual call.

            @param _rGuard
                a clearable guard to our mutex. Must be the only active guard to our mutex.
            @param _rEvent
                the event to fire
            @param _pSyncronousResult
                a place to take a possible result of the script call.

            @precond
                m_pScriptExecutor is not <NULL/>.
        */
        void    impl_doFireScriptEvent_nothrow( ::osl::ClearableMutexGuard& _rGuard, const ScriptEvent& _rEvent, Any* _pSyncronousResult );

    private:
        DECL_LINK( OnAsyncScriptEvent, ScriptEvent* );
    };

    //====================================================================
    //= FormScriptingEnvironment
    //====================================================================
    class FormScriptingEnvironment : public IFormScriptingEnvironment
    {
    private:
        typedef ::comphelper::ImplementationReference< FormScriptListener, XScriptListener >    ListenerImplementation;

    private:
        ::osl::Mutex            m_aMutex;
        oslInterlockedCount     m_refCount;
        ListenerImplementation  m_pScriptListener;
        FmFormModel&            m_rFormModel;
        bool                    m_bDisposed;

    public:
        FormScriptingEnvironment( FmFormModel& _rModel );
        virtual ~FormScriptingEnvironment();

        // callback for FormScriptListener
        void doFireScriptEvent( const ScriptEvent& _rEvent, Any* _pSyncronousResult );

        // IFormScriptingEnvironment
        virtual void registerEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager );
        virtual void revokeEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager );
        virtual void dispose();

        // IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        void impl_registerOrRevoke_throw( const Reference< XEventAttacherManager >& _rxManager, bool _bRegister );

    private:
        FormScriptingEnvironment();                                                 // never implemented
        FormScriptingEnvironment( const FormScriptingEnvironment& );                // never implemented
        FormScriptingEnvironment& operator=( const FormScriptingEnvironment& );     // never implemented
    };

    //====================================================================
    //= FormScriptListener
    //====================================================================
    //--------------------------------------------------------------------
    FormScriptListener::FormScriptListener( FormScriptingEnvironment* pScriptExecutor )
        :m_pScriptExecutor( pScriptExecutor )
    {
    }

    //--------------------------------------------------------------------
    FormScriptListener::~FormScriptListener()
    {
    }

    //--------------------------------------------------------------------
    bool FormScriptListener::impl_allowAsynchronousCall_nothrow( const ::rtl::OUString& _rListenerType, const ::rtl::OUString& _rMethodName ) const
    {
        bool bAllowAsynchronousCall = false;
        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XHierarchicalNameAccess > xTypeDescriptions( aContext.getSingleton( "com.sun.star.reflection.theTypeDescriptionManager" ), UNO_QUERY_THROW );

            ::rtl::OUString sMethodDescription( _rListenerType );
            sMethodDescription += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "::" ));
            sMethodDescription += _rMethodName;

            Reference< XInterfaceMethodTypeDescription > xMethod( xTypeDescriptions->getByHierarchicalName( sMethodDescription ), UNO_QUERY_THROW );
            bAllowAsynchronousCall = xMethod->isOneway();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bAllowAsynchronousCall;
    }

    //--------------------------------------------------------------------
    void FormScriptListener::impl_doFireScriptEvent_nothrow( ::osl::ClearableMutexGuard& _rGuard, const ScriptEvent& _rEvent, Any* _pSyncronousResult )
    {
        OSL_PRECOND( m_pScriptExecutor, "FormScriptListener::impl_doFireScriptEvent_nothrow: this will crash!" );

        _rGuard.clear();
        m_pScriptExecutor->doFireScriptEvent( _rEvent, _pSyncronousResult );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormScriptListener::firing( const ScriptEvent& _rEvent ) throw (RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );
       static const ::rtl::OUString vbaInterOp( RTL_CONSTASCII_USTRINGPARAM("VBAInterop") );
       if ( _rEvent.ScriptType.equals(vbaInterOp) )
           return; // not handled here

        if ( impl_isDisposed_nothrow() )
            return;

        if ( !impl_allowAsynchronousCall_nothrow( _rEvent.ListenerType.getTypeName(), _rEvent.MethodName ) )
        {
            impl_doFireScriptEvent_nothrow( aGuard, _rEvent, NULL );
            return;
        }

        acquire();
        Application::PostUserEvent( LINK( this, FormScriptListener, OnAsyncScriptEvent ), new ScriptEvent( _rEvent ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL FormScriptListener::approveFiring( const ScriptEvent& _rEvent ) throw (InvocationTargetException, RuntimeException)
    {
        Any aResult;

        ::osl::ClearableMutexGuard aGuard( m_aMutex );
        if ( !impl_isDisposed_nothrow() )
            impl_doFireScriptEvent_nothrow( aGuard, _rEvent, &aResult );

        return aResult;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormScriptListener::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormScriptListener::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_pScriptExecutor = NULL;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( FormScriptListener, OnAsyncScriptEvent, ScriptEvent*, _pEvent )
    {
        OSL_PRECOND( _pEvent != NULL, "FormScriptListener::OnAsyncScriptEvent: invalid event!" );
        if ( !_pEvent )
            return 1L;

        {
            ::osl::ClearableMutexGuard aGuard( m_aMutex );

            if ( !impl_isDisposed_nothrow() )
                impl_doFireScriptEvent_nothrow( aGuard, *_pEvent, NULL );
        }

        delete _pEvent;
        // we acquired ourself immediately before posting the event
        release();
        return 0L;
    }

    //====================================================================
    //= FormScriptingEnvironment
    //====================================================================
    //--------------------------------------------------------------------
    FormScriptingEnvironment::FormScriptingEnvironment( FmFormModel& _rModel )
        :m_refCount( 0 )
        ,m_pScriptListener( NULL )
        ,m_rFormModel( _rModel )
        ,m_bDisposed( false )
    {
        m_pScriptListener = ListenerImplementation( new FormScriptListener( this ) );
        // note that this is a cyclic reference between the FormScriptListener and the FormScriptingEnvironment
        // This cycle is broken up when our instance is disposed.
    }

    //--------------------------------------------------------------------
    FormScriptingEnvironment::~FormScriptingEnvironment()
    {
    }

    //--------------------------------------------------------------------
    void FormScriptingEnvironment::impl_registerOrRevoke_throw( const Reference< XEventAttacherManager >& _rxManager, bool _bRegister )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !_rxManager.is() )
            throw IllegalArgumentException();
        if ( m_bDisposed )
            throw DisposedException();

        try
        {
            if ( _bRegister )
                _rxManager->addScriptListener( m_pScriptListener.getRef() );
            else
                _rxManager->removeScriptListener( m_pScriptListener.getRef() );
        }
        catch( const RuntimeException& ) { throw; }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void FormScriptingEnvironment::registerEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager )
    {
        impl_registerOrRevoke_throw( _rxManager, true );
    }

    //--------------------------------------------------------------------
    void FormScriptingEnvironment::revokeEventAttacherManager( const Reference< XEventAttacherManager >& _rxManager )
    {
        impl_registerOrRevoke_throw( _rxManager, false );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL FormScriptingEnvironment::acquire()
    {
        return osl_atomic_increment( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL FormScriptingEnvironment::release()
    {
        if ( 0 == osl_atomic_decrement( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }

    //--------------------------------------------------------------------
    IFormScriptingEnvironment::~IFormScriptingEnvironment()
    {
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        //. NewStyleUNOScript
        //................................................................
        class SAL_NO_VTABLE IScript
        {
        public:
            virtual void invoke( const Sequence< Any >& _rArguments, Any& _rSynchronousResult ) = 0;

            virtual ~IScript() { }
        };
        typedef ::boost::shared_ptr< IScript >  PScript;

        //................................................................
        //. NewStyleUNOScript
        //................................................................
        class NewStyleUNOScript : public IScript
        {
            SfxObjectShell&         m_rObjectShell;
            const ::rtl::OUString   m_sScriptCode;

        public:
            NewStyleUNOScript( SfxObjectShell& _rObjectShell, const ::rtl::OUString& _rScriptCode )
                :m_rObjectShell( _rObjectShell )
                ,m_sScriptCode( _rScriptCode )
            {
            }

            // IScript
            virtual void invoke( const Sequence< Any >& _rArguments, Any& _rSynchronousResult );
        };

        //................................................................
        void NewStyleUNOScript::invoke( const Sequence< Any >& _rArguments, Any& _rSynchronousResult )
        {
            Sequence< sal_Int16 > aOutArgsIndex;
            Sequence< Any > aOutArgs;
            EventObject aEvent;
            Any aCaller;
            if ( ( _rArguments.getLength() > 0 ) && ( _rArguments[ 0 ] >>= aEvent ) )
            {
                try
                {
                    Reference< XControl > xControl( aEvent.Source, UNO_QUERY_THROW );
                    Reference< XPropertySet > xProps( xControl->getModel(), UNO_QUERY_THROW );
                    aCaller = xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ) );
                }
                catch( Exception& ) {}
            }
            m_rObjectShell.CallXScript( m_sScriptCode, _rArguments, _rSynchronousResult, aOutArgsIndex, aOutArgs, true, aCaller.hasValue() ? &aCaller : 0 );
        }
    }

    //--------------------------------------------------------------------
    void FormScriptingEnvironment::doFireScriptEvent( const ScriptEvent& _rEvent, Any* _pSyncronousResult )
    {
#ifdef DISABLE_SCRIPTING
        (void) _rEvent;
        (void) _pSyncronousResult;
#else
        SolarMutexClearableGuard aSolarGuard;
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        if ( m_bDisposed )
            return;

        // SfxObjectShellRef is good here since the model controls the lifetime of the object
        SfxObjectShellRef xObjectShell = m_rFormModel.GetObjectShell();
        if( !xObjectShell.Is() )
            return;

        // the script to execute
        PScript pScript;

        if ( _rEvent.ScriptType != "StarBasic" )
        {
            pScript.reset( new NewStyleUNOScript( *xObjectShell, _rEvent.ScriptCode ) );
        }
        else
        {
            ::rtl::OUString sScriptCode = _rEvent.ScriptCode;
            ::rtl::OUString sMacroLocation;

            // is there a location in the script name ("application" or "document")?
            sal_Int32 nPrefixLen = sScriptCode.indexOf( ':' );
            DBG_ASSERT( 0 <= nPrefixLen, "FormScriptingEnvironment::doFireScriptEvent: Basic script name in old format encountered!" );

            if ( 0 <= nPrefixLen )
            {
                // and it has such a prefix
                sMacroLocation = sScriptCode.copy( 0, nPrefixLen );
                DBG_ASSERT( 0 == sMacroLocation.compareToAscii( "document" )
                        ||  0 == sMacroLocation.compareToAscii( "application" ),
                        "FormScriptingEnvironment::doFireScriptEvent: invalid (unknown) prefix!" );

                // strip the prefix: the SfxObjectShell::CallScript knows nothing about such prefixes
                sScriptCode = sScriptCode.copy( nPrefixLen + 1 );
            }

            if ( sMacroLocation.isEmpty() )
            {
                // legacy format: use the app-wide Basic, if it has a respective method, otherwise fall back to the doc's Basic
                if ( SFX_APP()->GetBasicManager()->HasMacro( sScriptCode ) )
                    sMacroLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application" ) );
                else
                    sMacroLocation = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "document" ) );
            }

            ::rtl::OUStringBuffer aScriptURI;
            aScriptURI.appendAscii( "vnd.sun.star.script:" );
            aScriptURI.append( sScriptCode );
            aScriptURI.appendAscii( "?language=Basic" );
            aScriptURI.appendAscii( "&location=" );
            aScriptURI.append( sMacroLocation );

            const ::rtl::OUString sScriptURI( aScriptURI.makeStringAndClear() );
            pScript.reset( new NewStyleUNOScript( *xObjectShell, sScriptURI ) );
        }

        OSL_ENSURE( pScript.get(), "FormScriptingEnvironment::doFireScriptEvent: no script to execute!" );
        if ( !pScript.get() )
            // this is an internal error in the above code
            throw RuntimeException();

        aGuard.clear();
        aSolarGuard.clear();

        Any aIgnoreResult;
        pScript->invoke( _rEvent.Arguments, _pSyncronousResult ? *_pSyncronousResult : aIgnoreResult );
        pScript.reset();

        {
            // object shells are not thread safe, so guard the destruction
            SolarMutexGuard aSolarGuarsReset;
            xObjectShell = NULL;
        }
#endif
    }

    //--------------------------------------------------------------------
    void FormScriptingEnvironment::dispose()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bDisposed = true;
        m_pScriptListener->dispose();
    }

    //--------------------------------------------------------------------
    PFormScriptingEnvironment createDefaultFormScriptingEnvironment( FmFormModel& _rModel )
    {
        return new FormScriptingEnvironment( _rModel );
    }

//........................................................................
} // namespace svxform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
