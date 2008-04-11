/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treesegment.cxx,v $
 * $Revision: 1.8 $
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

#include "treesegment.hxx"
#include "builddata.hxx"
#include "treeaccessor.hxx"
#include "utility.hxx"
#include <osl/diagnose.h>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
// -----------------------------------------------------------------------------
struct TreeSegment::Impl : configmgr::SimpleReferenceObject
{
    Impl() : base() {}
    ~Impl();

    data::TreeAddress   base;
};

// -----------------------------------------------------------------------------
TreeSegment::TreeSegment()
: m_pImpl()
{
}

// -----------------------------------------------------------------------------
TreeSegment::TreeSegment(Impl * _pImpl)
: m_pImpl(_pImpl)
{
}

// -----------------------------------------------------------------------------
TreeSegment::TreeSegment(TreeSegment const & _aOther)
: m_pImpl( _aOther.m_pImpl )
{
}

// -----------------------------------------------------------------------------
TreeSegment& TreeSegment::operator=(TreeSegment const & _aOther)
{
    m_pImpl = _aOther.m_pImpl;
    return *this;
}

// -----------------------------------------------------------------------------
TreeSegment::~TreeSegment()
{
}

// -----------------------------------------------------------------------------
void TreeSegment::clear()
{
    m_pImpl.clear();
}

// -----------------------------------------------------------------------------
TreeAccessor TreeSegment::getTreeAccess() const
{
    return TreeAccessor(getTreeData());
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
TreeSegment::Impl::~Impl()
{
    if (base != NULL)
        destroyTree(base);
}

// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(RawTreeData& _aTree, RawName const & _aTypeName)
{
    if (_aTree.get() == NULL) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    aNewImpl->base = buildElementTree(*_aTree,_aTypeName,false); // no defaults for set element trees

    if (aNewImpl->base == NULL) aNewImpl.reset();

    return aNewImpl.release();
}

// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(RawName const & _aTreeName, RawTreeData& _aTree)
{
    if (_aTree.get() == NULL) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    aNewImpl->base = buildTree(_aTreeName,*_aTree,false); // no defaults for set element trees

    if (aNewImpl->base == NULL) aNewImpl.reset();

    return aNewImpl.release();
}


// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(TreeAccessor const & _aTree)
{
    if (_aTree == NULL) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    aNewImpl->base = _aTree.copyTree();

    if (aNewImpl->base == NULL) aNewImpl.reset();

    return aNewImpl.release();
}

// -----------------------------------------------------------------------------
TreeSegment::RawTreeData TreeSegment::cloneData(bool _bUseTreeName) const
{
    return convertTree(this->getTreeAccess(),_bUseTreeName);
}

// -----------------------------------------------------------------------------
TreeSegment TreeSegment::cloneSegment() const
{
    if (!is()) return TreeSegment();

    return createNew( this->getTreeAccess() );
}

// -----------------------------------------------------------------------------
bool TreeSegment::is() const
{
    return hasData() && m_pImpl->base != NULL;
}

// -----------------------------------------------------------------------------
TreeSegment::Name TreeSegment::getName() const
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (!is()) return Name();

    return configuration::makeElementName( getTreeData()->getName(), Name::NoValidate() );
}

// -----------------------------------------------------------------------------
void TreeSegment::setName(Name const & _aNewName)
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (is())
    {
        sharable::String aOldName = getTreeDataForUpdate()->header.name;

        sharable::String aNewName = sharable::allocString(_aNewName.toString());

        getTreeDataForUpdate()->header.name = aNewName;

        sharable::freeString(aOldName);
    }
}
// -----------------------------------------------------------------------------
void TreeSegment::markRemovable()
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (is())
        getTreeDataForUpdate()->header.state |= State::flag_removable;
}
// -----------------------------------------------------------------------------
TreeAddress TreeSegment::getBaseAddress() const
{
    return hasData() ? m_pImpl->base : NULL;
}

// -----------------------------------------------------------------------------
TreeSegment::TreeDataPtr TreeSegment::getTreeData() const
{
    if (!is()) return NULL;

    return m_pImpl->base;
}

// -----------------------------------------------------------------------------
TreeSegment::TreeDataUpdatePtr TreeSegment::getTreeDataForUpdate() const
{
    OSL_ASSERT(this->is());

    if (!is()) return NULL;

    return m_pImpl->base;
}

// -----------------------------------------------------------------------------
TreeSegment::NodeDataPtr TreeSegment::getSegmentRootNode() const
{
    if (sharable::TreeFragment const * pTree = getTreeData())
        return & pTree->nodes[0];

    else
        return NULL;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr


