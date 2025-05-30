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
#include <com/sun/star/container/XChild.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <tools/link.hxx>
#include "FormattedFieldBeautifier.hxx"
#include "FixedTextColor.hxx"
#include <memory>
#include <vector>


class VclSimpleEvent;

namespace rptui
{
    class OReportController;


    class OXReportControllerObserver
        : public ::cppu::WeakImplHelper<   css::beans::XPropertyChangeListener
                                        ,   css::container::XContainerListener
                                        ,   css::util::XModifyListener
                                        >
    {

        ::std::vector< css::uno::Reference< css::container::XChild> > m_aSections;
        ::osl::Mutex             m_aMutex;
        oslInterlockedCount      m_nLocks;
        FormattedFieldBeautifier m_aFormattedFieldBeautifier;
        FixedTextColor           m_aFixedTextColor;

        // do not allow copy
        OXReportControllerObserver(const OXReportControllerObserver&) = delete;
        OXReportControllerObserver& operator=(const OXReportControllerObserver&) = delete;
        virtual  ~OXReportControllerObserver() override; // UNO Object must have private destructor!
    public:
        OXReportControllerObserver(const OReportController& _rController);

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) override;
        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) override;
        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;


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

        void Clear();
    private:

        void switchListening( const css::uno::Reference< css::container::XIndexAccess >& _rxContainer, bool _bStartListening );
        void switchListening( const css::uno::Reference< css::uno::XInterface >& _rxObject, bool _bStartListening );

        DECL_LINK(SettingsChanged, VclSimpleEvent&, void );
    };

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTCONTROLLEROBSERVER_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
