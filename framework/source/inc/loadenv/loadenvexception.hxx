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

#ifndef __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_
#define __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Exception.hpp>

#include <rtl/string.hxx>

namespace framework{

/** @short  specify an exception, which can be used inside the
            load environment only.

    @descr  Of course outside code must wrapp it, to transport
            the occurred information to its caller.
 */
class LoadEnvException
{
    public:
        /** @short  Can be used as an ID for an instance of a LoadEnvException.
            @descr  To prevent errors on adding/removing/changing such IDs here,
                    an enum field is used. Its int values are self organized ...
         */
        enum EIDs
        {
            /** @short  The specified URL/Stream/etcpp. can not be handled by a LoadEnv instance. */
            ID_UNSUPPORTED_CONTENT,

            /** @short  It was not possible to get access to global filter configuration.
                @descr  Might som neccsessary services could not be created. */
            ID_NO_CONFIG_ACCESS,

            /** @short  Some data obtained from the filter configuration seems to incorrect.
                @descr  Might a filter-type relation ship seem to be damaged. */
            ID_INVALID_FILTER_CONFIG,

            /** @short  indicates a corrupted media descriptor.
                @descr  Some parts are required - some other ones are optional. Such exception
                        should be thrown, if a required item does not exists. */
            ID_INVALID_MEDIADESCRIPTOR,

            /** @short  Its similar to an uno::RuntimeException ....
                @descr  But such runtime exception can break the whole office code.
                        So its capsulated to this specialized load environment only.
                        Mostly it indicates a missing but needed resource ... e.g the
                        global desktop reference! */
            ID_INVALID_ENVIRONMENT,

            /** @short  indicates a failed search for the right target frame. */
            ID_NO_TARGET_FOUND,

            /** @short  An already existing document was found inside a target frame.
                        But its controller could not be suspended successfully. Thats
                        why the new load request was cancelled. The document could not
                        be replaced. */
            ID_COULD_NOT_SUSPEND_CONTROLLER,

            /** @short  TODO */
            ID_COULD_NOT_REACTIVATE_CONTROLLER,

            /** @short  inidcates an already running load operation. Of yourse the same
                        instance cant be used for multiple load requests at the same time.
             */
            ID_STILL_RUNNING,

            /** @short  sometiems we cant specify the reason for an error, because we
                        was interrupted by an called code in an unexpected way ...
             */
            ID_GENERAL_ERROR
        };

        sal_Int32 m_nID;
        OUString m_sMessage;
        css::uno::Any m_exOriginal;

        LoadEnvException(
            sal_Int32 id, OUString const & message = OUString(),
            css::uno::Any const & original = css::uno::Any()):
            m_nID(id), m_sMessage(message), m_exOriginal(original)
        {}

        ~LoadEnvException() {}
};

} // namespace framework

#endif // __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
