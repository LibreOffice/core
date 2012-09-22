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
#ifndef INCLUDED_unotools_CMDOPTIONS_HXX
#define INCLUDED_unotools_CMDOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/*-************************************************************************************************************//**
    @descr          The method GetList() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtCommandOptions_Impl;

/*-************************************************************************************************************//**
    @short          collect informations about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SvtCommandOptions: public utl::detail::Options
{
    friend class SvtCommandOptions_Impl;

    public:

        enum CmdOption
        {
            CMDOPTION_DISABLED,
            CMDOPTION_NONE
        };

        /*-****************************************************************************************************//**
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         SvtCommandOptions();
        virtual ~SvtCommandOptions();

        /*-****************************************************************************************************//**
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @param      "eOption" select the list to retrieve.
            @return     A list of command strings is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        sal_Bool HasEntries( CmdOption eOption ) const;

        /*-****************************************************************************************************//**
            @short      Lookup if a command URL is inside a given list
            @descr      Lookup if a command URL is inside a given lst

            @seealso    -

            @param      "eOption" select right command list
            @param      "aCommandURL" a command URL that is used for the look up
            @return     "sal_True" if the command is inside the list otherwise "sal_False"

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool Lookup( CmdOption eOption, const OUString& aCommandURL ) const;

        /*-****************************************************************************************************//**
            @short      register an office frame, which must update its dispatches if
                        the underlying configuration was changed.

            @descr      To avoid using of "dead" frame objects or implementing
                        deregistration mechanism too, we use weak references to
                        the given frames.

            @param      "xFrame"            points to the frame, which wish to be
                                            notified, if configuration was changed.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void EstablisFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);

    private:

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtCommandOptions_Impl*  m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtCmdOptions

#endif  // #ifndef INCLUDED_unotools_CMDOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
