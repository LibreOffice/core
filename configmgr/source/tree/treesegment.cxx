/*************************************************************************
 *
 *  $RCSfile: treesegment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:38:28 $
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

#include "treesegment.hxx"

#ifndef CONFIGMGR_BUILDDATA_HXX
#include "builddata.hxx"
#endif
#ifndef CONFIGMGR_SEGMENT_HXX
#include "segment.hxx"
#endif
#ifndef CONFIGMGR_HEAPFACTORY_HXX
#include "heapfactory.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
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

//        typedef std::auto_ptr<INode> RawTreeData;
//        typedef configuration::Name  Name;
        using memory::Pointer;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
struct TreeSegment::Impl : salhelper::SimpleReferenceObject
{
    Impl() : data( memory::localHeap() ), base() {}
    ~Impl();

    memory::Segment     data;
    data::TreeAddress   base;
};
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

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
memory::Segment * TreeSegment::getSegment() const
{
    return is() ? &m_pImpl->data : 0;
}

// -----------------------------------------------------------------------------
memory::Accessor  TreeSegment::getAccessor() const
{
    return memory::Accessor(getSegment());
}

// -----------------------------------------------------------------------------
TreeAccessor    TreeSegment::getTreeAccess() const
{
    return TreeAccessor( getAccessor(), getTreeData() );
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
TreeSegment::Impl::~Impl()
{
    if (base.is())
    {
        memory::UpdateAccessor aAccess( & this->data );
        destroyTree(aAccess,base);
    }
}

// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(RawTreeData& _aTree, RawName const & _aTypeName)
{
    if (_aTree.get() == NULL) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    memory::UpdateAccessor aNewAccess( & aNewImpl->data );

    aNewImpl->base = buildElementTree(aNewAccess,*_aTree,_aTypeName,false); // no defaults for set element trees

    if (!aNewImpl->base.is()) aNewImpl.reset();

    return aNewImpl.release();
}

// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(RawName const & _aTreeName, RawTreeData& _aTree)
{
    if (_aTree.get() == NULL) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    memory::UpdateAccessor aNewAccess( & aNewImpl->data );

    aNewImpl->base = buildTree(aNewAccess,_aTreeName,*_aTree,false); // no defaults for set element trees

    if (!aNewImpl->base.is()) aNewImpl.reset();

    return aNewImpl.release();
}


// -----------------------------------------------------------------------------
TreeSegment::Impl* TreeSegment::createNewSegment(TreeAccessor const & _aTree)
{
    if (!_aTree.isValid()) return NULL;

    std::auto_ptr<Impl> aNewImpl( new Impl );

    memory::UpdateAccessor aNewAccess( & aNewImpl->data );

    aNewImpl->base = _aTree.copyTree(aNewAccess);

    if (!aNewImpl->base.is()) aNewImpl.reset();

    return aNewImpl.release();
}

// -----------------------------------------------------------------------------
TreeSegment::RawTreeData TreeSegment::cloneData(bool _bUseTreeName) const
{
    return convertTree( this->getTreeAccess(), _bUseTreeName );
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
    return hasData() && m_pImpl->base.is();
}

// -----------------------------------------------------------------------------
TreeSegment::Name TreeSegment::getName() const
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (!is()) return Name();

    Accessor accessor = getAccessor();

    return configuration::makeElementName( getTreeData()->getName(accessor), Name::NoValidate() );
}

// -----------------------------------------------------------------------------
void TreeSegment::setName(Name const & _aNewName)
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (is())
    {
        memory::UpdateAccessor aUpdater( this->getSegment() );

        sharable::String aOldName = getTreeDataForUpdate(aUpdater)->header.name;

        sharable::String aNewName = sharable::allocString(aUpdater.allocator(),_aNewName.toString());

        getTreeDataForUpdate(aUpdater)->header.name = aNewName;

        sharable::freeString(aUpdater.allocator(),aOldName);
    }
}
// -----------------------------------------------------------------------------
void TreeSegment::markRemovable()
{
    OSL_ENSURE(is(), "Operation requires a valid tree");

    if (is())
    {
        memory::UpdateAccessor aUpdater( this->getSegment() );
        getTreeDataForUpdate(aUpdater)->header.state |= State::flag_removable;
    }
}
// -----------------------------------------------------------------------------
TreeAddress TreeSegment::getBaseAddress() const
{
    return hasData() ? m_pImpl->base : TreeAddress();
}

// -----------------------------------------------------------------------------
TreeSegment::TreeDataPtr TreeSegment::getTreeData() const
{
    if (!is()) return NULL;

    return TreeAccessor::access(m_pImpl->base, this->getAccessor());
}

// -----------------------------------------------------------------------------
TreeSegment::TreeDataUpdatePtr TreeSegment::getTreeDataForUpdate(memory::UpdateAccessor& _anUpdater) const
{
    OSL_ASSERT(_anUpdater.is());
    OSL_ASSERT(this->is());

    if (!is()) return NULL;

    return TreeAccessor::access(m_pImpl->base,_anUpdater);
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


