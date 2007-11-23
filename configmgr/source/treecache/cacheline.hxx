/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheline.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:36:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_CACHELINE_HXX
#define CONFIGMGR_CACHELINE_HXX

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_REQUESTTYPES_HXX_
#include "requesttypes.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef INCLUDED_SHARABLE_TREEFRAGMENT_HXX
#include "treefragment.hxx"
#endif

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
////////////////////////////////////////////////////////////////////////////////
    using ::rtl::OUString;

    namespace data {
        /* class NodeAddress; */ class NodeAccess;
    }
////////////////////////////////////////////////////////////////////////////////
    /** This object represents a cache line for a single configuration tree
    */
    class CacheLine : public configmgr::SimpleReferenceObject, Noncopyable
    {
    public:
        typedef configuration::Name         Name;
        typedef configuration::AbsolutePath Path;

        // create a new CacheLine for the given component name
        static rtl::Reference<CacheLine>
            createNew( Name const & _aModuleName );

        // create a new CacheLine attached to the given memory location
        static rtl::Reference<CacheLine>
            createAttached( Name const & _aModuleName, data::TreeAddress _aLocation );

        bool hasDefaults() const;

        bool isEmpty() const { return m_base == NULL; }
        data::TreeAddress   getTreeAddress() const { return m_base; }
        data::TreeAddress   getPartialTree(Path const & _aTemplatePath ) const;
        data::NodeAddress   getNode(Path const & _aPath) const;

        data::TreeAddress   setComponentData( backend::ComponentData const & _aComponentInstance,
                                              bool _bWithDefaults
                                           ) CFG_UNO_THROW_RTE();

        data::TreeAddress   insertDefaults(   backend::NodeInstance const & _aDefaultInstance
                                           ) CFG_UNO_THROW_RTE();

        // get the module name for this component
        Name getModuleName() const;

        /// add a client for this module's data
        oslInterlockedCount clientReferences() const { return m_nDataRefs; }
        /// add a client for this modules data
        oslInterlockedCount clientAcquire() { return osl_incrementInterlockedCount(&m_nDataRefs); }
        /// subtract a client for this modules data
        oslInterlockedCount clientRelease() { return osl_decrementInterlockedCount(&m_nDataRefs); }

    protected:
        // create a new CacheLine attached to the given memory location
        explicit
        CacheLine( Name const & _aModuleName, data::TreeAddress _pLocation );

        // create a new empty CacheLine for the given component name
        explicit
        CacheLine( Name const & _aModuleName );

        data::NodeAccess internalGetNode(Path const & _rPath) const;

        data::TreeAddress base() const { return m_base; }
        void setBase(data::TreeAddress _base);

    private:
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
            createNew( Name const & _aModuleName );

        // create a new CacheLine attached to the given memory location
        static rtl::Reference<ExtendedCacheLine>
            createAttached( Name const & _aModuleName,
                            data::TreeAddress _aLocation );

    // management of pending changes
        bool hasPending() const {return m_pPending.get() != NULL;}

        void addPending(backend::ConstUpdateInstance const & _anUpdate) CFG_UNO_THROW_RTE();
        std::auto_ptr<SubtreeChange> releasePending() {return m_pPending;}

    private:
        // create a new empty CacheLine for the given component name
        explicit
        ExtendedCacheLine( Name const & _aModuleName );

        // create a new CacheLine attached to the given memory location
        explicit
        ExtendedCacheLine( Name const & _aModuleName,
                           data::TreeAddress _aLocation );

    private:
        std::auto_ptr<SubtreeChange> m_pPending;
    };
    typedef rtl::Reference<ExtendedCacheLine> ExtendedCacheLineRef;

} // namespace configmgr

#endif

