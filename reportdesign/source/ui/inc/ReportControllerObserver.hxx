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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLEROBSERVER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLEROBSERVER_HXX

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <cppuhelper/implbase.hxx>

#include <memory>
#include <dllapi.h>
#include <vector>

#include <vcl/svapp.hxx>
#include <tools/link.hxx>

#include "FormattedFieldBeautifier.hxx"
#include "FixedTextColor.hxx"

namespace rptui
{
    class OReportController;
    class OXReportControllerObserverImpl;


    class OXReportControllerObserver
        : public ::cppu::WeakImplHelper<   css::beans::XPropertyChangeListener
                                        ,   css::container::XContainerListener
                                        ,   css::util::XModifyListener
                                        >
    {

        const ::std::unique_ptr<OXReportControllerObserverImpl> m_pImpl;

        FormattedFieldBeautifier m_aFormattedFieldBeautifier;
        FixedTextColor           m_aFixedTextColor;

        // do not allow copy
        OXReportControllerObserver(const OXReportControllerObserver&) = delete;
        OXReportControllerObserver& operator=(const OXReportControllerObserver&) = delete;
        virtual  ~OXReportControllerObserver(); // UNO Object must have private destructor!
    public:
        OXReportControllerObserver(const OReportController& _rController);

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) throw(css::uno::RuntimeException, std::exception) override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;


        void AddElement(const css::uno::Reference< css::uno::XInterface>& Element);
        void RemoveElement(const css::uno::Reference< css::uno::XInterface>& Element);

        void AddSection( const css::uno::Reference< css::report::XSection>& _xSection);
        void RemoveSection( const css::uno::Reference< css::report::XSection>& _xSection );

        /**
           Create an object ob OUndoEnvLock locks the undo possibility
           As long as in the OUndoEnvLock scope, no undo is possible for manipulated object.
         */
        class OEnvLock
        {
            OXReportControllerObserver& m_rObserver;
        public:
            OEnvLock(OXReportControllerObserver& _rObserver): m_rObserver(_rObserver){m_rObserver.Lock();}
            ~OEnvLock(){ m_rObserver.UnLock(); }
        };

        void Lock();
        void UnLock();
        bool IsLocked() const;

        void Clear();
    private:

        void TogglePropertyListening(const css::uno::Reference< css::uno::XInterface>& Element);
        void switchListening( const css::uno::Reference< css::container::XIndexAccess >& _rxContainer, bool _bStartListening );
        void switchListening( const css::uno::Reference< css::uno::XInterface >& _rxObject, bool _bStartListening );

        DECL_LINK_TYPED(SettingsChanged, VclSimpleEvent&, void );
    private:

        ::std::vector< css::uno::Reference< css::container::XChild> >::const_iterator getSection(const css::uno::Reference< css::container::XChild>& _xContainer) const;

    };

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLEROBSERVER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
