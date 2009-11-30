/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: broadcaster.cxx,v $
 * $Revision: 1.21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "broadcaster.hxx"
#include "notifierimpl.hxx"
#include "confignotifier.hxx"
#include "noderef.hxx"
#include "nodechange.hxx"
#include "nodechangeinfo.hxx"
#include "translatechanges.hxx"
#include "apifactory.hxx"
#include "apitreeaccess.hxx"
#include "apitreeimplobj.hxx"
#include <vos/refernce.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif
#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif
#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

namespace configmgr
{
    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------
// Broadcaster implementation
// ---------------------------------------------------------------------------------------------------
    class BroadcasterHelper
    {
    public:
        static vos::ORef<NotifierImpl> getImpl(Notifier const& aNotifier) { return aNotifier.m_aImpl; }
    };
// ---------------------------------------------------------------------------------------------------

    namespace
    {
    // -----------------------------------------------------------------------------------------------
        template <class T>
        struct LessORefBodyPtr
        {
            bool operator()(vos::ORef<T> const& lhs, vos::ORef<T> const& rhs) const
            {
                return ptr_less(lhs.getBodyPtr(), rhs.getBodyPtr());
            }

            std::less<T*> ptr_less;
        };
    // -----------------------------------------------------------------------------------------------
        class ApiTreeRef
        {
            ApiTreeImpl const*  m_pApiTree;
            uno::Reference<uno::XInterface>     m_xKeepAlive;
        public:
            explicit ApiTreeRef(ApiTreeImpl const* _pApiTree = NULL)
            : m_pApiTree(_pApiTree)
            , m_xKeepAlive()
            {
                if (m_pApiTree) m_xKeepAlive = m_pApiTree->getUnoInstance();
            }

            bool is() const
            {
                OSL_ASSERT(!m_pApiTree == !m_xKeepAlive.is());
                return m_pApiTree != NULL;
            }

            ApiTreeImpl const* get()        const { return m_pApiTree; }
            ApiTreeImpl const* operator->() const { return m_pApiTree; }

            friend bool operator==(ApiTreeRef const& lhs,ApiTreeRef const& rhs)
            { return lhs.m_pApiTree == rhs.m_pApiTree; }

            friend bool operator!=(ApiTreeRef const& lhs,ApiTreeRef const& rhs)
            { return lhs.m_pApiTree != rhs.m_pApiTree; }
        };
    // -----------------------------------------------------------------------------------------------
        typedef std::map< vos::ORef<NotifierImpl>, ApiTreeRef, LessORefBodyPtr<NotifierImpl> > NotifierSet;
    // -----------------------------------------------------------------------------------------------

    }
// ---------------------------------------------------------------------------------------------------
// class Broadcaster::Impl
// ---------------------------------------------------------------------------------------------------
    class Broadcaster::Impl : public vos::OReference
    {
    private:
        NotifierSet::value_type m_aNotifierData;
    public:
        Impl(NotifierSet::value_type const& aNotifierData) : m_aNotifierData(aNotifierData) {}

        NotifierSet::value_type getNotifierData() const { return m_aNotifierData; }

        bool translateChanges(configuration::NodeChangesInformation& aInfos, configuration::NodeChanges const& aChanges, bool bSingleBase) const;
        bool translateChanges(configuration::NodeChangesInformation& aInfos, configuration::NodeChangesInformation const& aChanges, bool bSingleBase) const;

        void queryConstraints(configuration::NodeChangesInformation const& aChanges)    { this->doQueryConstraints(aChanges); }
        void notifyListeners(configuration::NodeChangesInformation const& aChanges) { this->doNotifyListeners(aChanges); }

        void notifyRootListeners(configuration::NodeChangesInformation const& aChanges);

        static vos::ORef<Impl> create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChange const& aChange, bool bLocal);
        static vos::ORef<Impl> create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChanges const& aChange, bool bLocal);
        static vos::ORef<Impl> create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChangeInformation const& aChange, bool bLocal);
        static vos::ORef<Impl> create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChangesInformation const& aChange, bool bLocal);

    private:
        virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges) = 0;
        virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges) = 0;
    };
