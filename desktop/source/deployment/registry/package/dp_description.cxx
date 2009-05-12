/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_description.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_description.hxx"

#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/xml/dom/XDocumentBuilder.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "com/sun/star/ucb/IOErrorCode.hpp"

#include "com/sun/star/beans/PropertyValue.hpp"


#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;

namespace dp_registry {
namespace backend {
namespace bundle {

ExtensionDescription::ExtensionDescription(
    const cssu::Reference<cssu::XComponentContext>& xContext,
    const ::rtl::OUString& installDir,
    const cssu::Reference< css::ucb::XCommandEnvironment >& xCmdEnv)
{
    try {
        m_sExtensionRootUrl = installDir;
        //may throw ::com::sun::star::ucb::ContentCreationException
        //If there is no description.xml then ucb will start an interaction which
        //brings up a dialog.We want to prevent this. Therefore we wrap the xCmdEnv
        //and filter the respective exception out.
        ::rtl::OUString sDescriptionUri(installDir + OUSTR("/description.xml"));
        cssu::Reference<css::ucb::XCommandEnvironment> xFilter =
            static_cast<css::ucb::XCommandEnvironment*>(
                new FileDoesNotExistFilter(xCmdEnv));
        ::ucbhelper::Content descContent(sDescriptionUri, xFilter);

        //throws an com::sun::star::uno::Exception if the file is not available
        cssu::Reference<css::io::XInputStream> xIn;
        try
        {   //throws com.sun.star.ucb.InteractiveAugmentedIOException
            xIn = descContent.openStream();
        }
        catch (cssu::Exception& )
        {
            if ( ! static_cast<FileDoesNotExistFilter*>(xFilter.get())->exist())
                throw NoDescriptionException();
            throw;
        }
        if (!xIn.is())
        {
            throw cssu::Exception(
                OUSTR("Could not get XInputStream for description.xml of extension ") +
                sDescriptionUri, 0);
        }

        //get root node of description.xml
        cssu::Reference<css::xml::dom::XDocumentBuilder> xDocBuilder(
            xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.xml.dom.DocumentBuilder"),
                xContext ), cssu::UNO_QUERY);
        if (!xDocBuilder.is())
            throw css::uno::Exception(OUSTR(" Could not create service com.sun.star.xml.dom.DocumentBuilder"), 0);

        if (xDocBuilder->isNamespaceAware() == sal_False)
        {
            throw cssu::Exception(
                OUSTR("Service com.sun.star.xml.dom.DocumentBuilder is not namespace aware."), 0);
        }

        cssu::Reference<css::xml::dom::XDocument> xDoc = xDocBuilder->parse(xIn);
        if (!xDoc.is())
        {
            throw cssu::Exception(sDescriptionUri + OUSTR(" contains data which cannot be parsed. "), 0);
        }

        //check for proper root element and namespace
        cssu::Reference<css::xml::dom::XElement> xRoot = xDoc->getDocumentElement();
        if (!xRoot.is())
        {
            throw cssu::Exception(
                sDescriptionUri + OUSTR(" contains no root element."), 0);
        }

        if ( ! xRoot->getTagName().equals(OUSTR("description")))
        {
            throw cssu::Exception(
                sDescriptionUri + OUSTR(" does not contain the root element <description>."), 0);
        }

        m_xRoot = cssu::Reference<css::xml::dom::XNode>(
            xRoot, cssu::UNO_QUERY_THROW);
        ::rtl::OUString nsDescription = xRoot->getNamespaceURI();

        //check if this namespace is supported
        if ( ! nsDescription.equals(OUSTR("http://openoffice.org/extensions/description/2006")))
        {
            throw cssu::Exception(sDescriptionUri + OUSTR(" contains a root element with an unsupported namespace. "), 0);
        }
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::deployment::DeploymentException &) {
        throw;
    } catch (css::uno::Exception & e) {
        css::uno::Any a(cppu::getCaughtException());
        throw css::deployment::DeploymentException(
            e.Message, css::uno::Reference< css::uno::XInterface >(), a);
    }
}

ExtensionDescription::~ExtensionDescription()
{
}

//======================================================================
FileDoesNotExistFilter::FileDoesNotExistFilter(
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv):
    m_bExist(true), m_xCommandEnv(xCmdEnv)
{}

FileDoesNotExistFilter::~FileDoesNotExistFilter()
{
};

bool FileDoesNotExistFilter::exist()
{
    return m_bExist;
}
    // XCommandEnvironment
cssu::Reference<css::task::XInteractionHandler >
    FileDoesNotExistFilter::getInteractionHandler() throw (css::uno::RuntimeException)
{
    return static_cast<css::task::XInteractionHandler*>(this);
}

cssu::Reference<css::ucb::XProgressHandler >
    FileDoesNotExistFilter::getProgressHandler() throw (css::uno::RuntimeException)
{
    return m_xCommandEnv.is()
        ? m_xCommandEnv->getProgressHandler()
        : cssu::Reference<css::ucb::XProgressHandler>();
}

// XInteractionHandler
//If the interaction was caused by a non-existing file which is specified in the ctor
//of FileDoesNotExistFilter, then we do nothing
void  FileDoesNotExistFilter::handle(
        cssu::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException)
{
    cssu::Any request( xRequest->getRequest() );

    css::ucb::InteractiveAugmentedIOException ioexc;
    if ((request>>= ioexc) && ioexc.Code == css::ucb::IOErrorCode_NOT_EXISTING )
    {
        m_bExist = false;
        return;
    }
    css::uno::Reference<css::task::XInteractionHandler> xInteraction;
    if (m_xCommandEnv.is()) {
        xInteraction = m_xCommandEnv->getInteractionHandler();
    }
    if (xInteraction.is()) {
        xInteraction->handle(xRequest);
    }
}


} // namespace bundle
} // namespace backend
} // namespace dp_registry

