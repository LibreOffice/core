/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cacheline.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "cacheline.hxx"
#include "builddata.hxx"
#include "treechangefactory.hxx"
#include "mergechange.hxx"
#include "configexcept.hxx"
#include "tracer.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
// -----------------------------------------------------------------------------
    static inline rtl::OUString implExtractModuleName(configuration::AbsolutePath const& _aConfigPath)
    {
        return _aConfigPath.getModuleName();
    }


// -----------------------------------------------------------------------------
// class CacheLine
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

    CacheLine::CacheLine(rtl::OUString const & _aModuleName)
    : m_base(NULL)
    , m_name(_aModuleName)
    , m_nDataRefs(0)
    {
    }
// -----------------------------------------------------------------------------

    CacheLine::CacheLine(rtl::OUString const & _aModuleName, sharable::TreeFragment * _pSegment)
    : m_base(_pSegment)
    , m_name(_aModuleName)
    , m_nDataRefs(0)
    {
    }

// -----------------------------------------------------------------------------

    void CacheLine::setBase(sharable::TreeFragment * _base)
    {
        OSL_PRECOND(m_base == NULL, "CacheLine: Data base address was already set");
        OSL_PRECOND( _base != NULL, "CacheLine: Cannot set NULL base address");
        m_base = _base;
    }
// -----------------------------------------------------------------------------

    rtl::Reference<CacheLine> CacheLine::createAttached( rtl::OUString const & _aModuleName,
                                            sharable::TreeFragment * _aSegment
                                          ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        if (_aModuleName.getLength() == 0)
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aSegment == NULL)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL segment");
            return NULL;
        }

        rtl::Reference<CacheLine> xResult = new CacheLine(_aModuleName,_aSegment);

        return xResult;
    }
// -----------------------------------------------------------------------------

    rtl::Reference<CacheLine> CacheLine::createNew( rtl::OUString const & _aModuleName
                                      ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        if (_aModuleName.getLength() == 0)
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }

        rtl::Reference<CacheLine> xResult = new CacheLine(_aModuleName);

        return xResult;
    }
// -------------------------------------------------------------------------

    rtl::OUString CacheLine::getModuleName() const
    {
        return m_name;
    }
// -----------------------------------------------------------------------------

    sharable::TreeFragment * CacheLine::getPartialTree(configuration::AbsolutePath const& aConfigName) const
    {
        sharable::Node * parent = internalGetNode(aConfigName.getParentPath());

        if (parent != 0 && parent->isSet())
            return parent->set.getElement(aConfigName.getLocalName().getName());
        else
            return NULL;
    }
// -----------------------------------------------------------------------------

    bool CacheLine::hasDefaults() const
    {
        return m_base != 0 && m_base->hasDefaultsAvailable();
    }
// -----------------------------------------------------------------------------
    sharable::Node * CacheLine::internalGetNode(configuration::AbsolutePath const& aConfigName) const
    {
        OSL_ASSERT(m_base != 0);
        sharable::Node * node = m_base->getRootNode();
        OSL_ASSERT(node != 0);
        std::vector< configuration::Path::Component >::const_reverse_iterator i(
            aConfigName.begin());
        OSL_ASSERT(
            i != aConfigName.end() && node->getName() == i->getInternalName());
        while (node != 0 && ++i != aConfigName.end()) {
            node = node->getSubnode(i->getName());
        }
        return node;
    }
// -----------------------------------------------------------------------------

    sharable::Node * CacheLine::getNode(configuration::AbsolutePath const& aConfigName) const
    {
        return internalGetNode(aConfigName);
    }
// -------------------------------------------------------------------------

    sharable::TreeFragment * CacheLine::setComponentData( backend::ComponentDataStruct const & _aComponentInstance,
                                                   bool _bWithDefaults
                                                   ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        OSL_PRECOND(_aComponentInstance.data.get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aComponentInstance.name == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(base() == NULL, "Data is already loaded");

        if (base() == NULL) // no data yet
        {
            this->setBase( data::buildTree(_aComponentInstance.data->getName(), *_aComponentInstance.data, _bWithDefaults) );
        }

        return this->base();
    }
// -----------------------------------------------------------------------------

    sharable::TreeFragment * CacheLine::insertDefaults( backend::NodeInstance const & _aDefaultInstance
                                               ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        OSL_PRECOND(_aDefaultInstance.data().get(), "CacheLine::insertDefaults: inserting NULL defaults !");
        OSL_PRECOND(_aDefaultInstance.root().getDepth() == 1, "Should have complete component to fill tree with defaults");
        OSL_PRECOND(_aDefaultInstance.root().getModuleName() == this->getModuleName(),"Data location does not match module");

        OSL_PRECOND(m_base != NULL, "Data must already be loaded to insert defaults");

        if (m_base != NULL)
        {
            data::mergeDefaults(m_base,*_aDefaultInstance.data());
        }

        return m_base;
    }
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(rtl::OUString const & _aModuleName)
    : CacheLine(_aModuleName)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    ExtendedCacheLine::ExtendedCacheLine(rtl::OUString const & _aModuleName,
                                         sharable::TreeFragment * _aSegment)
    : CacheLine(_aModuleName,_aSegment)
    , m_pPending()
    {
    }

// -----------------------------------------------------------------------------

    rtl::Reference<ExtendedCacheLine> ExtendedCacheLine::createAttached( rtl::OUString const & _aModuleName,
                                                            sharable::TreeFragment * _aSegment
                                                          ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        if (_aModuleName.getLength() == 0)
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }
        if (_aSegment == NULL)
        {
            OSL_ENSURE(false, "Cannot attach a cache line to a NULL segment");
            return NULL;
        }

        rtl::Reference<ExtendedCacheLine> xResult = new ExtendedCacheLine(_aModuleName,_aSegment);

        return xResult;
    }
