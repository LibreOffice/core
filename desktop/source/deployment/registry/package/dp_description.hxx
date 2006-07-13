/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_description.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:06:49 $
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

#if !defined INCLUDED_DESKTOP_EXTENSION_DESCRIPTION_HXX
#define INCLUDED_DESKTOP_EXTENSION_DESCRIPTION_HXX



#include "com/sun/star/uno/Reference.hxx"

#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
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

     */
    ExtensionDescription(
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        const ::rtl::OUString& installDir,
        const css::uno::Reference< css::ucb::XCommandEnvironment >& xCmdEnv);

    ~ExtensionDescription();

    css::uno::Reference<css::xml::dom::XElement> getRootElement() const
    {
        return m_xRoot;
    }
    css::uno::Reference<css::xml::xpath::XXPathAPI> getXPathAPI() const
    {
        return m_xXPath;
    }
    ::rtl::OUString getExtensionRootUrl() const
    {
        return m_sExtensionRootUrl;
    }


private:
    ExtensionDescription(const ExtensionDescription& );
    ExtensionDescription& operator = (const ExtensionDescription& );

    css::uno::Reference<css::xml::dom::XElement> m_xRoot;
    css::uno::Reference<css::xml::xpath::XXPathAPI> m_xXPath;
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
    css::uno::Reference<css::task::XInteractionHandler> m_xInteraction;

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
