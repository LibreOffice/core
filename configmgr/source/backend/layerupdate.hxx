/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdate.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_BACKEND_LAYERUPDATE_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
} } } } }

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno        = ::com::sun::star::uno;
        namespace backenduno = ::com::sun::star::configuration::backend;

        using rtl::OUString;
// -----------------------------------------------------------------------------
        class NodeUpdate;
        typedef rtl::Reference<NodeUpdate> NodeUpdateRef;

        class LayerUpdate
        {
        public:
            typedef uno::Reference< backenduno::XLayerHandler > LayerWriter;

            LayerUpdate();
            LayerUpdate(LayerUpdate const & _aOther);
            ~LayerUpdate();
            LayerUpdate & operator =(LayerUpdate const & _aOther);
        public:
            typedef std::vector<OUString> ContextPath;

            bool isEmpty() const { return ! m_xContextNode.is(); }

            NodeUpdateRef       getContextNode() const;
            ContextPath const & getContextPath() const;

            void setContextNode(NodeUpdateRef const & _xContextNode);

        private:
            void makeContextPath(OUString const & _aContextPath);

        private:
            NodeUpdateRef   m_xContextNode;
            ContextPath     m_aContextPath;
        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




