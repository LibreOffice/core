/*************************************************************************
 *
 *  $RCSfile: dlgevtatt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:27:22 $
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

#ifndef SCRIPTING_DLGEVTATT_HXX
#define SCRIPTING_DLGEVTATT_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENER_HPP_
#include <com/sun/star/script/XAllListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHER_HPP_
#include <com/sun/star/script/XEventAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTEVENTSATTACHER_HPP_
#include <com/sun/star/script/XScriptEventsAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTLISTENER_HPP_
#include <com/sun/star/script/XScriptListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif


//.........................................................................
namespace dlgprov
{
//.........................................................................

    // =============================================================================
    // class DialogEventsAttacherImpl
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::XScriptEventsAttacher > DialogEventsAttacherImpl_BASE;


    class DialogEventsAttacherImpl : public DialogEventsAttacherImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacher > m_xEventAttacher;

    public:
        DialogEventsAttacherImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
        virtual ~DialogEventsAttacherImpl();

        // XScriptEventsAttacher
        virtual void SAL_CALL attachEvents( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& Objects,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& xListener,
            const ::com::sun::star::uno::Any& Helper )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::beans::IntrospectionException,
                   ::com::sun::star::script::CannotCreateAdapterException,
                   ::com::sun::star::lang::ServiceNotRegisteredException,
                   ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class DialogAllListenerImpl
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::XAllListener > DialogAllListenerImpl_BASE;


    class DialogAllListenerImpl : public DialogAllListenerImpl_BASE
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener > m_xScriptListener;
        ::rtl::OUString m_sScriptType;
        ::rtl::OUString m_sScriptCode;

        virtual void firing_impl( const ::com::sun::star::script::AllEventObject& Event, ::com::sun::star::uno::Any* pRet );

    public:
        DialogAllListenerImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XScriptListener >& rxListener,
            const ::rtl::OUString& rScriptType, const ::rtl::OUString& rScriptCode );
        virtual ~DialogAllListenerImpl();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException);

        // XAllListener
        virtual void SAL_CALL firing( const ::com::sun::star::script::AllEventObject& Event )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL approveFiring( const ::com::sun::star::script::AllEventObject& Event )
            throw (::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);
    };


    // =============================================================================
    // class DialogScriptListenerImpl
    // =============================================================================

    typedef ::cppu::WeakImplHelper1<
        ::com::sun::star::script::XScriptListener > DialogScriptListenerImpl_BASE;


    class DialogScriptListenerImpl : public DialogScriptListenerImpl_BASE
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >             m_xModel;

        virtual void firing_impl( const ::com::sun::star::script::ScriptEvent& aScriptEvent, ::com::sun::star::uno::Any* pRet );

    public:
        DialogScriptListenerImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel );
        virtual ~DialogScriptListenerImpl();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw (::com::sun::star::uno::RuntimeException);

        // XScriptListener
        virtual void SAL_CALL firing( const ::com::sun::star::script::ScriptEvent& aScriptEvent )
            throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL approveFiring( const ::com::sun::star::script::ScriptEvent& aScriptEvent )
            throw (::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace dlgprov
//.........................................................................

#endif // SCRIPTING_DLGEVT_HXX
