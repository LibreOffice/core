/*************************************************************************
 *
 *  $RCSfile: broadcaster.hxx,v $
 *
 *  $Revision: 1.1 $
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

#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#define CONFIGMGR_API_BROADCASTER_HXX_

#include "apitypes.hxx"
#include <vos/ref.hxx>

namespace com { namespace sun { namespace star {
    namespace beans
    {
        class PropertyVetoException;
    }
} } }

namespace configmgr
{
    namespace configuration
    {
// ---------------------------------------------------------------------------------------------------
        class NodeChange;
        class NodeChanges;
    }
// ---------------------------------------------------------------------------------------------------
    namespace configapi
    {
        using configuration::NodeChange;
        using configuration::NodeChanges;

        class Notifier;

        namespace css = ::com::sun::star;
// ---------------------------------------------------------------------------------------------------

        /// broadcasts events for changes to a single config node or several sibling nodes
        class Broadcaster
        {
        public:
            /// construct a broadcaster
            Broadcaster(Notifier const& aNotifier, NodeChange const& aChange, bool bLocal);
            Broadcaster(Notifier const& aNotifier, NodeChanges const& aChanges, bool bLocal);
            Broadcaster(Broadcaster const& aOther);
            ~Broadcaster();

            /// give all property veto listeners on the affected node a chance to veto
            void queryConstraints(NodeChange const& aChange) throw(css::beans::PropertyVetoException);
            /// give all property veto listeners on any of the affected nodes a chance to veto
            void queryConstraints(NodeChanges const& aChanges) throw(css::beans::PropertyVetoException);
            /// notify all listeners which are affected by this change
            void notifyListeners(NodeChange const& aChange) throw();
            /// notify all listeners which are affected by any of these changes
            void notifyListeners(NodeChanges const& aChanges) throw();

            class Impl;
        private:
            vos::ORef<Impl> m_pImpl;
        private:
            void operator=(Broadcaster const& aOther);
        };

// ---------------------------------------------------------------------------------------------------

    }
}

#endif // CONFIGMGR_API_BROADCASTER_HXX_