// ---------------------------------------------------------------------------------------------------
    namespace
    {
    // -----------------------------------------------------------------------------------------------

        class EmptyBroadcaster_Impl : public Broadcaster::Impl
        {
            EmptyBroadcaster_Impl(NotifierSet::value_type const& rNotifierData)
            : Broadcaster::Impl(rNotifierData)
            {
            }
        public:
            static
            vos::ORef< Broadcaster::Impl > create(NotifierSet::value_type const& rRootNotifier)
            {
                return new EmptyBroadcaster_Impl(rRootNotifier);
            }
        private:
            virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges);
            virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges);
        };

        void EmptyBroadcaster_Impl::doQueryConstraints(configuration::NodeChangesInformation const&) {}
        void EmptyBroadcaster_Impl::doNotifyListeners(configuration::NodeChangesInformation const&) {}
    // -----------------------------------------------------------------------------------------------

        class NodeLocalBroadcaster_Impl : public Broadcaster::Impl
        {
            configuration::NodeID       aAffectedNode;

        public:
            NodeLocalBroadcaster_Impl(NotifierSet::value_type const& rTreeNotifierData, configuration::NodeID const& aAffectedID)
            : Broadcaster::Impl(rTreeNotifierData)
            , aAffectedNode(aAffectedID)
            {
            }

            configuration::NodeID getAffectedNodeID() const { return aAffectedNode; }
            unsigned int getNodeIndex() const { return aAffectedNode.toIndex(); }

        protected:
            void querySingleConstraint(configuration::NodeChangeInformation const& aChange, bool bMore);
            void notifySingleChange(configuration::NodeChangeInformation const& aChange, bool bMore, css::beans::PropertyChangeEvent*& pCurEvent);
        };
    // -----------------------------------------------------------------------------------------------

        class SingleChangeBroadcaster_Impl : public NodeLocalBroadcaster_Impl
        {
            configuration::SubNodeID m_aChangingValue;

            SingleChangeBroadcaster_Impl(NotifierSet::value_type const& rTreeNotifierData, configuration::NodeID const& aAffectedID, configuration::SubNodeID const& aChangedValue);

        public:
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeChangeLocation const& aChange);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeID const& aAffectedID,
                        configuration::NodeChangeLocation const& aChange);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeID const& aAffectedID,
                        configuration::SubNodeID const& aChangedNode,
                        configuration::NodeChangeLocation const& aChange);
        private:
            virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges);
            virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges);
        };

    // -----------------------------------------------------------------------------------------------
        class MultiChangeBroadcaster_Impl : public NodeLocalBroadcaster_Impl
        {
            std::set< configuration::SubNodeID >    m_aChangingNodes;

            MultiChangeBroadcaster_Impl(NotifierSet::value_type const& rTreeNotifierData, configuration::NodeID const& aAffectedID, std::set< configuration::SubNodeID >& aChangedNodes);

        public:
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeChangesInformation const& aChanges);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeID const& aAffectedID,
                        configuration::NodeChangesInformation const& aChanges);
        private:
            virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges);
            virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges);
        };
    // -----------------------------------------------------------------------------------------------
        class SingleTreeBroadcaster_Impl : public Broadcaster::Impl
        {
            std::vector< vos::ORef<NodeLocalBroadcaster_Impl> > m_aBroadcasters;

            SingleTreeBroadcaster_Impl(NotifierSet::value_type const& rTreeNotifierData, std::vector< vos::ORef<NodeLocalBroadcaster_Impl> >& aBroadcasters);

        public:
        //--------------------------
            static
            vos::ORef< Broadcaster::Impl > create(
                        NotifierSet::value_type const& rRootNotifier,
                        NotifierSet::value_type const& rLocalNotifier,
                        configuration::NodeChangesInformation const& aChanges);

            static bool selectChanges(configuration::NodeChangesInformation& rSelected, configuration::NodeChangesInformation const& aOriginal, configuration::NodeID const& aSelector);
        //--------------------------
        private:
            virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges);
            virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges);
        };
    // -----------------------------------------------------------------------------------------------
        class MultiTreeBroadcaster_Impl : public Broadcaster::Impl
        {
            std::vector< vos::ORef< Broadcaster::Impl > > m_aBroadcasters;

            MultiTreeBroadcaster_Impl(NotifierSet::value_type const& rRootNotifierData, std::vector< vos::ORef< Broadcaster::Impl > >& aBroadcasters);
        public:
        //--------------------------
            static
            vos::ORef< Broadcaster::Impl > create(
                        NotifierSet::value_type const& rRootNotifier,
                        NotifierSet const& rNotifiers,
                        configuration::NodeChangesInformation const& aChanges);

            static bool selectChanges(configuration::NodeChangesInformation& rSelected, configuration::NodeChangesInformation const& aOriginal, NotifierSet::value_type const& aSelector);
        //--------------------------
        private:
            virtual void doQueryConstraints(configuration::NodeChangesInformation const& aChanges);
            virtual void doNotifyListeners(configuration::NodeChangesInformation const& aChanges);
        };

    // -----------------------------------------------------------------------------------------------

        inline configuration::NodeID makeRootID( rtl::Reference< configuration::Tree > const& aTree ) { return configuration::NodeID( aTree, aTree->getRootNode() ); }
        inline configuration::NodeID makeRootID( ApiTreeRef const& pTreeImpl ) { return makeRootID( pTreeImpl->getTree() ); }
    // -----------------------------------------------------------------------------------------------
        NotifierSet::value_type findNotifier(configuration::NodeChangeLocation const& aChange, ApiTreeRef const& pTreeImpl)
        {
            OSL_ENSURE(aChange.isValidData(),"Invalid change location - cannot find notifier");

            configuration::NodeID aAffectedNode = aChange.getAffectedNodeID();
            if (aAffectedNode.isEmpty())
                return NotifierSet::value_type();

            ApiTreeRef aAffectedImpl( Factory::findDescendantTreeImpl(aAffectedNode, pTreeImpl.get()) );
            if (aAffectedImpl.is())
            {
                vos::ORef<NotifierImpl> aAffectedNotifier = BroadcasterHelper::getImpl(aAffectedImpl->getNotifier());

                return  NotifierSet::value_type(aAffectedNotifier, aAffectedImpl);
            }
            else
                return NotifierSet::value_type();
        }
    // -----------------------------------------------------------------------------------------------
        inline
        NotifierSet::value_type findNotifier(configuration::NodeChangeInformation const& aChange, ApiTreeRef const& pTreeImpl)
        {
            return findNotifier(aChange.location,pTreeImpl);
        }
    // -----------------------------------------------------------------------------------------------

        void findNotifiers(NotifierSet& aNotifiers, configuration::NodeChangesInformation const& aChanges, ApiTreeRef const& pTreeImpl )
        {
            for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
            {
                NotifierSet::value_type aNotifierData( findNotifier(*it,pTreeImpl) );

                if (aNotifierData.first.isValid())
                {
                    aNotifiers.insert( aNotifierData );
                    OSL_ENSURE( aNotifiers[aNotifierData.first] == aNotifierData.second, "Different Api Trees for the same notifier" );
                }
            }
        }
    // -----------------------------------------------------------------------------------------------
    // NodeLocalBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------
        void NodeLocalBroadcaster_Impl::querySingleConstraint(configuration::NodeChangeInformation const& aChange, bool bMore)
        {
            uno::Reference< css::beans::XVetoableChangeListener > const * const SelectListener = 0;

            vos::ORef<NotifierImpl> pNotifierImpl = getNotifierData().first;

            cppu::OInterfaceContainerHelper* pListeners = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );
            cppu::OInterfaceContainerHelper* pSpecial       = pNotifierImpl->m_aListeners.getSpecialContainer( aChange.location.getChangingValueID() );

            if (pSpecial || pListeners)
            {
                css::beans::PropertyChangeEvent aEvent;
                aEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

                if (configapi::fillEventDataFromResolved(aEvent,aChange,bMore))
                {
                    // Catch only RuntimeExceptions here: vetoableChange issues its veto by throwing
                    // a PropertyVetoException (which is not a RuntimeException)
                    if (pListeners)
                    {
                        ListenerContainerIterator< css::beans::XVetoableChangeListener > aIterator(*pListeners);

                        UnoApiLockReleaser aGuardReleaser;
                        while (aIterator.hasMoreElements())
                        try
                        {
                            aIterator.next()->vetoableChange(aEvent);
                        }
                        catch (uno::RuntimeException & )
                        {}
                    }
                    if (pSpecial)
                    {
                        ListenerContainerIterator< css::beans::XVetoableChangeListener > aIterator(*pSpecial);

                        UnoApiLockReleaser aGuardReleaser;
                        while (aIterator.hasMoreElements())
                        try
                        {
                            aIterator.next()->vetoableChange(aEvent);
                        }
                        catch (uno::RuntimeException & )
                        {}
                    }
                }
            }

        }
    // -----------------------------------------------------------------------------------------------
        void NodeLocalBroadcaster_Impl::notifySingleChange(configuration::NodeChangeInformation const& aChange, bool bMore, css::beans::PropertyChangeEvent*& pCurEvent)
        {
            uno::Reference< css::beans::XPropertyChangeListener > const * const     SelectPropertyListener = 0;
            uno::Reference< css::container::XContainerListener > const * const          SelectContainerListener = 0;

            vos::ORef<NotifierImpl> pNotifierImpl = getNotifierData().first;

            cppu::OInterfaceContainerHelper* pContainerListeners    = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectContainerListener) );

            if (pContainerListeners)
            {
                css::container::ContainerEvent aEvent;
                aEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

                if (configapi::fillEventDataFromResolved(aEvent,aChange))
                {

                    ListenerContainerIterator< css::container::XContainerListener > aIterator(*pContainerListeners);

                    UnoApiLockReleaser aGuardReleaser;
                    while (aIterator.hasMoreElements())
                    try
                    {
                        uno::Reference<css::container::XContainerListener> xListener( aIterator.next() );
                        OSL_ASSERT( xListener.is() );

                        switch (aChange.change.type)
                        {
                        case configuration::NodeChangeData::eSetValue:
                        case configuration::NodeChangeData::eSetDefault:
                        case configuration::NodeChangeData::eReplaceElement:
                            xListener->elementReplaced(aEvent);
                            break;

                        case configuration::NodeChangeData::eInsertElement:
                            xListener->elementInserted(aEvent);
                            break;

                        case configuration::NodeChangeData::eRemoveElement:
                            xListener->elementRemoved(aEvent);
                            break;


                        case configuration::NodeChangeData::eResetSetDefault:
                        case configuration::NodeChangeData::eRenameElementTree:
                        case configuration::NodeChangeData::eNoChange:
                            OSL_ASSERT(false);
                            break;
                        }
                    }
                    catch (uno::Exception &)
                    {}
                }
            }


            OSL_ASSERT(pCurEvent);
            css::beans::PropertyChangeEvent& rEvent = *pCurEvent;

            rEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

            if (configapi::fillEventDataFromResolved(rEvent,aChange,bMore))
            {
                cppu::OInterfaceContainerHelper* pPropertyListeners = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectPropertyListener) );
                if (pPropertyListeners)
                {
                    ListenerContainerIterator< css::beans::XPropertyChangeListener > aIterator(*pPropertyListeners);
                    UnoApiLockReleaser aGuardReleaser;
                    while (aIterator.hasMoreElements())
                        try { aIterator.next()->propertyChange(rEvent); } catch (uno::Exception & ) {}
                }

                cppu::OInterfaceContainerHelper* pSpecialListeners  = pNotifierImpl->m_aListeners.getSpecialContainer( aChange.location.getChangingValueID() );
                if (pSpecialListeners)
                {
                    ListenerContainerIterator< css::beans::XPropertyChangeListener > aIterator(*pSpecialListeners);
                    UnoApiLockReleaser aGuardReleaser;
                    while (aIterator.hasMoreElements())
                        try { aIterator.next()->propertyChange(rEvent); } catch (uno::Exception & ) {}
                }

                ++pCurEvent;
            }

        }
    // -----------------------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------------------
    // SingleBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------
        SingleChangeBroadcaster_Impl::SingleChangeBroadcaster_Impl(
            NotifierSet::value_type const& rTreeNotifierData,
            configuration::NodeID const& aAffectedID, configuration::SubNodeID const& aChangedNode
        )
        : NodeLocalBroadcaster_Impl(rTreeNotifierData,aAffectedID)
        , m_aChangingValue(aChangedNode)
        {
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeChangeLocation const& aChange)
        {
            OSL_ENSURE(rLocalNotifier.second->getTree() == aChange.getAffectedTreeRef(),
                        "ERROR: Tree Mismatch creating Single Broadcaster");

            OSL_ENSURE(aChange.isValidData(), "ERROR: Invalid Change Location for Broadcaster");

            configuration::NodeID aAffectedNodeID = aChange.getAffectedNodeID();
            if (aAffectedNodeID.isEmpty())
                return 0;

            return create(rLocalNotifier,aAffectedNodeID,aChange.getChangingValueID(),aChange);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeID const& aAffectedID,
                    configuration::NodeChangeLocation const& aChange)
        {

            return create(rLocalNotifier,aAffectedID,aChange.getChangingValueID(),aChange);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeID const& aAffectedID,
                    configuration::SubNodeID const& aChangedNodeID,
                    configuration::NodeChangeLocation const& aChange)
        {
                    { (void)aChange; }
            OSL_ENSURE(aChange.isValidData(), "ERROR: Invalid Change Location for Broadcaster");
            OSL_ENSURE(aAffectedID.isValidNode(),"Cannot broadcast without affected node");

            OSL_ENSURE(rLocalNotifier.second->getTree() == aChange.getAffectedTreeRef(),
                        "ERROR: Tree Mismatch creating Single Broadcaster");
            OSL_ENSURE( aChange.getAffectedNodeID() == aAffectedID,
                        "ERROR: Node Mismatch creating Single Broadcaster");
            OSL_ENSURE( aChange.getChangingValueID() == aChangedNodeID,
                        "ERROR: Value Node Mismatch creating Single Broadcaster");

            return new SingleChangeBroadcaster_Impl(rLocalNotifier,aAffectedID,aChangedNodeID);
        }
    // -----------------------------------------------------------------------------------------------
        void SingleChangeBroadcaster_Impl::doQueryConstraints(configuration::NodeChangesInformation const& aChanges)
        {
            OSL_ASSERT(aChanges.size() <= 1);
            if (!aChanges.empty())
            {
                std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin();

                OSL_ENSURE( m_aChangingValue == it->location.getChangingValueID(), "Broadcasting unanticipated change");

                querySingleConstraint(*it, false);
            }

        }
    // -----------------------------------------------------------------------------------------------
        void SingleChangeBroadcaster_Impl::doNotifyListeners(configuration::NodeChangesInformation const& aChanges)
        {
            OSL_ASSERT(aChanges.size() <= 1);
            if (!aChanges.empty())
            {
                css::beans::PropertyChangeEvent aEvent;
                css::beans::PropertyChangeEvent * pEventNext = &aEvent;

                std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin();

                OSL_ENSURE( m_aChangingValue == it->location.getChangingValueID(), "Broadcasting unanticipated change");

                notifySingleChange(*it, false, pEventNext);

                if (pEventNext != &aEvent)
                {
                    uno::Sequence< css::beans::PropertyChangeEvent > aPropertyEvents(&aEvent,1);

                    uno::Reference< css::beans::XPropertiesChangeListener > const * const SelectListener = 0;

                    vos::ORef<NotifierImpl> pNotifierImpl = getNotifierData().first;

                    cppu::OInterfaceContainerHelper* pContainer = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );

                    if (pContainer)
                    {
                        ListenerContainerIterator< css::beans::XPropertiesChangeListener > aIterator(*pContainer);
                        UnoApiLockReleaser aGuardReleaser;
                        while (aIterator.hasMoreElements())
                            try { aIterator.next()->propertiesChange(aPropertyEvents); } catch (uno::Exception & ) {}
                    }
                }
            }
        }

    // -----------------------------------------------------------------------------------------------
    // MultiChangeBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------

        MultiChangeBroadcaster_Impl::MultiChangeBroadcaster_Impl(
            NotifierSet::value_type const& rTreeNotifierData,
            configuration::NodeID const& aAffectedID, std::set< configuration::SubNodeID >& aChangedNodes
        )
        : NodeLocalBroadcaster_Impl(rTreeNotifierData,aAffectedID)
        , m_aChangingNodes()
        {
            m_aChangingNodes.swap(aChangedNodes);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* MultiChangeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeChangesInformation const& aChanges)
        {
            if (aChanges.empty())
                return 0;

            OSL_ENSURE(aChanges.begin()->hasValidLocation(), "ERROR: Invalid Change Location for Broadcaster");

            configuration::NodeID aAffectedNodeID = aChanges.begin()->location.getAffectedNodeID();
            if (aAffectedNodeID.isEmpty())
                return 0;

            return create(rLocalNotifier, aAffectedNodeID, aChanges);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* MultiChangeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeID const& aAffectedNodeID,
                    configuration::NodeChangesInformation const& aChanges)
        {
            if (aChanges.empty())
                return 0;

            else if (aChanges.size() == 1)
                return SingleChangeBroadcaster_Impl::create(rLocalNotifier,aAffectedNodeID,aChanges.begin()->location);

            else
            {
                OSL_ENSURE(aAffectedNodeID.isValidNode(),"Cannot broadcast without affected node");

                std::set< configuration::SubNodeID > aChangedNodes;
                for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
                {
                    OSL_ENSURE(it->hasValidLocation(), "ERROR: Invalid Change Location for Broadcaster");

                    OSL_ENSURE(it->location.getAffectedNodeID() == aAffectedNodeID, "ERROR: Change is not local to affected node (as advertised)");
                    OSL_ENSURE(rLocalNotifier.second->getTree() == it->location.getAffectedTreeRef(),
                                "ERROR: Tree Mismatch creating Multi Change Broadcaster");

                    configuration::SubNodeID aChangedValueID = it->location.getChangingValueID();

                    aChangedNodes.insert(aChangedValueID);
                }
                OSL_ENSURE(!aChangedNodes.empty(), "Changes don't affect any nodes");

                if (aChangedNodes.size() == 1) OSL_TRACE("WARNING: Different changes all affect the same node !");

                return new MultiChangeBroadcaster_Impl(rLocalNotifier, aAffectedNodeID, aChangedNodes);
            }
        }
    // -----------------------------------------------------------------------------------------------
        void MultiChangeBroadcaster_Impl::doQueryConstraints(configuration::NodeChangesInformation const& aChanges)
        {
            std::vector< configuration::NodeChangeInformation >::const_iterator const stop = aChanges.end(), last = stop-1;

            for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != stop; ++it)
            {
                OSL_ENSURE( m_aChangingNodes.find( it->location.getChangingValueID() ) != m_aChangingNodes.end(), "Broadcasting unanticipated change");

                querySingleConstraint(*it, it != last);
            }

        }
    // -----------------------------------------------------------------------------------------------
        void MultiChangeBroadcaster_Impl::doNotifyListeners(configuration::NodeChangesInformation const& aChanges)
        {
            uno::Sequence< css::beans::PropertyChangeEvent > aPropertyEvents(aChanges.size());

            css::beans::PropertyChangeEvent * const pEventStart = aPropertyEvents.getArray();
            css::beans::PropertyChangeEvent * pEventNext = pEventStart;

            std::vector< configuration::NodeChangeInformation >::const_iterator const stop = aChanges.end(), last = stop-1;

            for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != stop; ++it)
            {
                // #92463# Skip nodes that are not in the tree
                if (it->location.getAffectedNodeID().isEmpty()) continue;

                OSL_ENSURE( m_aChangingNodes.find( it->location.getChangingValueID() ) != m_aChangingNodes.end(), "Broadcasting unanticipated change");

                notifySingleChange(*it, it != last, pEventNext);
            }

            sal_Int32 nPropertyEvents = pEventNext-pEventStart;

            if (nPropertyEvents > 0)
            {
                OSL_ASSERT(nPropertyEvents <= aPropertyEvents.getLength());
                if (nPropertyEvents != aPropertyEvents.getLength())
                    aPropertyEvents.realloc(nPropertyEvents);

                uno::Reference< css::beans::XPropertiesChangeListener > const * const SelectListener = 0;

                vos::ORef<NotifierImpl> pNotifierImpl = getNotifierData().first;

                cppu::OInterfaceContainerHelper* pContainer = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );

                if (pContainer)
                {
                    ListenerContainerIterator< css::beans::XPropertiesChangeListener > aIterator(*pContainer);
                    UnoApiLockReleaser aGuardReleaser;
                    while (aIterator.hasMoreElements())
                        try { aIterator.next()->propertiesChange(aPropertyEvents); } catch (uno::Exception & ) {}
                }
            }
        }
    // -----------------------------------------------------------------------------------------------
    // TreeLocalBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------

        SingleTreeBroadcaster_Impl::SingleTreeBroadcaster_Impl(
            NotifierSet::value_type const& aTreeNotifierData,
            std::vector< vos::ORef<NodeLocalBroadcaster_Impl> >& aBroadcasters
        )
        : Broadcaster::Impl(aTreeNotifierData)
        , m_aBroadcasters()
        {
            m_aBroadcasters.swap(aBroadcasters);
        }

    // -----------------------------------------------------------------------------------------------

        bool SingleTreeBroadcaster_Impl::selectChanges(configuration::NodeChangesInformation& rSelected, configuration::NodeChangesInformation const& aOriginal, configuration::NodeID const& aSelector)
        {
            OSL_ASSERT(rSelected.empty()); // nothing in there yet

            for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if ( it->location.getAffectedNodeID() == aSelector )
                {
                    rSelected.push_back(*it);
                }
            }
            return !rSelected.empty();
        }
    // -----------------------------------------------------------------------------------------------

        vos::ORef< Broadcaster::Impl > SingleTreeBroadcaster_Impl::create(
                    NotifierSet::value_type const& rRootNotifier,
                    NotifierSet::value_type const& rLocalNotifier,
                    configuration::NodeChangesInformation const& aChanges)
        {
            std::set< configuration::NodeID > aNodes;
            for (std::vector< configuration::NodeChangeInformation >::const_iterator itChanges = aChanges.begin(); itChanges != aChanges.end(); ++itChanges)
            {
                OSL_ENSURE(itChanges->hasValidLocation(), "ERROR: Invalid Change Location for Broadcaster");

                configuration::NodeID aAffectedNodeID = itChanges->location.getAffectedNodeID();
                if (!aAffectedNodeID.isEmpty())
                    aNodes.insert(aAffectedNodeID);
            }

            std::vector< vos::ORef<NodeLocalBroadcaster_Impl> > aNodecasters;
            for (std::set< configuration::NodeID >::const_iterator itNodes = aNodes.begin(); itNodes != aNodes.end(); ++itNodes)
            {
                OSL_ASSERT(itNodes->isValidNode()); // filtered empty ones above

                configuration::NodeChangesInformation aSelectedChanges;
                if ( selectChanges(aSelectedChanges, aChanges, *itNodes))
                {
                    NodeLocalBroadcaster_Impl* pSelectedImpl = MultiChangeBroadcaster_Impl::create(rLocalNotifier, *itNodes, aSelectedChanges);
                    if (pSelectedImpl)
                        aNodecasters.push_back(pSelectedImpl);
                }
            }

            if (aNodecasters.empty())
                return 0;

            else if (aNodecasters.size() == 1)
                return aNodecasters.begin()->getBodyPtr();

            else
                return new SingleTreeBroadcaster_Impl(rRootNotifier, aNodecasters);
        }
    // -----------------------------------------------------------------------------------------------
        void SingleTreeBroadcaster_Impl::doQueryConstraints(configuration::NodeChangesInformation const& aChanges)
        {
            for(std::vector< vos::ORef<NodeLocalBroadcaster_Impl> >::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                configuration::NodeChangesInformation aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getAffectedNodeID()) )
                    (*it)->queryConstraints(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
        void SingleTreeBroadcaster_Impl::doNotifyListeners(configuration::NodeChangesInformation const& aChanges)
        {
            for(std::vector< vos::ORef<NodeLocalBroadcaster_Impl> >::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                configuration::NodeChangesInformation aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getAffectedNodeID()) )
                    (*it)->notifyListeners(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
    // MultiTreeBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------
        MultiTreeBroadcaster_Impl::MultiTreeBroadcaster_Impl(NotifierSet::value_type const& aRootSelector, std::vector< vos::ORef< Broadcaster::Impl > >& aBroadcasters)
        : Broadcaster::Impl(aRootSelector)
        , m_aBroadcasters()
        {
            m_aBroadcasters.swap(aBroadcasters);
        }

    // -----------------------------------------------------------------------------------------------

        bool MultiTreeBroadcaster_Impl::selectChanges(configuration::NodeChangesInformation& rSelected, configuration::NodeChangesInformation const& aOriginal, NotifierSet::value_type const& aSelector)
        {
            OSL_ASSERT(aSelector.first.isValid());
            OSL_ASSERT(aSelector.second.is());

            OSL_ASSERT(rSelected.empty()); // nothing in there yet

            rtl::Reference< configuration::Tree > const aSelectedTree( aSelector.second->getTree() );

            for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if (it->location.getAffectedTreeRef() == aSelectedTree)
                {
                    rSelected.push_back(*it);
                }
            }
            return !rSelected.empty();
        }
        // -------------------------------------------------------------------------------------------

        vos::ORef< Broadcaster::Impl > MultiTreeBroadcaster_Impl::create(NotifierSet::value_type const& rRootNotifier, NotifierSet const& rNotifiers, configuration::NodeChangesInformation const& aChanges)
        {
            std::vector< vos::ORef< Broadcaster::Impl > > aTreecasters;
            for (NotifierSet::const_iterator it = rNotifiers.begin(); it != rNotifiers.end(); ++it)
            {
                configuration::NodeChangesInformation aSelectedChanges;
                if ( selectChanges(aSelectedChanges, aChanges, *it))
                {
                    vos::ORef< Broadcaster::Impl > pSelectedImpl = SingleTreeBroadcaster_Impl::create(rRootNotifier, *it, aSelectedChanges);
                    if (pSelectedImpl.isValid())
                        aTreecasters.push_back(pSelectedImpl);
                }
            }

            if (aTreecasters.empty())
                return 0;

            else if (aTreecasters.size() == 1)
                return *aTreecasters.begin();

            else
                return new MultiTreeBroadcaster_Impl(rRootNotifier, aTreecasters);
        }
        // -------------------------------------------------------------------------------------------

        void MultiTreeBroadcaster_Impl::doQueryConstraints(configuration::NodeChangesInformation const& aChanges)
        {
            for(std::vector< vos::ORef< Broadcaster::Impl > >::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                configuration::NodeChangesInformation aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getNotifierData()) )
                    (*it)->queryConstraints(aSelectedInfos);
            }
        }
        // -------------------------------------------------------------------------------------------

        void MultiTreeBroadcaster_Impl::doNotifyListeners(configuration::NodeChangesInformation const& aChanges)
        {
            for(std::vector< vos::ORef< Broadcaster::Impl > >::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                configuration::NodeChangesInformation aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getNotifierData()) )
                    (*it)->notifyListeners(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------------------------------

    vos::ORef< Broadcaster::Impl > Broadcaster::Impl::create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChange const& aChange, bool bLocal)
    {
        OSL_ASSERT(pTreeImpl.is());

        vos::ORef< Broadcaster::Impl > pRet;

        configuration::NodeChangeLocation aLocation;
        if (aChange.getChangeLocation(aLocation))
        {
            if (bLocal)
            {
                pRet = SingleChangeBroadcaster_Impl::create( NotifierSet::value_type(rNotifierImpl,pTreeImpl), aLocation);
            }
            else
            {
                NotifierSet::value_type aAffectedNotifier( findNotifier(aLocation, pTreeImpl) );
                if (aAffectedNotifier.second.is()) // only if we found a notifier we are able to create a broadcaster (DG)
                    pRet = SingleChangeBroadcaster_Impl::create( aAffectedNotifier, aLocation);
            }
        }
        else
        {
            OSL_ENSURE(false, "Invalid change location set in node change - cannot broadcast");
            // can't create a matching change - must still create an empty one
        }

        if (pRet.isEmpty())
            pRet = EmptyBroadcaster_Impl::create( NotifierSet::value_type(rNotifierImpl,pTreeImpl) );

        return pRet;
    }
