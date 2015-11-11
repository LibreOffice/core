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

class ComplexToolbarController : public svt::ToolboxController

{
    public:
        ComplexToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                  const css::uno::Reference< css::frame::XFrame >& rFrame,
                                  ToolBox* pToolBar,
                                  sal_uInt16       nID,
                                  const OUString& aCommand );
        virtual ~ComplexToolbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

        DECL_STATIC_LINK_TYPED( ComplexToolbarController, ExecuteHdl_Impl, void*, void );
        DECL_STATIC_LINK_TYPED( ComplexToolbarController, Notify_Impl, void*, void);

        struct ExecuteInfo
        {
            css::uno::Reference< css::frame::XDispatch >     xDispatch;
            css::util::URL                                   aTargetURL;
            css::uno::Sequence< css::beans::PropertyValue >  aArgs;
        };

        struct NotifyInfo
        {
            OUString                                                        aEventName;
            css::uno::Reference< css::frame::XControlNotificationListener > xNotifyListener;
            css::util::URL                                                  aSourceURL;
            css::uno::Sequence< css::beans::NamedValue >                    aInfoSeq;
        };

    protected:
        static sal_Int32 getFontSizePixel( const vcl::Window* pWindow );
        css::uno::Reference< css::frame::XDispatch > getDispatchFromCommand( const OUString& aCommand ) const;
        void addNotifyInfo( const OUString&                                                        aEventName,
                            const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                            const css::uno::Sequence< css::beans::NamedValue >& rInfo );

        virtual void executeControlCommand( const css::frame::ControlCommand& rControlCommand ) = 0;
        virtual css::uno::Sequence< css::beans::PropertyValue> getExecuteArgs(sal_Int16 KeyModifier) const;
        const css::util::URL& getInitializedURL();
        void notifyFocusGet();
        void notifyFocusLost();
        void notifyTextChanged( const OUString& aText );

        VclPtr<ToolBox>                                      m_pToolbar;
        sal_uInt16                                           m_nID;
        bool                                                 m_bMadeInvisible;
        mutable css::util::URL                               m_aURL;
        css::uno::Reference< css::util::XURLTransformer >    m_xURLTransformer;
};

}

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_COMPLEXTOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