// -----------------------------------------------------------------------------

    rtl::Reference<ExtendedCacheLine> ExtendedCacheLine::createNew( rtl::OUString const & _aModuleName
                                                      ) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        if (_aModuleName.getLength() == 0)
        {
            OSL_ENSURE(false, "Cannot make a cache line without a name");
            return NULL;
        }

        rtl::Reference<ExtendedCacheLine> xResult = new ExtendedCacheLine(_aModuleName);

        return xResult;
    }
// -------------------------------------------------------------------------

    void ExtendedCacheLine::addPending(backend::ConstUpdateInstance const & _anUpdate) SAL_THROW((com::sun::star::uno::RuntimeException))
    {
        configuration::AbsolutePath aRootLocation = _anUpdate.root();

        OSL_PRECOND(!aRootLocation.isRoot(),"Pending change cannot be located at root");
        OSL_PRECOND(aRootLocation.getModuleName() == this->getModuleName(),"Pending change location does not match module");

        OSL_PRECOND(_anUpdate.data() != NULL,"Adding NULL 'pending' change");
        OSL_PRECOND(_anUpdate.data()->getNodeName() == aRootLocation.getLocalName().getName(),
                    "Path to pending change does not match change name");

        using std::auto_ptr;

        // first make the _pSubtreeChange a full tree starting at the module root
        auto_ptr<SubtreeChange> pRootChange;
        SubtreeChange *pExistingEntry = NULL;

        std::vector<configuration::Path::Component>::const_reverse_iterator last = aRootLocation.end();

        OSL_ASSERT(last != aRootLocation.begin());
        --last;

        for (std::vector<configuration::Path::Component>::const_reverse_iterator it = aRootLocation.begin();
             it != last;
             ++it)
        {
            OSL_ASSERT( it   != aRootLocation.end());
            OSL_ASSERT( it+1 != aRootLocation.end());
            // We need to create a new SubtreeChange
            rtl::OUString const aChangeName      = it->getName();
            rtl::OUString const aElementTypeName = (it+1)->getTypeName();

            auto_ptr<SubtreeChange> pNewChange =
                        OTreeChangeFactory::createDummyChange(aChangeName, aElementTypeName);

            if (pExistingEntry == NULL)
            {
                OSL_ASSERT(pRootChange.get() == NULL);

                pRootChange     = pNewChange;
                pExistingEntry  = pRootChange.get();
            }
            else
            {
                OSL_ASSERT(pRootChange.get() != NULL);

                pExistingEntry->addChange(base_ptr(pNewChange));

                Change* pChange = pExistingEntry->getChange(aChangeName);
                pExistingEntry = static_cast<SubtreeChange*>(pChange);

                OSL_ENSURE(dynamic_cast< SubtreeChange * >(pChange) != 0, "ERROR: Cannot recover change just added");
            }
        }

        auto_ptr<SubtreeChange> pAddedChange( new SubtreeChange(*_anUpdate.data(), treeop::DeepChildCopy()) );

        if (aRootLocation.getDepth() > 1)
        {
            OSL_ASSERT(pRootChange.get() != NULL && pExistingEntry != NULL);

            // the _pSubtreeChange did not start at root, so add its clone to the built dummies
            pExistingEntry->addChange(base_ptr(pAddedChange));
        }
        else
        {
            OSL_ASSERT(pRootChange.get() == NULL && pExistingEntry == NULL);

            // the _pSubtreeChange starts at root, so just reset pRootChange
            pRootChange = pAddedChange;
        }
        OSL_ASSERT(pRootChange.get() != NULL);

        if (m_pPending.get() == NULL)
        {
            // no merge is need, because the existing pending changes are empty
            m_pPending = pRootChange;
        }
        else
        {
            try
            {
                // We need to merge the new rebased changes into the m_pPending
                combineUpdates(*pRootChange,*m_pPending);
            }
            catch (configuration::Exception& e)
            {
                rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("Update cache for module: Could not add pending changes at"));

                sMessage += aRootLocation.toString();

                sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(". Internal Exception:")) + e.message();

                throw uno::RuntimeException(sMessage,0);
            }
        }

        OSL_POSTCOND(m_pPending.get() != NULL, "Could not insert new pending changes");
    }
// -----------------------------------------------------------------------------


} // namespace configmgr


