/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_INC_JOBS_HELPONSTARTUP_HXX
#define INCLUDED_FRAMEWORK_INC_JOBS_HELPONSTARTUP_HXX

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

namespace framework{

/** @short  implements a job component, which handle the special
            feature to show a suitable help page for every (visible!)
            loaded document.

    @author as96863
 */
class HelpOnStartup : public ::cppu::WeakImplHelper< css::lang::XServiceInfo,css::lang::XEventListener,css::task::XJob >
{

    // member
    private:
        osl::Mutex m_mutex;

        /** @short  reference to an uno service manager. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** @short  such module manager is used to classify new opened documents. */
        css::uno::Reference< css::frame::XModuleManager2 > m_xModuleManager;

        /** @short  is needed to locate a might open help frame. */
        css::uno::Reference< css::frame::XDesktop2 > m_xDesktop;

        /** @short  provides read access to the underlying configuration. */
        css::uno::Reference< css::container::XNameAccess > m_xConfig;

        /** @short  knows the current locale of this office session,
                    which is needed to build complete help URLs.
         */
        OUString m_sLocale;

        /** @short  knows the current operating system of this office session,
                    which is needed to build complete help URLs.
         */
        OUString m_sSystem;

    // native interface
    public:

        /** @short  create new instance of this class.

            @param  xContext
                    reference to the uno service manager, which created this instance.
                    Can be used later to create own needed uno resources on demand.
         */
        HelpOnStartup(const css::uno::Reference< css::uno::XComponentContext >& xContext);

        /** @short  does nothing real ...

            @descr  But it should exists as virtual function,
                    so this class can't make trouble
                    related to inline/symbols etcpp.!
         */
        virtual ~HelpOnStartup();

    // uno interface
    public:

        // css.lang.XServiceInfo
        DECLARE_XSERVICEINFO

        // css.task.XJob
        virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
            throw(css::lang::IllegalArgumentException,
                  css::uno::Exception                ,
                  css::uno::RuntimeException, std::exception         ) override;

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException, std::exception) override;

    // helper
    private:

        /** @short  analyze the given job arguments, try to locate a model reference
                    and try to classify this model.

            @descr  As a result of this operation a module identifier will be returned.
                    It can be used against the module configuration then to retrieve further information.

            @param  lArguments
                    the list of job arguments which is given on our interface method execute().

            @return [string]
                    a module identifier ... or an empty value if no model could be located ...
                    or if it could not be classified successfully.
         */
        OUString its_getModuleIdFromEnv(const css::uno::Sequence< css::beans::NamedValue >& lArguments);

        /** @short  tries to locate the open help module and return
                    the url of the currently shown help content.

            @descr  It returns an empty string, if the help isn't still
                    open at calling time.

            @return The URL of the current shown help content;
                    or an empty value if the help isn't still open.
         */
        OUString its_getCurrentHelpURL();

        /** @short  checks if the given help url match to a default help url
                    of any office module.

            @param  sHelpURL
                    the help url for checking.

            @return [bool]
                    sal_True if the given URL is any default one ...
                    sal_False otherwise.
         */
        bool its_isHelpUrlADefaultOne(const OUString& sHelpURL);

        /** @short  checks, if the help module should be shown automatically for the
                    currently opened office module.

            @descr  This value is readed from the module configuration.
                    In case the help should be shown, this method returns
                    a help URL, which can be used to show the right help content.

            @param  sModule
                    identifies the used office module.

            @return [string]
                    A valid help URL in case the help content should be shown;
                    an empty value if such automatism was disabled for the specified office module.
         */
        OUString its_checkIfHelpEnabledAndGetURL(const OUString& sModule);

        /** @short  create a help URL for the given parameters.

            @param  sBaseURL
                    must be the base URL for a requested help content
                    e.g. "vnd.sun.star.help://swriter/"
                    or   "vnd.sun.star.help://swriter/67351"

            @param  sLocale
                    the current office locale
                    e.g. "en-US"

            @param  sSystem
                    the current operating system
                    e.g. "WIN"

            @return The URL which was generated.
                    e.g.
                    e.g. "vnd.sun.star.help://swriter/?Language=en-US&System=WIN"
                    or   "vnd.sun.star.help://swriter/67351?Language=en-US&System=WIN"
         */
        static OUString ist_createHelpURL(const OUString& sBaseURL,
                                                 const OUString& sLocale ,
                                                 const OUString& sSystem );
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_JOBS_HELPONSTARTUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
