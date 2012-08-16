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
#ifndef _FILNOT_HXX_
#define _FILNOT_HXX_

#include <boost/unordered_map.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include "filglob.hxx"


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

        void notifyChildInserted( const rtl::OUString& aChildName );
        void notifyDeleted( void );
        void notifyRemoved( const rtl::OUString& aChildName );
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

        void SAL_CALL notifyPropertyAdded( const rtl::OUString & aPropertyName );
        void SAL_CALL notifyPropertyRemoved( const rtl::OUString & aPropertyName );
    };


    typedef boost::unordered_map< rtl::OUString,
                           com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >,
                           hashOUString,
                           equalOUString >      ListenerMap;

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
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent > seqChanged );
    };


    class Notifier
    {
    public:
        // Side effect of this function is the change of the name
        virtual ContentEventNotifier*          cEXC( const rtl::OUString aNewName ) = 0;
        // Side effect is the change of the state of the object to "deleted".
        virtual ContentEventNotifier*          cDEL( void ) = 0;
        virtual ContentEventNotifier*          cCEL( void ) = 0;
        virtual PropertySetInfoChangeNotifier* cPSL( void ) = 0;
        virtual PropertyChangeNotifier*        cPCL( void ) = 0;
        virtual rtl::OUString                  getKey( void ) = 0;

    protected:
        ~Notifier() {}
    };


}   // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
