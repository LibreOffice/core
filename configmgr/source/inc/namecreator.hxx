/*************************************************************************
 *
 *  $RCSfile: namecreator.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-07-05 17:05:46 $
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

