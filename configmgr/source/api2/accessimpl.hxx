/*************************************************************************
 *
 *  $RCSfile: accessimpl.hxx,v $
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

#ifndef CONFIGMGR_API_BASEACCESSIMPL_HXX_
#define CONFIGMGR_API_BASEACCESSIMPL_HXX_

#ifndef CONFIGMGR_APITYPES_HXX_
#include "apitypes.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATEINSTANCE_HPP_
#include <com/sun/star/configuration/XTemplateInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATECONTAINER_HPP_
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTY_HPP_
#include <com/sun/star/beans/XProperty.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGESCAPE_HPP_
#include <com/sun/star/util/XStringEscape.hpp>
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
        class NodeAccess;
        class NodeSetInfoAccess;
        class NodeGroupInfoAccess;

        // XHierarchicalName
        OUString implGetHierarchicalName(NodeAccess& rNode)
            throw(uno::RuntimeException);

        OUString implComposeHierarchicalName(NodeAccess& rNode, const OUString& aRelativeName )
            throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, uno::RuntimeException);

        // XElementAccess, base class of XNameAccess
        uno::Type implGetElementType(NodeGroupInfoAccess& rNode)
            throw(uno::RuntimeException);

        uno::Type implGetElementType(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

        sal_Bool implHasElements(NodeGroupInfoAccess& rNode)
            throw(uno::RuntimeException);

        sal_Bool implHasElements(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

        // XNameAccess
        uno::Any implGetByName(NodeAccess& rNode, const OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        uno::Sequence< OUString > implGetElementNames(NodeAccess& rNode)
            throw( uno::RuntimeException);

        sal_Bool implHasByName(NodeAccess& rNode, const OUString& aName )
            throw(uno::RuntimeException);

        // XHierarchicalNameAccess
        uno::Any implGetByHierarchicalName(NodeAccess& rNode, const OUString& aName )
            throw(css::container::NoSuchElementException, uno::RuntimeException);

        sal_Bool implHasByHierarchicalName(NodeAccess& rNode, const OUString& aName )
            throw(uno::RuntimeException);

        // XExactName
        OUString implGetExactName(NodeAccess& rNode, const OUString& aApproximateName )
            throw(uno::RuntimeException);

        // XProperty
        css::beans::Property implGetAsProperty(NodeAccess& rNode)
            throw(uno::RuntimeException);

        // set-specific interfaces

        // XTemplateContainer
        OUString SAL_CALL implGetElementTemplateName(NodeSetInfoAccess& rNode)
            throw(uno::RuntimeException);

        // XStringEscape
        OUString SAL_CALL implEscapeString(NodeAccess& rNode, const OUString& aString)
            throw(css::lang::IllegalArgumentException, uno::RuntimeException);

        OUString SAL_CALL implUnescapeString(NodeAccess& rNode, const OUString& aEscapedString)
            throw(css::lang::IllegalArgumentException, uno::RuntimeException);
    }

}
#endif // CONFIGMGR_API_BASEACCESSIMPL_HXX_


