/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: namecreator.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CONFIGMGR_NAMECREATOR_HXX
#define INCLUDED_CONFIGMGR_NAMECREATOR_HXX

#include "configpath.hxx"

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

