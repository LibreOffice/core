/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: namecreator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:50:42 $
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

#ifndef INCLUDED_CONFIGMGR_NAMECREATOR_HXX
#define INCLUDED_CONFIGMGR_NAMECREATOR_HXX

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
    // -----------------------------------------------------------------------------
    class Change;
    class SubtreeChange;
    // -----------------------------------------------------------------------------
    using configuration::Name;
    using configuration::AbsolutePath;
    using configuration::RelativePath;
    // -----------------------------------------------------------------------------

    class ONameCreator
    {
    public:
        typedef configuration::Path::Component  FullName;
        typedef configuration::Path::Rep        PathRep;
        typedef std::vector< FullName > NameList;
    public:
        ONameCreator() {}

        void pushName(const FullName &_aName) { m_aNameList.push_back(_aName); }
        void popName()                       { m_aNameList.pop_back(); }

        void clear() { m_aNameList.clear(); }

        RelativePath buildPath() const;
        RelativePath buildPath(const FullName &_aPlusName) const;

        static FullName createName(Change const& _rChange, SubtreeChange const* _pParent);
    private:
        NameList     m_aNameList;
    };

    template <class PathClass>
    class OPathCreator : public ONameCreator
    {
    public:
        typedef PathClass PathType;
    public:
        OPathCreator() : m_aBasePath( PathRep() ) {}
        OPathCreator(PathClass const & _aBasePath) : m_aBasePath(_aBasePath) {}

        void init(PathClass const & _aBasePath)
        { clear(); m_aBasePath = _aBasePath; }

        PathClass createPath() const
        { return m_aBasePath.compose( ONameCreator::buildPath() ); }

        PathClass createPath(const FullName & _aPlusName) const
        { return m_aBasePath.compose( ONameCreator::buildPath(_aPlusName) ); }
    private:
        PathClass m_aBasePath;
    };
    // -----------------------------------------------------------------------------

} // namespace configmgr

#endif

