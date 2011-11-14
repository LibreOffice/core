/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _DESKTOP_CONFIGINIT_HXX_
#define _DESKTOP_CONFIGINIT_HXX_

#include "sal/config.h"

#include <com/sun/star/task/XInteractionHandler.hpp>

/**
    sets an InteractionHandler for configuration errors in the current context.

  */
    class ConfigurationErrorHandler
    {
    public:
        typedef com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > InteractionHandler;

        /// Constructor: Uses the default interaction handler
        ConfigurationErrorHandler()
        : m_pContext(0), m_xHandler()
        {}

        /// Constructor: Uses an externally provided interaction handler
        ConfigurationErrorHandler(const InteractionHandler & xHandler)
        : m_pContext(0), m_xHandler( xHandler )
        {}

        ~ConfigurationErrorHandler();

        static InteractionHandler getDefaultInteractionHandler();

        /// installs the handler into the current context
        void activate();
        /// deinstalls the handler from the current context, restoring the previous context
        void deactivate();
    private:
        class Context;
        Context * m_pContext;
        InteractionHandler m_xHandler;
    private:
        // not implemented - suppress copy
        ConfigurationErrorHandler(const ConfigurationErrorHandler&);
        void operator=(const ConfigurationErrorHandler&);
    };

//-----------------------------------------------------------------------------
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
