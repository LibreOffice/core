/*************************************************************************
 *
 *  $RCSfile: elementimpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#ifndef CONFIGMGR_API_ELEMENTIMPL_HXX_
#define CONFIGMGR_API_ELEMENTIMPL_HXX_

#ifndef CONFIGMGR_APITYPES_HXX_
#include "apitypes.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATEINSTANCE_HPP_
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = ::com::sun::star::uno;

    /* implementations of the interfaces supported by a (parent) node
        within the configuration tree.
        (read-only operation)
    */
    namespace configapi
    {
    //-------------------------------------------------------------------------
        class NodeAccess;
        class NodeElement;
        class InnerElement;
        class TreeElement;
        class SetElement;
        class RootElement;
        class UpdateRootElement;
    //-------------------------------------------------------------------------

        // XComponent and XInterface (EOL handling)
        //---------------------------------------------------------------------
        void implDispose( TreeElement& rElement)
            throw(uno::RuntimeException);

        void implDisposeObject( NodeAccess& aNode, TreeElement& rElement)
            throw(uno::RuntimeException);

        void implDisposeObject( NodeAccess& aNode, InnerElement& rElement)
            throw(uno::RuntimeException);

        // XTypeProvider
        //---------------------------------------------------------------------
        uno::Sequence<sal_Int8> implGetImplementationId(NodeAccess& rNode,  NodeElement& rElement ) throw(uno::RuntimeException);

        // XChild
        //---------------------------------------------------------------------
        uno::Reference< uno::XInterface > implGetParent(NodeAccess& rNode, InnerElement& rElement)
            throw(uno::RuntimeException);

        uno::Reference< uno::XInterface > implGetParent(NodeAccess& rNode, SetElement& rElement)
            throw(uno::RuntimeException);

        void implSetParent(NodeAccess& rNode, InnerElement& rElement, const uno::Reference< uno::XInterface >& Parent ) // generally not supported
            throw(css::lang::NoSupportException, css::uno::RuntimeException);

        void implSetParent(NodeAccess& rNode, SetElement& rElement, const uno::Reference< uno::XInterface >& Parent ) // maybe supported
            throw(css::lang::NoSupportException, css::uno::RuntimeException);

        // XNamed
        //---------------------------------------------------------------------
        OUString implGetName(NodeAccess& rNode, NodeElement& rElement)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, NodeElement& rElement, const OUString& aName ) // generally not supported (! - missing exception)
            throw(uno::RuntimeException);

        void implSetName(NodeAccess& rNode, SetElement& rElement, const OUString& aName ) // maybe supported
            throw(uno::RuntimeException);

        // XServiceInfo
        //---------------------------------------------------------------------
        OUString implGetImplementationName( NodeAccess& rNode, NodeElement& rElement )
            throw(uno::RuntimeException);

        sal_Bool implSupportsService( NodeAccess& rNode, NodeElement& rElement, const OUString& ServiceName )
            throw(uno::RuntimeException);

        uno::Sequence< OUString > implGetSupportedServiceNames( NodeAccess& rNode, NodeElement& rElement )
            throw(uno::RuntimeException);

    // Root only only
    //-------------------------------------------------------------------------

        // XLocalizable
        //---------------------------------------------------------------------

        css::lang::Locale implGetLocale( RootElement& rElement )
                throw(uno::RuntimeException);

        void implSetLocale( RootElement& rElement, const css::lang::Locale& eLocale )
                throw(uno::RuntimeException);

        // XChangesBatch
        //---------------------------------------------------------------------

        void implCommitChanges( UpdateRootElement& rElement )
                throw(css::lang::WrappedTargetException, uno::RuntimeException);

        sal_Bool implHasPendingChanges( TreeElement& rElement )
                throw(uno::RuntimeException);

        uno::Sequence< css::util::ElementChange > implGetPendingChanges( TreeElement& rElement )
                throw(uno::RuntimeException);

    // Set only
    //-------------------------------------------------------------------------
        // XTemplateInstance
        //---------------------------------------------------------------------
        OUString implGetTemplateName(SetElement& rElement)
            throw(uno::RuntimeException);

        // XUnoTunnel
        //---------------------------------------------------------------------
        sal_Int64 implGetSomething(SetElement& rElement, const uno::Sequence< sal_Int8 >& aIdentifier )
            throw(uno::RuntimeException);

    //-------------------------------------------------------------------------
    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


