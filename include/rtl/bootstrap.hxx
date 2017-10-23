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
#ifndef INCLUDED_RTL_BOOTSTRAP_HXX
#define INCLUDED_RTL_BOOTSTRAP_HXX

#include "sal/config.h"

#include <cstddef>

#include "rtl/ustring.hxx"
#include "rtl/bootstrap.h"

namespace rtl
{
    class Bootstrap
    {
        void * _handle;

        Bootstrap( Bootstrap const & ) SAL_DELETED_FUNCTION;
        Bootstrap & operator = ( Bootstrap const & ) SAL_DELETED_FUNCTION;

    public:
        /**
         * @see rtl_bootstrap_setIniFileName()
         */
        static inline void SAL_CALL setIniFilename( const ::rtl::OUString &sFileUri );

        /** Retrieves a bootstrap parameter
           @param sName name of the bootstrap value. case insensitive.
           @param[out] outValue On success contains the value, otherwise
                  an empty string.
           @return false, if no value could be retrieved, otherwise true
           @see rtl_bootstrap_get()
         */
        static inline bool get(
            const ::rtl::OUString &sName,
            ::rtl::OUString &outValue );

        /** Retrieves a bootstrap parameter

           @param sName name of the bootstrap value. case insensitive.
           @param[out] outValue Contains the value associated with <code>sName</code>.
           @param aDefault if none of the other methods retrieved a value,
                           <code>outValue</code> is assigned to <code>aDefault</code>.

           @see rtl_bootstrap_get()
         */
        static inline void get(
            const ::rtl::OUString &sName,
            ::rtl::OUString &outValue,
            const ::rtl::OUString &aDefault );

        /** Sets a bootstrap parameter.

            @param name
                   name of bootstrap parameter
            @param value
                   value of bootstrap parameter

            @see rtl_bootstrap_set()
        */
        static inline void set( ::rtl::OUString const & name, ::rtl::OUString const & value );

        /** default ctor.
         */
        inline Bootstrap();

        /** Opens a bootstrap argument container
            @see rtl_bootstrap_args_open()
         */
        inline Bootstrap(const rtl::OUString & iniName);

        /** Closes a bootstrap argument container
            @see rtl_bootstrap_args_close()
        */
        inline ~Bootstrap();

        /** Retrieves a bootstrap argument.

            It is first tried to retrieve the value via the global function
            and second via the special bootstrap container.
            @see rtl_bootstrap_get_from_handle()
        */

        inline bool getFrom(const ::rtl::OUString &sName,
                                ::rtl::OUString &outValue) const;

        /** Retrieves a bootstrap argument.

            It is first tried to retrieve the value via the global function
            and second via the special bootstrap container.
            @see rtl_bootstrap_get_from_handle()
        */
        inline void getFrom(const ::rtl::OUString &sName,
                            ::rtl::OUString &outValue,
                            const ::rtl::OUString &aDefault) const;

        /** Retrieves the name of the underlying ini-file.
            @see rtl_bootstrap_get_iniName_from_handle()
         */
        inline void getIniName(::rtl::OUString & iniName) const;

        /** Expands a macro using bootstrap variables.

            @param[in,out] macro The macro to be expanded
        */
        void expandMacrosFrom( ::rtl::OUString & macro ) const
            { rtl_bootstrap_expandMacros_from_handle( _handle, &macro.pData ); }

        /** Expands a macro using default bootstrap variables.

            @param[in,out] macro The macro to be expanded
        */
        static void expandMacros( ::rtl::OUString & macro )
            { rtl_bootstrap_expandMacros( &macro.pData ); }

        /** Provides the bootstrap internal handle.

            @return bootstrap handle
        */
        rtlBootstrapHandle getHandle() const
            { return _handle; }

        /** Escapes special characters ("$" and "\").

            @param value
            an arbitrary value

            @return
            the given value, with all occurrences of special characters ("$" and
            "\") escaped

            @since UDK 3.2.9
        */
        static inline ::rtl::OUString encode( ::rtl::OUString const & value );
    };


    // IMPLEMENTATION

    inline void Bootstrap::setIniFilename( const ::rtl::OUString &sFile )
    {
        rtl_bootstrap_setIniFileName( sFile.pData );
    }

    inline bool Bootstrap::get( const ::rtl::OUString &sName,
                                    ::rtl::OUString & outValue )
    {
        return rtl_bootstrap_get( sName.pData , &(outValue.pData) , NULL );
    }

    inline void Bootstrap::get( const ::rtl::OUString &sName,
                                ::rtl::OUString & outValue,
                                const ::rtl::OUString & sDefault )
    {
        rtl_bootstrap_get( sName.pData , &(outValue.pData) , sDefault.pData );
    }

    inline void Bootstrap::set( ::rtl::OUString const & name, ::rtl::OUString const & value )
    {
        rtl_bootstrap_set( name.pData, value.pData );
    }

    inline Bootstrap::Bootstrap()
    {
        _handle = NULL;
    }

    inline Bootstrap::Bootstrap(const rtl::OUString & iniName)
    {
        if(!iniName.isEmpty())
            _handle = rtl_bootstrap_args_open(iniName.pData);

        else
            _handle = NULL;
    }

    inline Bootstrap::~Bootstrap()
    {
        rtl_bootstrap_args_close(_handle);
    }


    inline bool Bootstrap::getFrom(const ::rtl::OUString &sName,
                                       ::rtl::OUString &outValue) const
    {
        return rtl_bootstrap_get_from_handle(_handle, sName.pData, &outValue.pData, NULL);
    }

    inline void Bootstrap::getFrom(const ::rtl::OUString &sName,
                                   ::rtl::OUString &outValue,
                                   const ::rtl::OUString &aDefault) const
    {
        rtl_bootstrap_get_from_handle(_handle, sName.pData, &outValue.pData, aDefault.pData);
    }

    inline void Bootstrap::getIniName(::rtl::OUString & iniName) const
    {
        rtl_bootstrap_get_iniName_from_handle(_handle, &iniName.pData);
    }

    inline ::rtl::OUString Bootstrap::encode( ::rtl::OUString const & value )
    {
        ::rtl::OUString encoded;
        rtl_bootstrap_encode(value.pData, &encoded.pData);
        return encoded;
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
