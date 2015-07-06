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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILNOT_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILNOT_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include "filglob.hxx"
#include <unordered_map>

namespace fileaccess {

    class shell;

    class ContentEventNotifier
    {
    private:
        shell* m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContent > m_xCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xOldId;
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > > m_sListeners;
    public:

        ContentEventNotifier(
            shell* pMyShell,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContent >& xCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >& sListeners );

        ContentEventNotifier(
            shell* pMyShell,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContent >& xCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xOldId,
            const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >& sListeners );

        void notifyChildInserted( const OUString& aChildName );
        void notifyDeleted();
        void notifyRemoved( const OUString& aChildName );
        void notifyExchanged( );
    };


    class PropertySetInfoChangeNotifier
    {
    private:
        com::sun::star::uno::Reference< com::sun::star::ucb::XContent > m_xCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > > m_sListeners;
    public:
        PropertySetInfoChangeNotifier(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContent >& xCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >& sListeners );

        void SAL_CALL notifyPropertyAdded( const OUString & aPropertyName );
        void SAL_CALL notifyPropertyRemoved( const OUString & aPropertyName );
    };


    typedef std::unordered_map< OUString,
                           com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >,
                           OUStringHash >      ListenerMap;

    class PropertyChangeNotifier
    {
    private:
        com::sun::star::uno::Reference< com::sun::star::ucb::XContent > m_xCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        ListenerMap* m_pListeners;
    public:
        PropertyChangeNotifier(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContent >& xCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            ListenerMap* pListeners );

        ~PropertyChangeNotifier();

        void notifyPropertyChanged(
            const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent >& seqChanged );
    };


    class Notifier
    {
    public:
        // Side effect of this function is the change of the name
        virtual ContentEventNotifier*          cEXC( const OUString& aNewName ) = 0;
        // Side effect is the change of the state of the object to "deleted".
        virtual ContentEventNotifier*          cDEL() = 0;
        virtual ContentEventNotifier*          cCEL() = 0;
        virtual PropertySetInfoChangeNotifier* cPSL() = 0;
        virtual PropertyChangeNotifier*        cPCL() = 0;

    protected:
        ~Notifier() {}
    };


}   // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
