/*************************************************************************
 *
 *  $RCSfile: broadcaster.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-20 01:38:18 $
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

#include <stl/map>
#include <stl/set>
#include <stl/functional>

namespace configmgr
{
    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------

        using configuration::Tree;
        using configuration::NodeID;

// ---------------------------------------------------------------------------------------------------
// Broadcaster implementation
// ---------------------------------------------------------------------------------------------------
    class BroadcasterHelper
    {
    public:
        static NotifierHolder getImpl(Notifier const& aNotifier) { return aNotifier.m_aImpl; }
    };
// ---------------------------------------------------------------------------------------------------

    namespace
    {
    // -----------------------------------------------------------------------------------------------
        template <class T>
        struct LessORefBodyPtr
        {
            typedef vos::ORef<T> Ref;
            bool operator()(Ref const& lhs, Ref const& rhs)
            {
                return ptr_less(lhs.getBodyPtr(), rhs.getBodyPtr());
            }

            std::less<T*> ptr_less;
        };
        typedef std::map< NotifierHolder, ApiTreeImpl const*, LessORefBodyPtr<NotifierImpl> > NotifierSet;
        typedef NotifierSet::value_type NotifierData;
    // -----------------------------------------------------------------------------------------------
        struct ChangeData
        {
        // --------------------------------------------------------------------
            typedef configuration::NodeChange               Change;
            typedef configuration::NodeChangeInfo           Info;
            typedef configuration::NodeChangeInfo::Type     ChangeType;
            typedef configuration::ExtendedNodeChangeInfo   ExtendedInfo;
        // --------------------------------------------------------------------

            Change          change;
            ExtendedInfo    info;
        // --------------------------------------------------------------------

            ChangeData()
            : change()
            , info()
            {}
            explicit
        // --------------------------------------------------------------------
            ChangeData(NodeChange const& aNodeChange)
            : change(aNodeChange)
            , info()
            {
                change.getChangeInfo(info);
            }
        // --------------------------------------------------------------------
            bool fill(NodeChange const& aNodeChange)
            {
                change = aNodeChange;
                return aNodeChange.getChangeInfo(info);
            }
        // --------------------------------------------------------------------
            bool isChange()         const { return info.change.isChange() || change.isChange(); }

            bool isEmpty()          const { return info.change.isEmpty(); }
            bool isValueChange()    const { return info.change.isValueChange(); }
            bool isSetChange()      const { return info.change.isSetChange(); }
        // --------------------------------------------------------------------

            ChangeType getType()    const { return info.change.type; }

            UnoAny getOldValue()    const { return info.change.oldValue; }
            UnoAny getNewValue()    const { return info.change.newValue; }

        // --------------------------------------------------------------------
            bool resolveObjects(configapi::Factory& rFactory) { return configapi::resolveToUno(info.change, rFactory); }
            bool rebase(configuration::Tree const& aBaseTree) { return configapi::rebaseChange(info, aBaseTree); }

        };
    // -----------------------------------------------------------------------------------------------
        typedef std::vector<ChangeData> ChangesInfos;
    // -----------------------------------------------------------------------------------------------
    }

// ---------------------------------------------------------------------------------------------------
// class Broadcaster::Impl
// ---------------------------------------------------------------------------------------------------
    class Broadcaster::Impl : public vos::OReference
    {
    private:
        NotifierData m_aNotifierData;
    public:
        Impl(NotifierData const& aNotifierData) : m_aNotifierData(aNotifierData) {}

        NotifierData getNotifierData() const { return m_aNotifierData; }

        bool translateChanges(ChangesInfos& aInfos, NodeChanges const& aChanges, bool bSingleBase) const;

        void queryConstraints(ChangesInfos const& aChanges) { this->doQueryConstraints(aChanges); }
        void notifyListeners(ChangesInfos const& aChanges)  { this->doNotifyListeners(aChanges); }

        void notifyRootListeners(ChangesInfos const& aChanges);

        static vos::ORef<Impl> create(NotifierHolder const& rNotifierImpl, ApiTreeImpl const* pTreeImpl, NodeChange const& aChange, bool bLocal);
        static vos::ORef<Impl> create(NotifierHolder const& rNotifierImpl, ApiTreeImpl const* pTreeImpl, NodeChanges const& aChange, bool bLocal);

    private:
        virtual void doQueryConstraints(ChangesInfos const& aChanges) = 0;
        virtual void doNotifyListeners(ChangesInfos const& aChanges) = 0;
    };
// ---------------------------------------------------------------------------------------------------
    namespace
    {
    // -----------------------------------------------------------------------------------------------
        using configuration::Tree;
        using configuration::NodeRef;
        using configuration::NodeID;
        using configuration::NodeOffset;
        using configuration::NodeChange;
        using configuration::NodeChangeInfo;

        typedef std::set< configuration::NodeID > NodeSet;
    // -----------------------------------------------------------------------------------------------
        typedef vos::ORef< Broadcaster::Impl > BroadcasterImplRef;
    // -----------------------------------------------------------------------------------------------

        class EmptyBroadcaster_Impl : public Broadcaster::Impl
        {
            EmptyBroadcaster_Impl(NotifierData const& rNotifierData)
            : Broadcaster::Impl(rNotifierData)
            {
            }
        public:
            static
            BroadcasterImplRef create(NotifierData const& rRootNotifier)
            {
                return new EmptyBroadcaster_Impl(rRootNotifier);
            }
        private:
            virtual void doQueryConstraints(ChangesInfos const& aChanges);
            virtual void doNotifyListeners(ChangesInfos const& aChanges);
        };

        void EmptyBroadcaster_Impl::doQueryConstraints(ChangesInfos const&) {}
        void EmptyBroadcaster_Impl::doNotifyListeners(ChangesInfos const&) {}
    // -----------------------------------------------------------------------------------------------

        class NodeLocalBroadcaster_Impl : public Broadcaster::Impl
        {
            NodeID      aAffectedNode;

        public:
            NodeLocalBroadcaster_Impl(NotifierData const& rTreeNotifierData, NodeID const& aAffectedID)
            : Broadcaster::Impl(rTreeNotifierData)
            , aAffectedNode(aAffectedID)
            {
            }

            NodeID getAffectedNodeID() const { return aAffectedNode; }
            NodeOffset getNodeIndex() const { return aAffectedNode.toIndex(); }

        protected:
            void querySingleConstraint(ChangeData const& aChange, bool bMore);
            void notifySingleChange(ChangeData const& aChange, bool bMore, css::beans::PropertyChangeEvent*& pCurEvent);
        };
    // -----------------------------------------------------------------------------------------------

        class SingleChangeBroadcaster_Impl : public NodeLocalBroadcaster_Impl
        {
            NodeID m_aChangingNode;

            SingleChangeBroadcaster_Impl(NotifierData const& rTreeNotifierData, NodeID const& aAffectedID, NodeID const& aChangedNode);

        public:
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierData const& rLocalNotifier,
                        NodeChange const& aChange);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierData const& rLocalNotifier,
                        NodeID const& aAffectedID,
                        NodeChange const& aChange);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierData const& rLocalNotifier,
                        NodeID const& aAffectedID,
                        NodeID const& aChangedNode,
                        NodeChange const& aChange);
        private:
            virtual void doQueryConstraints(ChangesInfos const& aChanges);
            virtual void doNotifyListeners(ChangesInfos const& aChanges);
        };

    // -----------------------------------------------------------------------------------------------
        class MultiChangeBroadcaster_Impl : public NodeLocalBroadcaster_Impl
        {
            NodeSet m_aChangingNodes;

            MultiChangeBroadcaster_Impl(NotifierData const& rTreeNotifierData, NodeID const& aAffectedID, NodeSet& aChangedNodes);

        public:
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierData const& rLocalNotifier,
                        NodeChanges const& aChanges);
            static
            NodeLocalBroadcaster_Impl* create(
                        NotifierData const& rLocalNotifier,
                        NodeID const& aAffectedID,
                        NodeChanges const& aChanges);
        private:
            virtual void doQueryConstraints(ChangesInfos const& aChanges);
            virtual void doNotifyListeners(ChangesInfos const& aChanges);
        };
    // -----------------------------------------------------------------------------------------------
        class SingleTreeBroadcaster_Impl : public Broadcaster::Impl
        {
            typedef std::vector< vos::ORef<NodeLocalBroadcaster_Impl> > BroadcasterList;
            BroadcasterList m_aBroadcasters;

            SingleTreeBroadcaster_Impl(NotifierData const& rTreeNotifierData, BroadcasterList& aBroadcasters);

        public:
            static
            BroadcasterImplRef create(
                        NotifierData const& rRootNotifier,
                        NotifierData const& rLocalNotifier,
                        NodeChanges const& aChanges);

            static bool selectChanges(NodeChanges& rSelected, NodeChanges const& aOriginal, NodeID const& aSelector);
            static bool selectChanges(ChangesInfos& rSelected, ChangesInfos const& aOriginal, NodeID const& aSelector);
        private:
            virtual void doQueryConstraints(ChangesInfos const& aChanges);
            virtual void doNotifyListeners(ChangesInfos const& aChanges);
        };
    // -----------------------------------------------------------------------------------------------
        class MultiTreeBroadcaster_Impl : public Broadcaster::Impl
        {
            typedef std::vector< BroadcasterImplRef > BroadcasterList;
            BroadcasterList m_aBroadcasters;

            MultiTreeBroadcaster_Impl(NotifierData const& rRootNotifierData, BroadcasterList& aBroadcasters);
        public:
            static
            BroadcasterImplRef create(
                        NotifierData const& rRootNotifier,
                        NotifierSet const& rNotifiers,
                        NodeChanges const& aChanges);

            static bool selectChanges(NodeChanges& rSelected, NodeChanges const& aOriginal, NotifierData const& aSelector);
            static bool selectChanges(ChangesInfos& rSelected, ChangesInfos const& aOriginal, NotifierData const& aSelector);
        private:
            virtual void doQueryConstraints(ChangesInfos const& aChanges);
            virtual void doNotifyListeners(ChangesInfos const& aChanges);
        };

    // -----------------------------------------------------------------------------------------------

        inline NodeID makeRootID( Tree const& aTree ) { return NodeID( aTree, aTree.getRootNode() ); }
        inline NodeID makeRootID( ApiTreeImpl const* pTreeImpl ) { return makeRootID( pTreeImpl->getTree() ); }
    // -----------------------------------------------------------------------------------------------
        NotifierData findNotifier(NodeChange const& aChange, ApiTreeImpl const* pTreeImpl)
        {
            NodeID aAffectedNode = aChange.getAffectedNodeID();

            if (ApiTreeImpl const* pAffectedImpl = Factory::findDescendantTreeImpl(aAffectedNode, pTreeImpl))
            {
                NotifierHolder aAffectedNotifier = BroadcasterHelper::getImpl(pAffectedImpl->getNotifier());

                return  NotifierData(aAffectedNotifier, pAffectedImpl);
            }
            else
                return NotifierData();
        }
    // -----------------------------------------------------------------------------------------------

        void findNotifiers(NotifierSet& aNotifiers, NodeChanges const& aChanges, ApiTreeImpl const* pTreeImpl )
        {
            for (NodeChanges::Iterator it = aChanges.begin(); it != aChanges.end(); ++it)
            {
                NotifierData aNotifierData( findNotifier(*it,pTreeImpl) );

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
        void NodeLocalBroadcaster_Impl::querySingleConstraint(ChangeData const& aChange, bool bMore)
        {
            using css::beans::XVetoableChangeListener;

            typedef ListenerContainerIterator< XVetoableChangeListener > ListenerIterator;
            uno::Reference< XVetoableChangeListener > const * const SelectListener = 0;

            NotifierImplHolder pNotifierImpl = getNotifierData().first;

            ListenerContainer* pListeners   = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );
            ListenerContainer* pSpecial     = pNotifierImpl->m_aListeners.getSpecialContainer( aChange.change.getChangedNodeID() );

            if (pSpecial || pListeners)
            {
                css::beans::PropertyChangeEvent aEvent;
                aEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

                if (configapi::fillEventDataFromResolved(aEvent,aChange.info,bMore))
                {

                    if (pListeners)
                    {
                        ListenerIterator aIterator(*pListeners);
                        while (aIterator.hasMoreElements())
                            aIterator.next()->vetoableChange(aEvent);
                    }
                    if (pSpecial)
                    {
                        ListenerIterator aIterator(*pSpecial);
                        while (aIterator.hasMoreElements())
                            aIterator.next()->vetoableChange(aEvent);
                    }
                }
            }

        }
    // -----------------------------------------------------------------------------------------------
        void NodeLocalBroadcaster_Impl::notifySingleChange(ChangeData const& aChange, bool bMore, css::beans::PropertyChangeEvent*& pCurEvent)
        {
            using css::beans::XPropertyChangeListener;
            using css::container::XContainerListener;

            typedef ListenerContainerIterator< XPropertyChangeListener >    PropertyListenerIterator;
            typedef ListenerContainerIterator< XContainerListener >         ContainerListenerIterator;
            uno::Reference< XPropertyChangeListener > const * const     SelectPropertyListener = 0;
            uno::Reference< XContainerListener > const * const          SelectContainerListener = 0;

            NotifierImplHolder pNotifierImpl = getNotifierData().first;

            ListenerContainer* pContainerListeners  = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectContainerListener) );

            if (pContainerListeners)
            {
                css::container::ContainerEvent aEvent;
                aEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

                if (configapi::fillEventDataFromResolved(aEvent,aChange.info))
                {

                    ContainerListenerIterator aIterator(*pContainerListeners);

                    while (aIterator.hasMoreElements())
                    {
                        uno::Reference<XContainerListener> xListener( aIterator.next() );
                        OSL_ASSERT( xListener.is() );

                        switch (aChange.getType())
                        {
                        case NodeChangeInfo::eSetValue:
                        case NodeChangeInfo::eSetDefault:
                        case NodeChangeInfo::eReplaceElement:
                            xListener->elementReplaced(aEvent);
                            break;

                        case NodeChangeInfo::eInsertElement:
                            xListener->elementInserted(aEvent);
                            break;

                        case NodeChangeInfo::eRemoveElement:
                            xListener->elementRemoved(aEvent);
                            break;


                        case NodeChangeInfo::eRenameElementTree:
                        case NodeChangeInfo::eNoChange:
                            OSL_ASSERT(false);
                            break;
                        }
                    }
                }
            }


            OSL_ASSERT(pCurEvent);
            css::beans::PropertyChangeEvent& rEvent = *pCurEvent;

            rEvent.Source = pNotifierImpl->m_aListeners.getObjectAt( getNodeIndex() );

            if (configapi::fillEventDataFromResolved(rEvent,aChange.info,bMore))
            {
                ListenerContainer* pPropertyListeners   = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectPropertyListener) );
                if (pPropertyListeners)
                {
                    PropertyListenerIterator aIterator(*pPropertyListeners);
                    while (aIterator.hasMoreElements())
                        aIterator.next()->propertyChange(rEvent);
                }

                ListenerContainer* pSpecialListeners    = pNotifierImpl->m_aListeners.getSpecialContainer( aChange.change.getChangedNodeID() );
                if (pSpecialListeners)
                {
                    PropertyListenerIterator aIterator(*pSpecialListeners);
                    while (aIterator.hasMoreElements())
                        aIterator.next()->propertyChange(rEvent);
                }

                ++pCurEvent;
            }

        }
    // -----------------------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------------------
    // SingleBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------
        SingleChangeBroadcaster_Impl::SingleChangeBroadcaster_Impl(
            NotifierData const& rTreeNotifierData,
            NodeID const& aAffectedID, NodeID const& aChangedNode
        )
        : NodeLocalBroadcaster_Impl(rTreeNotifierData,aAffectedID)
        , m_aChangingNode(aChangedNode)
        {
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierData const& rLocalNotifier,
                    NodeChange const& aChange)
        {
            OSL_ENSURE(configuration::equalTree(rLocalNotifier.second->getTree(), aChange.getAffectedTree()),
                        "ERROR: Tree Mismatch creating Single Broadcaster");

            return create(rLocalNotifier,aChange.getAffectedNodeID(),aChange.getChangedNodeID(),aChange);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierData const& rLocalNotifier,
                    NodeID const& aAffectedID,
                    NodeChange const& aChange)
        {

            return create(rLocalNotifier,aAffectedID,aChange.getChangedNodeID(),aChange);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* SingleChangeBroadcaster_Impl::create(
                    NotifierData const& rLocalNotifier,
                    NodeID const& aAffectedID,
                    NodeID const& aChangedNodeID,
                    NodeChange const& aChange)
        {
            OSL_ENSURE(configuration::equalTree(rLocalNotifier.second->getTree(), aChange.getAffectedTree()),
                        "ERROR: Tree Mismatch creating Single Broadcaster");
            OSL_ENSURE( aChange.getAffectedNodeID() == aAffectedID,
                        "ERROR: Node Mismatch creating Single Broadcaster");
            OSL_ENSURE( aChange.getChangedNodeID() == aChangedNodeID,
                        "ERROR: Node Mismatch creating Single Broadcaster");

            return new SingleChangeBroadcaster_Impl(rLocalNotifier,aAffectedID,aChangedNodeID);
        }
    // -----------------------------------------------------------------------------------------------
        void SingleChangeBroadcaster_Impl::doQueryConstraints(ChangesInfos const& aChanges)
        {
            OSL_ASSERT(aChanges.size() <= 1);
            if (!aChanges.empty())
            {
                ChangesInfos::const_iterator it = aChanges.begin();

                OSL_ENSURE( m_aChangingNode == it->change.getChangedNodeID(), "Broadcasting unanticipated change");

                querySingleConstraint(*it, false);
            }

        }
    // -----------------------------------------------------------------------------------------------
        void SingleChangeBroadcaster_Impl::doNotifyListeners(ChangesInfos const& aChanges)
        {
            using css::beans::XPropertiesChangeListener;
            using css::beans::PropertyChangeEvent;


            OSL_ASSERT(aChanges.size() <= 1);
            if (!aChanges.empty())
            {

                PropertyChangeEvent aEvent;
                PropertyChangeEvent * pEventNext = &aEvent;

                ChangesInfos::const_iterator it = aChanges.begin();

                OSL_ENSURE( m_aChangingNode == it->change.getChangedNodeID(), "Broadcasting unanticipated change");

                notifySingleChange(*it, false, pEventNext);

                if (pEventNext != &aEvent)
                {
                    uno::Sequence< PropertyChangeEvent > aPropertyEvents(&aEvent,1);

                    typedef ListenerContainerIterator< XPropertiesChangeListener > ListenerIterator;
                    uno::Reference< XPropertiesChangeListener > const * const SelectListener = 0;

                    NotifierImplHolder pNotifierImpl = getNotifierData().first;

                    ListenerContainer* pContainer = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );

                    if (pContainer)
                    {
                        ListenerIterator aIterator(*pContainer);
                        while (aIterator.hasMoreElements())
                            aIterator.next()->propertiesChange(aPropertyEvents);
                    }
                }
            }
        }

    // -----------------------------------------------------------------------------------------------
    // MultiChangeBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------

        MultiChangeBroadcaster_Impl::MultiChangeBroadcaster_Impl(
            NotifierData const& rTreeNotifierData,
            NodeID const& aAffectedID, NodeSet& aChangedNodes
        )
        : NodeLocalBroadcaster_Impl(rTreeNotifierData,aAffectedID)
        , m_aChangingNodes()
        {
            m_aChangingNodes.swap(aChangedNodes);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* MultiChangeBroadcaster_Impl::create(
                    NotifierData const& rLocalNotifier,
                    NodeChanges const& aChanges)
        {
            if (aChanges.isEmpty())
                return 0;

            NodeID aAffectedNodeID = aChanges.begin()->getAffectedNodeID();

            return create(rLocalNotifier, aAffectedNodeID, aChanges);
        }
    // -----------------------------------------------------------------------------------------------
        NodeLocalBroadcaster_Impl* MultiChangeBroadcaster_Impl::create(
                    NotifierData const& rLocalNotifier,
                    NodeID const& aAffectedNodeID,
                    NodeChanges const& aChanges)
        {
            if (aChanges.isEmpty())
                return 0;

            else if (aChanges.getCount() == 1)
                return SingleChangeBroadcaster_Impl::create(rLocalNotifier,aAffectedNodeID,*aChanges.begin());

            else
            {
                NodeSet aChangedNodes;
                for (NodeChanges::Iterator it = aChanges.begin(); it != aChanges.end(); ++it)
                {
                    OSL_ENSURE(it->getAffectedNodeID() == aAffectedNodeID, "ERROR: Change is not local to affected node (as advertised)");
                    OSL_ENSURE(configuration::equalTree(rLocalNotifier.second->getTree(), it->getAffectedTree()),
                                "ERROR: Tree Mismatch creating Multi Change Broadcaster");

                    aChangedNodes.insert(it->getChangedNodeID());
                }
                OSL_ENSURE(!aChangedNodes.empty(), "Changes don't affect any nodes");

                if (aChangedNodes.size() == 1) OSL_TRACE("WARNING: Different changes all affect the same node !");

                return new MultiChangeBroadcaster_Impl(rLocalNotifier, aAffectedNodeID, aChangedNodes);
            }
        }
    // -----------------------------------------------------------------------------------------------
        void MultiChangeBroadcaster_Impl::doQueryConstraints(ChangesInfos const& aChanges)
        {
            ChangesInfos::const_iterator const stop = aChanges.end(), last = stop-1;

            for (ChangesInfos::const_iterator it = aChanges.begin(); it != stop; ++it)
            {
                OSL_ENSURE( m_aChangingNodes.find( it->change.getChangedNodeID() ) != m_aChangingNodes.end(), "Broadcasting unanticipated change");

                querySingleConstraint(*it, it != last);
            }

        }
    // -----------------------------------------------------------------------------------------------
        void MultiChangeBroadcaster_Impl::doNotifyListeners(ChangesInfos const& aChanges)
        {
            using css::beans::XPropertiesChangeListener;
            using css::beans::PropertyChangeEvent;

            uno::Sequence< PropertyChangeEvent > aPropertyEvents(aChanges.size());

            PropertyChangeEvent * const pEventStart = aPropertyEvents.getArray();
            PropertyChangeEvent * pEventNext = pEventStart;

            ChangesInfos::const_iterator const stop = aChanges.end(), last = stop-1;

            for (ChangesInfos::const_iterator it = aChanges.begin(); it != stop; ++it)
            {
                OSL_ENSURE( m_aChangingNodes.find( it->change.getChangedNodeID() ) != m_aChangingNodes.end(), "Broadcasting unanticipated change");

                notifySingleChange(*it, it != last, pEventNext);
            }

            sal_Int32 nPropertyEvents = pEventNext-pEventStart;

            if (nPropertyEvents > 0)
            {
                OSL_ASSERT(nPropertyEvents <= aPropertyEvents.getLength());
                if (nPropertyEvents != aPropertyEvents.getLength())
                    aPropertyEvents.realloc(nPropertyEvents);

                typedef ListenerContainerIterator< XPropertiesChangeListener > ListenerIterator;
                uno::Reference< XPropertiesChangeListener > const * const SelectListener = 0;

                NotifierImplHolder pNotifierImpl = getNotifierData().first;

                ListenerContainer* pContainer = pNotifierImpl->m_aListeners.getContainer( getNodeIndex(), getCppuType(SelectListener) );

                if (pContainer)
                {
                    ListenerIterator aIterator(*pContainer);
                    while (aIterator.hasMoreElements())
                        aIterator.next()->propertiesChange(aPropertyEvents);
                }
            }
        }
    // -----------------------------------------------------------------------------------------------
    // TreeLocalBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------

        SingleTreeBroadcaster_Impl::SingleTreeBroadcaster_Impl(
            NotifierData const& aTreeNotifierData,
            BroadcasterList& aBroadcasters
        )
        : Broadcaster::Impl(aTreeNotifierData)
        , m_aBroadcasters()
        {
            m_aBroadcasters.swap(aBroadcasters);
        }

    // -----------------------------------------------------------------------------------------------
        bool SingleTreeBroadcaster_Impl::selectChanges(NodeChanges& rSelected, NodeChanges const& aOriginal, NodeID const& aSelector)
        {
            OSL_ASSERT(rSelected.isEmpty()); // nothing in there yet

            for (NodeChanges::Iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if ( it->getAffectedNodeID() == aSelector )
                    rSelected.add(*it);
            }
            return !rSelected.isEmpty();
        }
        // -------------------------------------------------------------------------------------------

        bool SingleTreeBroadcaster_Impl::selectChanges(ChangesInfos& rSelected, ChangesInfos const& aOriginal, NodeID const& aSelector)
        {
            OSL_ASSERT(rSelected.empty()); // nothing in there yet

            for (ChangesInfos::const_iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if ( it->change.getAffectedNodeID() == aSelector )
                {
                    rSelected.push_back(*it);
                }
            }
            return !rSelected.empty();
        }
    // -----------------------------------------------------------------------------------------------
        BroadcasterImplRef SingleTreeBroadcaster_Impl::create(
                    NotifierData const& rRootNotifier,
                    NotifierData const& rLocalNotifier,
                    NodeChanges const& aChanges)
        {
            NodeSet aNodes;
            for (NodeChanges::Iterator itChanges = aChanges.begin(); itChanges != aChanges.end(); ++itChanges)
            {
                aNodes.insert(itChanges->getAffectedNodeID());
            }

            BroadcasterList aNodecasters;
            for (NodeSet::const_iterator itNodes = aNodes.begin(); itNodes != aNodes.end(); ++itNodes)
            {
                NodeChanges aSelectedChanges;
                if ( selectChanges(aSelectedChanges, aChanges, *itNodes))
                {
                    NodeLocalBroadcaster_Impl* pSelectedImpl = MultiChangeBroadcaster_Impl::create(rRootNotifier, *itNodes, aSelectedChanges);
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
        void SingleTreeBroadcaster_Impl::doQueryConstraints(ChangesInfos const& aChanges)
        {
            for(BroadcasterList::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                ChangesInfos aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getAffectedNodeID()) )
                    (*it)->queryConstraints(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
        void SingleTreeBroadcaster_Impl::doNotifyListeners(ChangesInfos const& aChanges)
        {
            for(BroadcasterList::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                ChangesInfos aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getAffectedNodeID()) )
                    (*it)->notifyListeners(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
    // MultiTreeBroadcaster_Impl
    // -----------------------------------------------------------------------------------------------
        MultiTreeBroadcaster_Impl::MultiTreeBroadcaster_Impl(NotifierData const& aRootSelector, BroadcasterList& aBroadcasters)
        : Broadcaster::Impl(aRootSelector)
        , m_aBroadcasters()
        {
            m_aBroadcasters.swap(aBroadcasters);
        }

    // -----------------------------------------------------------------------------------------------
        bool MultiTreeBroadcaster_Impl::selectChanges(NodeChanges& rSelected, NodeChanges const& aOriginal, NotifierData const& aSelector)
        {
            OSL_ASSERT(aSelector.first.isValid());
            OSL_ASSERT(aSelector.second != 0);
            OSL_ASSERT(rSelected.isEmpty()); // nothing in there yet

            Tree const aSelectedTree( aSelector.second->getTree() );

            for (NodeChanges::Iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if ( configuration::equalTree(it->getAffectedTree(),aSelectedTree) )
                    rSelected.add(*it);
            }
            return !rSelected.isEmpty();
        }
        // -------------------------------------------------------------------------------------------

        bool MultiTreeBroadcaster_Impl::selectChanges(ChangesInfos& rSelected, ChangesInfos const& aOriginal, NotifierData const& aSelector)
        {
            OSL_ASSERT(aSelector.first.isValid());
            OSL_ASSERT(aSelector.second != 0);
            OSL_ASSERT(rSelected.empty()); // nothing in there yet

            Tree const aSelectedTree( aSelector.second->getTree() );

            for (ChangesInfos::const_iterator it = aOriginal.begin(); it != aOriginal.end(); ++it)
            {
                if ( configuration::equalTree(it->change.getAffectedTree(),aSelectedTree) )
                {
                    rSelected.push_back(*it);
                }
            }
            return !rSelected.empty();
        }
        // -------------------------------------------------------------------------------------------

        BroadcasterImplRef MultiTreeBroadcaster_Impl::create(NotifierData const& rRootNotifier, NotifierSet const& rNotifiers, NodeChanges const& aChanges)
        {
            BroadcasterList aTreecasters;
            for (NotifierSet::const_iterator it = rNotifiers.begin(); it != rNotifiers.end(); ++it)
            {
                NodeChanges aSelectedChanges;
                if ( selectChanges(aSelectedChanges, aChanges, *it))
                {
                    BroadcasterImplRef pSelectedImpl = SingleTreeBroadcaster_Impl::create(rRootNotifier, *it, aSelectedChanges);
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

        void MultiTreeBroadcaster_Impl::doQueryConstraints(ChangesInfos const& aChanges)
        {
            for(BroadcasterList::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                ChangesInfos aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getNotifierData()) )
                    (*it)->queryConstraints(aSelectedInfos);
            }
        }
        // -------------------------------------------------------------------------------------------

        void MultiTreeBroadcaster_Impl::doNotifyListeners(ChangesInfos const& aChanges)
        {
            for(BroadcasterList::iterator it = m_aBroadcasters.begin(); it != m_aBroadcasters.end(); ++it)
            {
                ChangesInfos aSelectedInfos;
                if ( selectChanges(aSelectedInfos, aChanges, (*it)->getNotifierData()) )
                    (*it)->notifyListeners(aSelectedInfos);
            }
        }
    // -----------------------------------------------------------------------------------------------
    }
// ---------------------------------------------------------------------------------------------------

    BroadcasterImplRef Broadcaster::Impl::create(NotifierHolder const& rNotifierImpl, ApiTreeImpl const* pTreeImpl, NodeChange const& aChange, bool bLocal)
    {
        OSL_ASSERT(pTreeImpl);

        BroadcasterImplRef pRet;
        if (bLocal)
        {
            pRet = SingleChangeBroadcaster_Impl::create( NotifierData(rNotifierImpl,pTreeImpl), aChange);
        }
        else
        {
            NotifierData aAffectedNotifier( findNotifier(aChange, pTreeImpl) );
            pRet = SingleChangeBroadcaster_Impl::create( aAffectedNotifier, aChange);
        }

        if (pRet.isEmpty())
            pRet = EmptyBroadcaster_Impl::create( NotifierData(rNotifierImpl,pTreeImpl) );

        return pRet;
    }
// ---------------------------------------------------------------------------------------------------

    BroadcasterImplRef Broadcaster::Impl::create(NotifierHolder const& rNotifierImpl, ApiTreeImpl const* pTreeImpl, NodeChanges const& aChanges, bool bLocal)
    {
        BroadcasterImplRef pRet;

        NotifierData aRootData(rNotifierImpl, pTreeImpl);

        if (aChanges.getCount() == 1)
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

    bool Broadcaster::Impl::translateChanges(ChangesInfos& aInfos, NodeChanges const& aChanges, bool bSingleBase) const
    {
        ChangesInfos aNewInfos;
        aNewInfos.reserve( aChanges.getCount() );

        Tree aBaseTree = m_aNotifierData.second->getTree();
        Factory& rFactory = m_aNotifierData.second->getFactory();

        for (NodeChanges::Iterator it = aChanges.begin(); it != aChanges.end(); ++it)
        {
            aNewInfos.push_back( ChangeData(*it) );

            ChangeData& aNewChange = aNewInfos.back();

            // enabling the Single base optimization requires a base node (not only a base tree) for correct accessors
            //if (!bSingleBase || !configuration::equalTree(aBaseTree,aNewChange.info.baseTree))
            {
                if( !aNewChange.rebase(aBaseTree) )
                {
                    OSL_TRACE("Change is not within expected tree - skipping for notification");
                    continue;
                }
            }

            if( !aNewChange.resolveObjects(rFactory) )  OSL_TRACE("Cannot find affected elements of Change");

        }

        aNewInfos.swap(aInfos);
        return true;
    }

// ---------------------------------------------------------------------------------------------------

    void Broadcaster::Impl::notifyRootListeners(ChangesInfos const& aChanges)
    {
        if (aChanges.empty()) return;

        if (ApiTreeImpl const* pRootTree = m_aNotifierData.second->getRootTreeImpl())
        {
            osl::ClearableMutexGuard aGuardRoot( pRootTree->getApiLock() );

            NotifierHolder aRootNotifier = BroadcasterHelper::getImpl(pRootTree->getNotifier());
            if (aRootNotifier.isValid())
            {
                uno::Reference< css::util::XChangesListener > const * const pSelect = 0;

                NodeID aNotifiedNode = makeRootID( pRootTree );

                if (ListenerContainer* pContainer = aRootNotifier->m_aListeners.getContainer(aNotifiedNode.toIndex(), ::getCppuType(pSelect)) )
                {
                    css::util::ChangesEvent aEvent;
                    aEvent.Source = pRootTree->getUnoInstance();

                    UnoInterfaceRef xBaseInstance = m_aNotifierData.second->getUnoInstance();
                    aEvent.Base <<= xBaseInstance;

                    // translate and collect the changes
                    aEvent.Changes.realloc(aChanges.size());
                    css::util::ElementChange* pChange = aEvent.Changes.getArray();

                    for (ChangesInfos::const_iterator it = aChanges.begin(); it != aChanges.end(); ++it)
                    {
                        fillChangeFromResolved(*pChange, it->info);
                        ++pChange;
                    }

                    // now notify
                    ListenerContainerIterator< css::util::XChangesListener > aIter(*pContainer);
                    aGuardRoot.clear();

                    while (aIter.hasMoreElements())
                        aIter.next()->changesOccurred(aEvent);
                }
            }
        }
    }

// ---------------------------------------------------------------------------------------------------
// class Broadcaster
// ---------------------------------------------------------------------------------------------------
Broadcaster::Broadcaster(Notifier const& aNotifier, NodeChange const& aChange, bool bLocal)
: m_pImpl( Impl::create(aNotifier.m_aImpl,aNotifier.m_pTree,aChange,bLocal) )
{
    OSL_ASSERT(m_pImpl.isValid());
}
// ---------------------------------------------------------------------------------------------------
Broadcaster::Broadcaster(Notifier const& aNotifier, NodeChanges const& aChanges, bool bLocal)
: m_pImpl( Impl::create(aNotifier.m_aImpl,aNotifier.m_pTree,aChanges,bLocal) )
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

void Broadcaster::queryConstraints(NodeChange const& aChange) throw(com::sun::star::beans::PropertyVetoException)
{
    OSL_ENSURE(aChange.isChange(),"Constraint query without a change !");

    NodeChanges aChanges;
    aChanges.add(aChange);
    this->queryConstraints(aChanges,true);
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::queryConstraints(NodeChanges const& aChanges, bool bSingleBase) throw(com::sun::star::beans::PropertyVetoException)
{
    OSL_ENSURE(!aChanges.isEmpty(),"Constraint query without a change !");

    ChangesInfos aInfos;
    if (m_pImpl->translateChanges(aInfos,aChanges,bSingleBase))
    {
        m_pImpl->queryConstraints(aInfos);
    }
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(NodeChange const& aChange) throw()
{
    OSL_ENSURE(aChange.isChange(),"Notifying without a change !");

    NodeChanges aChanges;
    aChanges.add(aChange);
    this->notifyListeners(aChanges, true);
}
// ---------------------------------------------------------------------------------------------------

void Broadcaster::notifyListeners(NodeChanges const& aChanges, bool bSingleBase) throw()
{
    OSL_ENSURE(!aChanges.isEmpty(),"Notifying without a change !");

    ChangesInfos aInfos;
    if (m_pImpl->translateChanges(aInfos,aChanges, bSingleBase))
    {
        m_pImpl->notifyListeners(aInfos);
        m_pImpl->notifyRootListeners(aInfos);
    }
}
// ---------------------------------------------------------------------------------------------------
    }
}