// ---------------------------------------------------------------------------------------------------

    vos::ORef< Broadcaster::Impl > Broadcaster::Impl::create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChanges const& aChanges, bool bLocal)
    {
        NotifierSet::value_type aRootData(rNotifierImpl, pTreeImpl);

        configuration::NodeChangesInformation aChangeInfos;
        if (aChanges.getChangesInfos(aChangeInfos))
        {
            return create(rNotifierImpl,pTreeImpl,aChangeInfos,bLocal);
        }
        else
        {
            OSL_ENSURE(aChanges.isEmpty(), "Cannot get information for changes - cannot notify");

            // make an empty one below
            vos::ORef< Broadcaster::Impl > pRet = EmptyBroadcaster_Impl::create( aRootData );

            return pRet;
        }

    }
// ---------------------------------------------------------------------------------------------------

    vos::ORef< Broadcaster::Impl > Broadcaster::Impl::create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChangeInformation const& aChange, bool bLocal)
    {
        OSL_ASSERT(pTreeImpl.is());

        vos::ORef< Broadcaster::Impl > pRet;

        if (aChange.hasValidLocation())
        {
            if (bLocal)
            {
                pRet = SingleChangeBroadcaster_Impl::create( NotifierSet::value_type(rNotifierImpl,pTreeImpl), aChange.location);
            }
            else
            {
                NotifierSet::value_type aAffectedNotifier( findNotifier(aChange.location, pTreeImpl) );
                if (aAffectedNotifier.second.is()) // only if we found a notifier we are able to create a broadcaster (DG)
                    pRet = SingleChangeBroadcaster_Impl::create( aAffectedNotifier, aChange.location);
            }
        }
        else
        {
            OSL_ENSURE(false, "Invalid change location set in node change - cannot broadcast");
            // can't create a matching change - must still create an empty one
        }

        if (pRet.isEmpty())
            pRet = EmptyBroadcaster_Impl::create( NotifierSet::value_type(rNotifierImpl,pTreeImpl) );

        return pRet;
    }
