/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatedispatch.hxx,v $
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

#ifndef CONFIGMGR_BACKEND_UPDATEDISPATCH_HXX
#define CONFIGMGR_BACKEND_UPDATEDISPATCH_HXX

#include "change.hxx"
#include "nodevisitor.hxx"
#include "utility.hxx"
#include <boost/utility.hpp>
#include <com/sun/star/uno/Reference.hxx>

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

        class UpdateDispatcher: private boost::noncopyable, private ChangeTreeAction, private data::SetVisitor
        {
        public:
            UpdateDispatcher(uno::Reference< backenduno::XUpdateHandler > const & _xUpdateHandler, rtl::OUString const & _aLocale);
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

            virtual bool handle(sharable::ValueNode * node);
            virtual bool handle(sharable::GroupNode * node);
            virtual bool handle(sharable::SetNode * node);
            virtual bool handle(sharable::TreeFragment * tree);
        private:
            sal_Int16 getUpdateAttributes(node::Attributes const & _aAttributes, bool bAdded);
            sal_Int16 getUpdateAttributeMask(node::Attributes const & _aAttributes);

            bool testReplacedAndGetName(sharable::Node * node, rtl::OUString & _aName);
        private:
            configuration::AbsolutePath const * m_pContextPath;
            uno::Reference< backenduno::XUpdateHandler >   m_xUpdateHandler;
            rtl::OUString        m_aLocale;
            rtl::OUString        m_aElementName;
            bool    m_bInValueSet;
            bool    m_bInLocalizedValues;
        };
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




