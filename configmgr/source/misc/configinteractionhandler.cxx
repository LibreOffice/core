/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configinteractionhandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:09:15 $
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

#include "configinteractionhandler.hxx"

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <uno/current_context.hxx>
#include <map>

namespace configmgr {
namespace apihelper {
//=========================================================================
    namespace uno  = com::sun::star::uno;
    namespace task = com::sun::star::task;
    using rtl::OUString;
//=========================================================================
#define OU2A(rtlOUString)   (::rtl::OUStringToOString((rtlOUString), RTL_TEXTENCODING_ASCII_US).getStr())
#define OUSTR(txt)  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(txt) )

//========================================================================
#define CONFIG_ERROR_HANDLER "configuration.interaction-handler"
//========================================================================

namespace
{
    typedef uno::Reference< uno::XCurrentContext > CurrentContext;
    class SimpleCurrentContext : public cppu::WeakImplHelper1< uno::XCurrentContext >
    {
        CurrentContext m_xChainedContext;
        typedef std::map< OUString, uno::Any > Settings;
        Settings m_settings;
    public:
        explicit
        SimpleCurrentContext(const CurrentContext & xChainedContext)
        : m_xChainedContext(xChainedContext)
        {}

        void addSetting(const OUString & aName, const uno::Any & aValue)
        { m_settings[aName] = aValue; }

        void install()      { uno::setCurrentContext(this); }
        void deinstall()    { uno::setCurrentContext(m_xChainedContext); }

        uno::Any getChainedValueByName( OUString const & aName) const
        {
            return m_xChainedContext.is()
                            ? m_xChainedContext->getValueByName(aName)
                            : uno::Any();
        }

        // XCurrentContext
        uno::Any SAL_CALL
            getValueByName( OUString const & aName)
                throw (uno::RuntimeException);
    };

    uno::Any SAL_CALL
        SimpleCurrentContext::getValueByName( OUString const & aName)
            throw (uno::RuntimeException)
    {
        Settings::iterator it = m_settings.find(aName);
        if (it != m_settings.end())
            return it->second;
        else
            return getChainedValueByName(aName);
    }

}
//========================================================================
class ConfigurationInteractionHandler::Context : public SimpleCurrentContext
{
public:
    explicit
    Context()
    : SimpleCurrentContext( uno::getCurrentContext() )
    {
        addSetting( OUSTR(CONFIG_ERROR_HANDLER), uno::Any() );
    }

    ~Context()
    {
    }

    bool hasInteractionHandler() const
    {
        return getInteractionHandler().is();
    }

    uno::Reference< task::XInteractionHandler > getInteractionHandler() const
    {
        uno::Reference< task::XInteractionHandler > xHandler;
        getChainedValueByName( OUSTR(CONFIG_ERROR_HANDLER) ) >>= xHandler;
        return xHandler;
    }
};
//========================================================================
ConfigurationInteractionHandler::ConfigurationInteractionHandler()
: m_pContext( 0 )
{
    try
    {
        Context * pContext = new Context();
        CurrentContext xContext(pContext);

        if (pContext->hasInteractionHandler())
        {
            m_pContext = pContext;
            m_pContext->acquire();
            m_pContext->install();
        }
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("Warning - Configuration: Retrieving error (interaction) handler failed: [%s]\n", OU2A(e.Message));
    }
}
//------------------------------------------------------------------------------
ConfigurationInteractionHandler::~ConfigurationInteractionHandler()
{
    if (m_pContext)
    {
        m_pContext->deinstall();
        m_pContext->release();
    }
}

//------------------------------------------------------------------------------
void ConfigurationInteractionHandler::handle( const uno::Reference< task::XInteractionRequest > & xRequest )
{
    if (m_pContext)
    {
        uno::Reference< task::XInteractionHandler > xHandler = m_pContext->getInteractionHandler();
        if (xHandler.is()) xHandler->handle(xRequest);
    }
}
//=========================================================================

} // namespace apihelper
} // namespace configmgr