// ---------------------------------------------------------------------------------------------------

    vos::ORef< Broadcaster::Impl > Broadcaster::Impl::create(vos::ORef<NotifierImpl> const& rNotifierImpl, ApiTreeRef const& pTreeImpl, configuration::NodeChangesInformation const& aChanges, bool bLocal)
    {
        vos::ORef< Broadcaster::Impl > pRet;

        NotifierSet::value_type aRootData(rNotifierImpl, pTreeImpl);

        if (aChanges.size() == 1)
        {
            pRet = create(rNotifierImpl, pTreeImpl, *aChanges.begin(), bLocal);
        }
        else if (bLocal)
        {
            pRet = MultiChangeBroadcaster_Impl::create( aRootData, aChanges);
        }
        else
        {
            NotifierSet aNotifiers;
            findNotifiers( aNotifiers, aChanges, pTreeImpl);

            if (aNotifiers.size() > 1)
            {
                pRet = MultiTreeBroadcaster_Impl::create(aRootData, aNotifiers, aChanges);
            }
            else if (!aNotifiers.empty())
            {
                pRet = SingleTreeBroadcaster_Impl::create(aRootData, *aNotifiers.begin(), aChanges);
            }
            // else: empty
        }

        if (pRet.isEmpty())
            pRet = EmptyBroadcaster_Impl::create( aRootData );

        return pRet;
    }
