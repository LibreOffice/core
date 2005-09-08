/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: roottreeimpl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:33:28 $
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

#ifndef CONFIGMGR_ROOTTREEIMPL_HXX_
#define CONFIGMGR_ROOTTREEIMPL_HXX_

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data { class NodeAccess; }
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------
        /** is the Implementation class for class <type>Tree</type>.
            <p> Holds a list of <type>Node</type> which it allows to access by
                <type>NodeOffset</type> (which is basically a one-based index).
            </p>
            <p> Also provides for navigation to the context this tree is located in
            </p>
        */
        class RootTreeImpl : public TreeImpl
        {
        public:
        //  Construction
            /// creates a TreeImpl without a parent tree
            RootTreeImpl(   rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            AbsolutePath const& aRootPath,
                            data::NodeAccess const& _aCacheNode, TreeDepth nDepth,
                            TemplateProvider const& aTemplateProvider);

            // make it public
        private:
            virtual RootTreeImpl const* doCastToRootTree() const;
            virtual ElementTreeImpl const* doCastToElementTree() const;

            virtual Path::Component doGetRootName() const;
            virtual void doFinishRootPath(Path::Rep& rPath) const;

            AbsolutePath            m_aRootPath;
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_ROOTTREEIMPL_HXX_
