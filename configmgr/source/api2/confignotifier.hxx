/*************************************************************************
 *
 *  $RCSfile: confignotifier.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-16 18:15:43 $
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

#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#define CONFIGMGR_CONFIGNOTIFIER_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"
#include "configpath.hxx"
#include <vos/ref.hxx>

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
        class DisposedException;
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
        class NodeChange;
        class NodeChanges;

        class Name;
        typedef uno::RuntimeException UnoError;
    }
// ---------------------------------------------------------------------------------------------------
    namespace configapi
    {
        using configuration::NodeRef;
        using configuration::NodeChange;
        using configuration::NodeChanges;

// ---------------------------------------------------------------------------------------------------
        class Broadcaster;
        class NotifierImpl;
        class ApiTreeImpl;
        typedef vos::ORef<NotifierImpl> NotifierHolder;

        namespace css = ::com::sun::star;
// ---------------------------------------------------------------------------------------------------
        /// manages collections of event listeners observing a config tree, thread-safe
        class Notifier
        {
            friend class Broadcaster;
            friend class BroadcasterHelper;
            NotifierHolder          m_aImpl;
            ApiTreeImpl const*const m_pTree;
        public:
            /// construct this around the given Implementation, for the given tree
            explicit Notifier(NotifierHolder const & aImpl, ApiTreeImpl const* pTree);
            Notifier(Notifier const& aOther);
            ~Notifier();

        // ---------------------------------------------------------------------------------------------------
            /// create a broadcaster for a single change (either local or (possibly) nested)
            Broadcaster makeBroadcaster(NodeChange const& aChange, bool bLocal) const;
            /// create a broadcaster for a collection of changes (either local or (possibly) nested)
            Broadcaster makeBroadcaster(NodeChanges const& aChange, bool bLocal) const;

        // ---------------------------------------------------------------------------------------------------
            osl::Mutex&         getMutex() const;
            //configuration::Tree getTree() const { return m_aTree; }
            //NotifierImpl&     getImpl() const { return m_aImpl.getBody(); }

        // ---------------------------------------------------------------------------------------------------
            bool checkAlive(uno::XInterface* pObject) const throw(css::lang::DisposedException);

        // ---------------------------------------------------------------------------------------------------
            /// Add a <type scope='com::sun::star::lang'>XEventListener</type> observing <var>aNode</var>.
            void add(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::container'>XContainerListener</type> observing <var>aNode</var>.
            void add(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::util'>XChangesListener</type> observing <var>aNode</var> and its descendants.
            void add(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const;

            /// Add a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing all children of <var>aNode</var>.
            void addForAll(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Add a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing only <var>aNode</var>.
            void addForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Add a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining all children of <var>aNode</var>.
            void addForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /// Add a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining only <var>aNode</var>.
            void addForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;

            /** Add a <type scope='com::sun::star::beans'>XPropertiesChangeListener</type>
                observing all properties of <var>aNode</var>.
            */
            void add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const;
            /** Add a <type scope='com::sun::star::beans'>XPropertiesChangeListener</type>
                observing the properties of <var>aNode</var> (optimally only those given by <var>aNames</var>.
            */
            void add(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener, uno::Sequence<OUString> const& aNames) const;

        // ---------------------------------------------------------------------------------------------------
            /// Remove a <type scope='com::sun::star::lang'>XEventListener</type> observing <var>aNode</var>.
            void remove(NodeRef const& aNode, uno::Reference< css::lang::XEventListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::container'>XContainerListener</type> observing <var>aNode</var>.
            void remove(NodeRef const& aNode, uno::Reference< css::container::XContainerListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::util'>XChangesListener</type> observing <var>aNode</var> and its descendants.
            void remove(NodeRef const& aNode, uno::Reference< css::util::XChangesListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing <var>aNode</var>.
            void removeForAll(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Remove a <type scope='com::sun::star::beans'>XPropertyChangeListener</type> observing <var>aNode</var>.
            void removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XPropertyChangeListener > const& xListener) const;
            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining <var>aNode</var>.
            void removeForAll(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining <var>aNode</var>.
            void removeForOne(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;

            /// Remove a <type scope='com::sun::star::beans'>XVetoableChangeListener</type> constraining <var>aNode</var>.
            void remove(NodeRef const& aNode, uno::Reference< css::beans::XVetoableChangeListener > const& xListener) const;
            /** Remove a <type scope='com::sun::star::beans'>XPropertiesChangeListener</type>
                observing any properties of <var>aNode</var>.
            */
            void remove(NodeRef const& aNode, uno::Reference< css::beans::XPropertiesChangeListener > const& xListener) const;
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
        class DisposeGuardImpl : NotCopyable
        {
            osl::MutexGuard m_aLock;
        public:
            DisposeGuardImpl(NotifierImpl& rNotifierImpl) throw();
            DisposeGuardImpl(Notifier const& rNotifier) throw();
            ~DisposeGuardImpl() throw ();
        };

        /// wraps a Notifier (from a node or tree); provides a simple lock for notifier access, does not check for disposed state
        class GuardedNotifier
        {
            Notifier            m_aNotifier;
            DisposeGuardImpl    m_aImpl;
        public:
            GuardedNotifier(Notifier const& rNotifier) throw();
            GuardedNotifier(NodeAccess& rNode) throw();
            GuardedNotifier(TreeElement& rTree) throw();
        public:
            Notifier const& get()        const  { return m_aNotifier; }

            Notifier const& operator *() const  { return  get(); }
            Notifier const* operator->() const  { return &get(); }
        };

        /// guards a Node or Tree provides a simple lock for non-data access, does (!) check for disposed state
        class DisposeGuard
        {
            DisposeGuardImpl    m_aImpl;
        public:
            DisposeGuard(NodeAccess& rNode) throw(css::lang::DisposedException);
            DisposeGuard(TreeElement& rTree) throw(css::lang::DisposedException);
        };

// ---------------------------------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNOTIFIER_HXX_
