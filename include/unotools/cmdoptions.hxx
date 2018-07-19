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
#ifndef INCLUDED_UNOTOOLS_CMDOPTIONS_HXX
#define INCLUDED_UNOTOOLS_CMDOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>
#include <memory>

/*-************************************************************************************************************
    @descr          The method GetList() returns a list of property values.
                    Use follow defines to separate values by names.
*//*-*************************************************************************************************************/

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtCommandOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtCommandOptions : public utl::detail::Options
{
    friend class SvtCommandOptions_Impl;

    public:

        enum CmdOption
        {
            CMDOPTION_DISABLED,
            CMDOPTION_NONE
        };

         SvtCommandOptions();
        virtual ~SvtCommandOptions() override;

        /*-****************************************************************************************************
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with its names and properties.
            @param      "eOption" select the list to retrieve.
            @return     A list of command strings is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        bool HasEntries( CmdOption eOption ) const;

        /*-****************************************************************************************************
            @short      Lookup if a command URL is inside a given list
            @descr      Lookup if a command URL is inside a given lst
            @param      "eOption" select right command list
            @param      "aCommandURL" a command URL that is used for the look up
            @return     "sal_True" if the command is inside the list otherwise "sal_False"
        *//*-*****************************************************************************************************/

        bool Lookup( CmdOption eOption, const OUString& aCommandURL ) const;

        /*-****************************************************************************************************
            @short      register an office frame, which must update its dispatches if
                        the underlying configuration was changed.

            @descr      To avoid using of "dead" frame objects or implementing
                        deregistration mechanism too, we use weak references to
                        the given frames.

            @param      "xFrame"            points to the frame, which wish to be
                                            notified, if configuration was changed.
        *//*-*****************************************************************************************************/

        void EstablishFrameCallback(const css::uno::Reference< css::frame::XFrame >& xFrame);

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods aren't safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:
        std::shared_ptr<SvtCommandOptions_Impl>  m_pImpl;

};      // class SvtCmdOptions

#endif // INCLUDED_UNOTOOLS_CMDOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
