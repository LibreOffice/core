/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_
#define __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_


#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Exception.hpp>

#include <rtl/string.hxx>

//_______________________________________________
// namespace

namespace framework{

#ifndef css // conflict with define :-(
namespace css = ::com::sun::star;
#endif

//_______________________________________________
// definitions

/** @short  specify an exception, which can be used inside the
            load environment only.

    @descr  Of course outside code must wrapp it, to transport
            the occurred information to its caller.

    @author as96863
 */
class LoadEnvException
{
    //___________________________________________
    // const

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

    //___________________________________________
    // member

    public:

        /** @short  contains a suitable message, which describes the reason for this
                    exception. */
        ::rtl::OString m_sMessage;

        /** @short  An ID, which make this exception unique among others. */
        sal_Int32 m_nID;

        /** @short  Contains the original exception, if any occurred. */
        css::uno::Any m_exOriginal;

        /** TODO
             Experimental use! May it can be usefully to know, if an exception was already
             catched and handled by an interaction and was might be rethrowed! */
        sal_Bool m_bHandled;

    //___________________________________________
    // interface

    public:

        /** @short  initialize a new instance with an ID.
            @descr  Some other items of this exception
                    (e.g. a suitable message) will be generated
                    automaticly.

            @param  nID
                    One of the defined const IDs of this class.
         */
        LoadEnvException(sal_Int32 nID)
        {
            m_nID = nID;
        }

        //_______________________________________

        /** @short  initialize a new instance with an ID
                    an wrap a detected exception into this one.
            @descr  Some other items of this exception
                    (e.g. a suitable message) will be generated
                    automaticly.

            @param  nID
                    One of the defined const IDs of this class.

            @param  exUno
                    the original catched uno exception.
         */
        LoadEnvException(      sal_Int32        nID  ,
                         const css::uno::Any&   exUno)
        {
            m_nID        = nID  ;
            m_exOriginal = exUno;
        }

        //_______________________________________

        /** @short  destruct an instance of this exception.
         */
        ~LoadEnvException()
        {
            m_sMessage = ::rtl::OString();
            m_nID      = 0;
            m_bHandled = false;
            m_exOriginal.clear();
        }
};

} // namespace framework

#endif // __FRAMEWORK_LOADENV_LOADENVEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
