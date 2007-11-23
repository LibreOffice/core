/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatedispatch.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:11:21 $
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

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCH_HXX
#define CONFIGMGR_BACKEND_UPDATEDISPATCH_HXX

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XUpdateHandler;
} } } } }

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace configuration
    {
        class AbsolutePath;
    }
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace backenduno = ::com::sun::star::configuration::backend;

// -----------------------------------------------------------------------------

        class UpdateDispatcher : ChangeTreeAction, data::SetVisitor, Noncopyable
        {
        public:
            typedef uno::Reference< backenduno::XUpdateHandler > UpdateHandler;
            UpdateDispatcher(UpdateHandler const & _xUpdateHandler, OUString const & _aLocale);
            ~UpdateDispatcher();

            void dispatchUpdate(configuration::AbsolutePath const & _aRootPath, SubtreeChange const& _anUpdate);

        protected:
            using data::NodeVisitor::handle;

        private:
            void startUpdate();
            void endUpdate();

            virtual void handle(ValueChange const& aValueNode);
            virtual void handle(AddNode const& aAddNode);
            virtual void handle(RemoveNode const& aRemoveNode);
            virtual void handle(SubtreeChange const& aSubtree) ;

            virtual Result handle(data::ValueNodeAccess const& _aNode);
            virtual Result handle(data::GroupNodeAccess const& _aNode);
            virtual Result handle(data::SetNodeAccess const& _aNode);
            virtual Result handle(data::TreeAccessor const& _aElement);
        private:
            sal_Int16 getUpdateAttributes(node::Attributes const & _aAttributes, bool bAdded);
            sal_Int16 getUpdateAttributeMask(node::Attributes const & _aAttributes);

            bool testReplacedAndGetName(data::NodeAccess const & _aNode, OUString & _aName);
        private:
            configuration::AbsolutePath const * m_pContextPath;
            UpdateHandler   m_xUpdateHandler;
            OUString        m_aLocale;
            OUString        m_aElementName;
            bool    m_bInValueSet;
            bool    m_bInLocalizedValues;
        };
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




