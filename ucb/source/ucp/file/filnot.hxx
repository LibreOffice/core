/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _FILNOT_HXX_
#define _FILNOT_HXX_

#include <hash_map>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include "filglob.hxx"


namespace fileaccess {

    class shell;
    class BaseContent;

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
        shell* m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContent > m_xCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > > m_sListeners;
    public:
        PropertySetInfoChangeNotifier(
            shell* pMyShell,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContent >& xCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >& sListeners );

        void SAL_CALL notifyPropertyAdded( const rtl::OUString & aPropertyName );
        void SAL_CALL notifyPropertyRemoved( const rtl::OUString & aPropertyName );
    };


    typedef std::hash_map< rtl::OUString,
                           com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >,
                           hashOUString,
                           equalOUString >      ListenerMap;

    class PropertyChangeNotifier
    {
    private:
        shell* m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContent > m_xCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        ListenerMap* m_pListeners;
    public:
        PropertyChangeNotifier(
            shell* pMyShell,
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
    };


}   // end namespace fileaccess

#endif