// ---------------------------------------------------------------------------------------------------

       bool Broadcaster::Impl::translateChanges(configuration::NodeChangesInformation& _rInfos, configuration::NodeChanges const& aChanges, bool /*bSingleBase*/) const
    {
        rtl::Reference< configuration::Tree > aBaseTree = m_aNotifierData.second->getTree();
        Factory& rFactory = m_aNotifierData.second->getFactory();

        configuration::NodeChangesInformation aRawInfos;

        sal_uInt32 nChanges = aChanges.getChangesInfos(aRawInfos);

        OSL_ENSURE(nChanges, "Cannot get info(s) for change - skipping for notification");
        OSL_ENSURE(nChanges == aRawInfos.size(), "Incorrect change count returned");

        configuration::NodeChangesInformation aNewInfos;
        aNewInfos.reserve(nChanges);

        for (std::vector< configuration::NodeChangeInformation >::const_iterator pos = aRawInfos.begin(); pos != aRawInfos.end(); ++pos)
        {
            configuration::NodeChangeInformation aInfo = *pos;
            if( !configapi::rebaseChange(aInfo.location,aBaseTree) )
            {
                OSL_TRACE("Change is not within expected tree - skipping for notification");
                continue;
            }

            OSL_ENSURE(!pos->isEmptyChange(), "Empty Change Found for Notification");
            // it actually is expected that elements may not be found - thus ignoring result
            configapi::resolveToUno(aInfo.change, rFactory);

            aNewInfos.push_back( aInfo );
        }

        aNewInfos.swap(_rInfos);

        return !_rInfos.empty();
    }

