/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confignotifier.hxx,v $
 * $Revision: 1.11 $
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

#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#define CONFIGMGR_CONFIGNOTIFIER_HXX_

#include "configexcept.hxx"
#include "configpath.hxx"
#include "datalock.hxx"
#include "utility.hxx"
#include <boost/utility.hpp>
#include <vos/ref.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans
    {
        class XPropertyChangeListener;
        class XVetoableChangeListener;
        class XPropertiesChangeListener;
        class PropertyVetoException;
    }
    namespace lang
    {
        class XEventListener;
        struct EventObject;
    }
    namespace container { class XContainerListener; }
    namespace util      { class XChangesListener; }
    namespace uno       { class RuntimeException; }
} } }
namespace osl { class Mutex; }

namespace configmgr
{
    namespace configuration
    {
// ---------------------------------------------------------------------------------------------------
        class NodeRef;
        class ValueRef;
        class NodeChange;
        class NodeChanges;
    }
// ---------------------------------------------------------------------------------------------------
    namespace configapi
    {
// ---------------------------------------------------------------------------------------------------
        class Broadcaster;
        class NotifierImpl;
        class ApiTreeImpl;

        namespace css = ::com::sun::star;
// ---------------------------------------------------------------------------------------------------
        /// manages collections of event listeners observing a config tree, thread-safe
        class Notifier
        {
            friend class Broadcaster;
            friend class BroadcasterHelper;
            vos::ORef<NotifierImpl>             m_aImpl;
            ApiTreeImpl const*const m_pTree;
        public:
            /// construct this around the given Implementation, for the given tree
            explicit Notifier(vos::ORef<NotifierImpl> const & aImpl, ApiTreeImpl const* pTree);
            Notifier(Notifier const& aOther);
            ~Notifier();

        // ---------------------------------------------------------------------------------------------------
            /// create a broadcaster for a single change (either local or (possibly) nested)
            Broadcaster makeBroadcaster(configuration::NodeChange const& aChange, bool bLocal) const;
            /// create a broadcaster for a collection of changes (either local or (possibly) nested)
            Broadcaster makeBroadcaster(configuration::NodeChanges const& aChange, bool bLocal) const;

        // ---------------------------------------------------------------------------------------------------
            bool checkAlive(uno::XInterface* pObject) const throw(css::lang::DisposedException);

        // ---------------------------------------------------------------------------------------------------
            /// Add a <type scope='com::sun::star::lang'>XEventListener</type> observing <var>aNode</var>.
            void add(configuration::NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::container'>XContainerListener</type> observing <var>aNode</var>.
            void add(configuration::NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::util'>XChangesListener</type> observing <var>aNode</var> and its descendants.
            void add(configuration::NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing all children of <var>aNode</var>.
            void addForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Add a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing only the child named <var>aName</var> of <var>aNode</var>.
            void addForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, rtl::OUString const& aName) const;
            /// Add a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining all children of <var>aNode</var>.
            void addForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /// Add a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining only the child named <var>aName</var> of <var>aNode</var>.
            void addForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, rtl::OUString const& aName) const;

            /** Add a <type scope='com::sun::star::beans'>XPropertiesChangeListener</type>
                observing the properties of <var>aNode</var> (optimally only those given by <var>aNames</var>.
            */
            void add(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener, uno::Sequence<rtl::OUString> const& aNames) const;

        // ---------------------------------------------------------------------------------------------------
            /// Remove a <type scope='com::sun::star::lang'>XEventListener</type> observing <var>aNode</var>.
            void remove(configuration::NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::container'>XContainerListener</type> observing <var>aNode</var>.
            void remove(configuration::NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::util'>XChangesListener</type> observing <var>aNode</var> and its descendants.
            void remove(configuration::NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing <var>aNode</var>.
            void removeForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Remove a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing the child named <var>aName</var> of <var>aNode</var>.
            void removeForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener, rtl::OUString const& aName) const;
            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining <var>aNode</var>.
            void removeForAll(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining the child named <var>aName</var> of <var>aNode</var>.
            void removeForOne(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener, rtl::OUString const& aName) const;

            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining <var>aNode</var>.
            void remove(configuration::NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /** Remove a <type scope='com::sun::star::beans'>XPropertiesChangeListener</type>
                observing any properties of <var>aNode</var>.
            */
            void remove(configuration::NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const;
        // ---------------------------------------------------------------------------------------------------
        private:
            void operator=(Notifier const& aOther);
        };
// ---------------------------------------------------------------------------------------------------

// Thin Wrappers around Notifiers: Provide guarding and convenient access
    // Guarding and locking implementations
        class NodeAccess;
        class TreeElement;

        ///  guards a NodeAccess; provides a simple lock for non-data access, does not check for disposed state
        class DisposeGuardImpl: private boost::noncopyable
        {
        public:
            DisposeGuardImpl(Notifier const& rNotifier) throw();
            ~DisposeGuardImpl() throw ();
        };

        /// wraps a Notifier (from a node or tree); provides a simple lock for notifier access, does not check for disposed state
        class GuardedNotifier
        {
            UnoApiLock          m_aLock;
            Notifier            m_aNotifier;
            DisposeGuardImpl    m_aImpl;
        public:
            GuardedNotifier(NodeAccess& rNode) throw();
        public:
            Notifier const& get()        const  { return m_aNotifier; }

            Notifier const& operator *() const  { return  get(); }
            Notifier const* operator->() const  { return &get(); }
        };

        /// guards a Node or Tree provides a simple lock for non-data access, does (!) check for disposed state
        class DisposeGuard
        {
            DisposeGuardImpl    m_aImpl;
            UnoApiLock          m_aLock;
        public:
            DisposeGuard(NodeAccess& rNode) throw(css::lang::DisposedException);
        };

// ---------------------------------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNOTIFIER_HXX_
