/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: roottreeimpl.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_ROOTTREEIMPL_HXX_
#define CONFIGMGR_ROOTTREEIMPL_HXX_

#include "tree.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------
        /** is the Implementation class for class <type>Tree</type>.
            <p> Holds a list of <type>Node</type> which it allows to access by
                <type>unsigned int</type> (which is basically a one-based index).
            </p>
            <p> Also provides for navigation to the context this tree is located in
            </p>
        */
        class RootTree : public Tree
        {
        public:
        //  Construction
            /// creates a Tree without a parent tree
            RootTree(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            AbsolutePath const& aRootPath,
                            sharable::Node * cacheNode, unsigned int nDepth,
                            TemplateProvider const& aTemplateProvider);

            // make it public
        private:
            virtual Path::Component doGetRootName() const;
            virtual void doFinishRootPath(Path::Rep& rPath) const;

            AbsolutePath            m_aRootPath;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_ROOTTREEIMPL_HXX_
