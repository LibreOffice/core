/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_description.hxx,v $
 * $Revision: 1.5 $
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

#if !defined INCLUDED_DESKTOP_EXTENSION_DESCRIPTION_HXX
#define INCLUDED_DESKTOP_EXTENSION_DESCRIPTION_HXX



#include "com/sun/star/uno/Reference.hxx"

#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XNode.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "cppuhelper/implbase2.hxx"


namespace css = ::com::sun::star;

namespace dp_registry {
namespace backend {
namespace bundle {

/**The class uses the UCB to access the description.xml file in an
   extension. The UCB must have been initialized already. It also
   requires that the extension has already be unzipped to a particular
   location.
 */
class ExtensionDescription
{
public:
    /**throws an exception if the description.xml is not
        available, cannot be read, does not contain the expected data,
        or any other error occured. Therefore it shoult only be used with
        new extensions.

        Throws com::sun::star::uno::RuntimeException,
        com::sun::star::deployment::DeploymentException,
        dp_registry::backend::bundle::NoDescriptionException.
     */
    ExtensionDescription(
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        const ::rtl::OUString& installDir,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv);

    ~ExtensionDescription();

    css::uno::Reference<css::xml::dom::XNode> getRootElement() const
    {
        return m_xRoot;
    }

    ::rtl::OUString getExtensionRootUrl() const
    {
        return m_sExtensionRootUrl;
    }


private:
    css::uno::Reference<css::xml::dom::XNode> m_xRoot;
    ::rtl::OUString m_sExtensionRootUrl;
};

class NoDescriptionException
{
};

class FileDoesNotExistFilter
    : public ::cppu::WeakImplHelper2< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler >

{
    //css::uno::Reference<css::task::XInteractionHandler> m_xHandler;
    bool m_bExist;
    css::uno::Reference< css::ucb::XCommandEnvironment > m_xCommandEnv;

public:
    virtual ~FileDoesNotExistFilter();
    FileDoesNotExistFilter(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv);

    bool exist();
    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (css::uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);
};


} // namespace bundle
} // namespace backend
} // namespace dp_registry


#endif // INCLUDED_DESKTOP_LICENSE_INTERACT_HXX
