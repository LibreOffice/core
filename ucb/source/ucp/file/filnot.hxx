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
#pragma once

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace fileaccess {

    class TaskManager;

    class ContentEventNotifier
    {
    private:
        TaskManager* m_pMyShell;
        css::uno::Reference< css::ucb::XContent > m_xCreatorContent;
        css::uno::Reference< css::ucb::XContentIdentifier > m_xCreatorId;
        css::uno::Reference< css::ucb::XContentIdentifier > m_xOldId;
        std::vector< css::uno::Reference< css::uno::XInterface > > m_sListeners;
    public:

        ContentEventNotifier(
            TaskManager* pMyShell,
            const css::uno::Reference< css::ucb::XContent >& xCreatorContent,
            const css::uno::Reference< css::ucb::XContentIdentifier >& xCreatorId,
            const std::vector< css::uno::Reference< css::uno::XInterface > >& sListeners );

        ContentEventNotifier(
            TaskManager* pMyShell,
            const css::uno::Reference< css::ucb::XContent >& xCreatorContent,
            const css::uno::Reference< css::ucb::XContentIdentifier >& xCreatorId,
            const css::uno::Reference< css::ucb::XContentIdentifier >& xOldId,
            const std::vector< css::uno::Reference< css::uno::XInterface > >& sListeners );

        void notifyChildInserted( const OUString& aChildName );
        void notifyDeleted();
        void notifyRemoved( const OUString& aChildName );
        void notifyExchanged( );
    };


    class PropertySetInfoChangeNotifier
    {
    private:
        css::uno::Reference< css::ucb::XContent > m_xCreatorContent;
        std::vector< css::uno::Reference< css::uno::XInterface > > m_sListeners;
    public:
        PropertySetInfoChangeNotifier(
            const css::uno::Reference< css::ucb::XContent >& xCreatorContent,
            const std::vector< css::uno::Reference< css::uno::XInterface > >& sListeners );

        void notifyPropertyAdded( const OUString & aPropertyName );
        void notifyPropertyRemoved( const OUString & aPropertyName );
    };


    typedef std::unordered_map< OUString,
                           css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > >  ListenerMap;

    class PropertyChangeNotifier
    {
    private:
        css::uno::Reference< css::ucb::XContent > m_xCreatorContent;
        std::unique_ptr<ListenerMap> m_pListeners;
    public:
        PropertyChangeNotifier(
            const css::uno::Reference< css::ucb::XContent >& xCreatorContent,
            std::unique_ptr<ListenerMap> pListeners );

        ~PropertyChangeNotifier();

        void notifyPropertyChanged(
            const css::uno::Sequence< css::beans::PropertyChangeEvent >& seqChanged );
    };


    class Notifier
    {
    public:
        // Side effect of this function is the change of the name
        virtual std::unique_ptr<ContentEventNotifier> cEXC( const OUString& aNewName ) = 0;
        // Side effect is the change of the state of the object to "deleted".
        virtual std::unique_ptr<ContentEventNotifier> cDEL() = 0;
        virtual std::unique_ptr<ContentEventNotifier> cCEL() = 0;
        virtual std::unique_ptr<PropertySetInfoChangeNotifier> cPSL() = 0;
        virtual std::unique_ptr<PropertyChangeNotifier> cPCL() = 0;

    protected:
        ~Notifier() {}
    };


}   // end namespace fileaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
