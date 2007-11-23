/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treesegment.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:33:52 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "treesegment.hxx"

#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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


