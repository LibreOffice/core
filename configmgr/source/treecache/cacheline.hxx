/*************************************************************************
 *
 *  $RCSfile: cacheline.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:42 $
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

#ifndef CONFIGMGR_CACHELINE_HXX
#define CONFIGMGR_CACHELINE_HXX

#ifndef CONFIGMGR_TREEADDRESS_HXX
#include "treeaddress.hxx"
#endif
#ifndef CONFIGMGR_SEGMENT_HXX
#include "segment.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#include "requesttypes.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

    namespace data { class NodeAddress; class TreeAddress; class NodeAccess; }
////////////////////////////////////////////////////////////////////////////////
    /** This object represents a cache line for a single configuration tree
    */
    class CacheLine : public salhelper::SimpleReferenceObject, Noncopyable
    {
    public:
        typedef oslInterlockedCount RefCount;

        typedef configuration::Name         Name;
        typedef configuration::AbsolutePath Path;

        // create a new CacheLine for the given component name
        static rtl::Reference<CacheLine>
            createNew( Name const & _aModuleName, memory::HeapManager & _rHeapImpl );

        // create a new CacheLine attached to the given memory location
        static rtl::Reference<CacheLine>
            createAttached( Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                            memory::SegmentAddress const & _aLocation );

        memory::Segment * getDataSegment();

        memory::SegmentAddress getDataSegmentAddress() const;

        bool isEmpty() const { return m_base.isNull(); }

        bool hasDefaults(memory::Accessor const & _aAccessor) const;

        data::TreeAddress getModuleRootAddress( ) const { return m_base; }

        data::TreeAddress   getPartialTree(memory::Accessor const & _aAccessor, Path const & _aTemplatePath ) const;
        data::NodeAddress   getNode(memory::Accessor const & _aAccessor, Path const & _aPath) const;

        data::TreeAddress   setComponentData( memory::UpdateAccessor& _aAccessToken,
                                              backend::ComponentData const & _aComponentInstance,
                                              bool _bWithDefaults
                                           ) CFG_UNO_THROW_RTE();

        data::TreeAddress   insertDefaults(   memory::UpdateAccessor& _aAccessToken,
                                              backend::NodeInstance const & _aDefaultInstance
                                           ) CFG_UNO_THROW_RTE();

        // get the module name for this component
        Name getModuleName() const;

        /// add a client for this module's data
        RefCount clientReferences() const { return m_nDataRefs; }
        /// add a client for this modules data
        RefCount clientAcquire() { return osl_incrementInterlockedCount(&m_nDataRefs); }
        /// subtract a client for this modules data
        RefCount clientRelease() { return osl_decrementInterlockedCount(&m_nDataRefs); }

    protected:
        // create a new CacheLine attached to the given memory location
        explicit
        CacheLine( Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                    memory::SegmentAddress const & _aLocation );

        // create a new empty CacheLine for the given component name
        explicit
        CacheLine( Name const & _aModuleName, memory::HeapManager & _rHeapImpl );

        data::NodeAccess internalGetNode(memory::Accessor const & _anAccessor, Path const & _rPath) const;

        data::TreeAddress base() const { return m_base; }
        void setBase(data::TreeAddress _base);

    private:
        memory::Segment     m_storage;
        data::TreeAddress   m_base;
        Name                m_name;

        oslInterlockedCount m_nDataRefs;            /// the number of clients on this modules data
    };
    typedef rtl::Reference<CacheLine> CacheLineRef;

////////////////////////////////////////////////////////////////////////////////
    /** This object represents a cache line and associated data for a single configuration tree
    */
    class ExtendedCacheLine : public CacheLine
    {
    public:
        // create a new CacheLine for the given component name
        static rtl::Reference<ExtendedCacheLine>
            createNew( Name const & _aModuleName, memory::HeapManager & _rHeapImpl );

        // create a new CacheLine attached to the given memory location
        static rtl::Reference<ExtendedCacheLine>
            createAttached( Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                            memory::SegmentAddress const & _aLocation );

    // management of pending changes
        bool hasPending() const {return m_pPending.get() != NULL;}

        void addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE();
        std::auto_ptr<SubtreeChange> releasePending() {return m_pPending;}

    private:
        // create a new empty CacheLine for the given component name
        explicit
        ExtendedCacheLine( Name const & _aModuleName, memory::HeapManager & _rHeapImpl );

        // create a new CacheLine attached to the given memory location
        explicit
        ExtendedCacheLine( Name const & _aModuleName, memory::HeapManager & _rHeapImpl,
                            memory::SegmentAddress const & _aLocation );

    private:
        std::auto_ptr<SubtreeChange> m_pPending;
    };
    typedef rtl::Reference<ExtendedCacheLine> ExtendedCacheLineRef;
////////////////////////////////////////////////////////////////////////////////

    inline
    memory::Segment * CacheLine::getDataSegment()
    { return &m_storage; }

////////////////////////////////////////////////////////////////////////////////

} // namespace configmgr

#endif

