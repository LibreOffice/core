/*************************************************************************
 *
 *  $RCSfile: filnot.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:36 $
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
#ifndef _FILNOT_HXX_
#define _FILNOT_HXX_

#ifndef __SGI_STL_HASH_MAP
#include <stl/hash_map>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTIDENTIFIER_HPP_
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#endif
#ifndef _FILGLOB_HXX_
#include "filglob.hxx"
#endif


namespace fileaccess {

    class shell;
    class BaseContent;

    class ContentEventNotifier
    {
    private:
        shell* m_pMyShell;
        BaseContent* m_pCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xOldId;
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > > m_sListeners;
    public:

        ContentEventNotifier(
            shell* pMyShell,
            BaseContent* pCreatorContent,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& xCreatorId,
            const com::sun::star::uno::Sequence<
            com::sun::star::uno::Reference< com::sun::star::uno::XInterface > >& sListeners );

        ContentEventNotifier(
            shell* pMyShell,
            BaseContent* pCreatorContent,
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
        BaseContent* m_pCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::uno::XInterface > > m_sListeners;
    public:
        PropertySetInfoChangeNotifier(
            shell* pMyShell,
            BaseContent* pCreatorContent,
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
        BaseContent* m_pCreatorContent;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > m_xCreatorId;
        ListenerMap* m_pListeners;
    public:
        PropertyChangeNotifier(
            shell* pMyShell,
            BaseContent* pCreatorContent,
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