// ---------------------------------------------------------------------------------------------------

       bool Broadcaster::Impl::translateChanges(configuration::NodeChangesInformation& aInfos, configuration::NodeChangesInformation const& aChanges, bool /*bSingleBase*/) const
    {
        configuration::NodeChangesInformation aNewInfos;
        aNewInfos.reserve( aChanges.size() );

        rtl::Reference< configuration::Tree > aBaseTree = m_aNotifierData.second->getTree();
        Factory& rFactory = m_aNotifierData.second->getFactory();

        for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
        {
            configuration::NodeChangeInformation aInfo(*it);
            if( !configapi::rebaseChange(aInfo.location,aBaseTree) )
            {
                OSL_TRACE("Change is not within expected tree - skipping for notification");
                continue;
            }

            if( !configapi::resolveToUno(aInfo.change,rFactory) )
            {
                // it actually is expected that elements may not be found
                // OSL_TRACE("Cannot find affected elements of Change");
            }

            aNewInfos.push_back( aInfo );
        }

        aNewInfos.swap(aInfos);
        return !aInfos.empty();
    }

// ---------------------------------------------------------------------------------------------------
    void Broadcaster::Impl::notifyRootListeners(configuration::NodeChangesInformation const& aChanges)
    {
        if (aChanges.empty()) return;

        ApiTreeRef pRootTree( m_aNotifierData.second->getRootTreeImpl() );
        if (pRootTree.is())
        {
            vos::ORef<NotifierImpl> aRootNotifier = BroadcasterHelper::getImpl(pRootTree->getNotifier());
            if (aRootNotifier.isValid())
            {
                uno::Reference< css::util::XChangesListener > const * const pSelect = 0;

                configuration::NodeID aNotifiedNode = makeRootID( pRootTree );

                if (cppu::OInterfaceContainerHelper* pContainer = aRootNotifier->m_aListeners.getContainer(aNotifiedNode.toIndex(), ::getCppuType(pSelect)) )
                {
                    css::util::ChangesEvent aEvent;
                    aEvent.Source = pRootTree->getUnoInstance();

                    uno::Reference<uno::XInterface> xBaseInstance = m_aNotifierData.second->getUnoInstance();
                    aEvent.Base <<= xBaseInstance;

                    // translate and collect the changes
                    aEvent.Changes.realloc(aChanges.size());
                    css::util::ElementChange* pChange = aEvent.Changes.getArray();

                    for (std::vector< configuration::NodeChangeInformation >::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
                    {
                        fillChangeFromResolved(*pChange, *it);
                        ++pChange;
                    }

                    // now notify
                    ListenerContainerIterator< css::util::XChangesListener > aIter(*pContainer);

                    UnoApiLockReleaser aGuardReleaser;
                    while (aIter.hasMoreElements())
                        try { aIter.next()->changesOccurred(aEvent); } catch (uno::Exception & ) {}
                }
            }
        }
    }

// ---------------------------------------------------------------------------------------------------
// class Broadcaster
// ---------------------------------------------------------------------------------------------------
Broadcaster::Broadcaster(Notifier const& aNotifier, configuration::NodeChange const& aChange, bool bLocal)
: m_pImpl( Impl::create(aNotifier.m_aImpl,ApiTreeRef(aNotifier.m_pTree),aChange,bLocal) )
{
    OSL_ASSERT(m_pImpl.isValid());
}
// ---------------------------------------------------------------------------------------------------
Broadcaster::Broadcaster(Notifier const& aNotifier, configuration::NodeChanges const& aChanges, bool bLocal)
: m_pImpl( Impl::create(aNotifier.m_aImpl,ApiTreeRef(aNotifier.m_pTree),aChanges,bLocal) )
{
    OSL_ASSERT(m_pImpl.isValid());
}
// ---------------------------------------------------------------------------------------------------
Broadcaster::Broadcaster(Notifier const& aNotifier, configuration::NodeChangesInformation const& aChanges, bool bLocal)
: m_pImpl( Impl::create(aNotifier.m_aImpl,ApiTreeRef(aNotifier.m_pTree),aChanges,bLocal) )
{
    OSL_ASSERT(m_pImpl.isValid());
}
// ---------------------------------------------------------------------------------------------------

Broadcaster::Broadcaster(Broadcaster const& aOther)
: m_pImpl(aOther.m_pImpl)
{
    OSL_ASSERT(m_pImpl.isValid());
}
// ---------------------------------------------------------------------------------------------------

Broadcaster::~Broadcaster()
{
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::queryConstraints(configuration::NodeChange const& aChange) throw(beans::PropertyVetoException)
{
    OSL_ENSURE(aChange.isChange(),"Constraint query without a change !");

    configuration::NodeChanges aChanges;
    aChanges.add(aChange);
    this->queryConstraints(aChanges,true);
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::queryConstraints(configuration::NodeChanges const& aChanges, bool bSingleBase) throw(beans::PropertyVetoException)
{
    OSL_ENSURE(!aChanges.isEmpty(),"Constraint query without a change !");

    try
    {
        configuration::NodeChangesInformation aInfos;
        if (m_pImpl->translateChanges(aInfos,aChanges,bSingleBase))
        {
            m_pImpl->queryConstraints(aInfos);
        }
    }
    catch (beans::PropertyVetoException & )
    {
        throw;
    }
    catch (uno::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected UNO exception in notifyListeners");
    }
    catch (configuration::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected internal exception in notifyListeners");
    }
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(configuration::NodeChange const& aChange) throw()
{
    OSL_ENSURE(aChange.isChange(),"Notifying without a change !");

    configuration::NodeChanges aChanges;
    aChanges.add(aChange);
    this->notifyListeners(aChanges, true);
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(configuration::NodeChanges const& aChanges, bool bSingleBase) throw()
{
    OSL_ENSURE(!aChanges.isEmpty(),"Notifying without a change !");

    try
    {
        configuration::NodeChangesInformation aInfos;
        if (m_pImpl->translateChanges(aInfos,aChanges, bSingleBase))
        {
            m_pImpl->notifyListeners(aInfos);
            m_pImpl->notifyRootListeners(aInfos);
        }
    }
    catch (uno::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected UNO exception in notifyListeners");
    }
    catch (configuration::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected internal exception in notifyListeners");
    }
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(configuration::NodeChangesInformation const& aChanges, bool bSingleBase) throw()
{
    OSL_ENSURE(!aChanges.empty(),"Notifying without a change !");

    try
    {
        configuration::NodeChangesInformation aInfos;
        if (m_pImpl->translateChanges(aInfos,aChanges, bSingleBase))
        {
            m_pImpl->notifyListeners(aInfos);
            m_pImpl->notifyRootListeners(aInfos);
        }
    }
    catch (uno::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected UNO exception in notifyListeners");
    }
    catch (configuration::Exception & )
    {
        OSL_ENSURE(false, "configmgr::Broadcaster: Unexpected internal exception in notifyListeners");
    }
}
// ---------------------------------------------------------------------------------------------------
    }
}
