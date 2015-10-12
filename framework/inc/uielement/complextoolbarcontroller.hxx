/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX

#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/ControlCommand.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

#include <svtools/toolboxcontroller.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;
namespace vcl { class Window; }

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
    OUString                                                                           aEventName;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XControlNotificationListener > xNotifyListener;
    ::com::sun::star::util::URL                                                               aSourceURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >                    aInfoSeq;
};

class ComplexToolbarController : public svt::ToolboxController

{
    public:
        ComplexToolbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                                  const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                  ToolBox* pToolBar,
                                  sal_uInt16       nID,
                                  const OUString& aCommand );
        virtual ~ComplexToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        DECL_STATIC_LINK_TYPED( ComplexToolbarController, ExecuteHdl_Impl, void*, void );
        DECL_STATIC_LINK_TYPED( ComplexToolbarController, Notify_Impl, void*, void);

    protected:
        static sal_Int32 getFontSizePixel( const vcl::Window* pWindow );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > getDispatchFromCommand( const OUString& aCommand ) const;
        void addNotifyInfo( const OUString&                                                        aEventName,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& xDispatch,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& rInfo );

        virtual void executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand ) = 0;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;
        const ::com::sun::star::util::URL& getInitializedURL();
        void notifyFocusGet();
        void notifyFocusLost();
        void notifyTextChanged( const OUString& aText );

        VclPtr<ToolBox>                                                             m_pToolbar;
        sal_uInt16                                                                  m_nID;
        bool                                                                        m_bMadeInvisible;
        mutable ::com::sun::star::util::URL                                         m_aURL;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
