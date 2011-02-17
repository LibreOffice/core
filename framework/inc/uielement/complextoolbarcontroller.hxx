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

#ifndef __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_

#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <svtools/toolboxcontroller.hxx>

#include <vcl/toolbox.hxx>

namespace framework
{

struct ExecuteInfo
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
    ::com::sun::star::util::URL                                                aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
};

struct NotifyInfo
{
    ::rtl::OUString                                                                           aEventName;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XControlNotificationListener > xNotifyListener;
    ::com::sun::star::util::URL                                                               aSourceURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >                    aInfoSeq;
};

class ToolBar;

class ComplexToolbarController : public svt::ToolboxController

{
    public:
        ComplexToolbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBox*     pToolBar,
                                  sal_uInt16       nID,
                                  const rtl::OUString& aCommand );
        virtual ~ComplexToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        DECL_STATIC_LINK( ComplexToolbarController, ExecuteHdl_Impl, ExecuteInfo* );
        DECL_STATIC_LINK( ComplexToolbarController, Notify_Impl, NotifyInfo* );

    protected:
        static sal_Int32 getFontSizePixel( const Window* pWindow );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > getDispatchFromCommand( const rtl::OUString& aCommand ) const;
        void addNotifyInfo( const ::rtl::OUString&                                                        aEventName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& xDispatch,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rInfo );

        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand ) = 0;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;
        const ::com::sun::star::util::URL& getInitializedURL();
        void notifyFocusGet();
        void notifyFocusLost();
        void notifyTextChanged( const ::rtl::OUString& aText );

        ToolBox*                                                                    m_pToolbar;
        sal_uInt16                                                                  m_nID;
        sal_Bool                                                                    m_bMadeInvisible;
        mutable ::com::sun::star::util::URL                                         m_aURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
};

}

#endif // __FRAMEWORK_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX_
