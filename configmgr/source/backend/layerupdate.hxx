/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerupdate.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:31:06 $
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

#ifndef CONFIGMGR_BACKEND_LAYERUPDATE_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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




