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

#pragma once
#if 1

#include "dbaccessdllapi.h"

#include <com/sun/star/embed/XStorage.hpp>

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= StorageOutputStream
    //====================================================================
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageOutputStream
    {
    public:
        StorageOutputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        virtual ~StorageOutputStream();

        /** simply calls closeOutput on our output stream, override to extend/modify this behavior
        */
        virtual void close();

    protected:
        const ::comphelper::ComponentContext&   getContext() const { return m_rContext; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >&
                                                getOutputStream() const { return m_xOutputStream; }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
              ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                                                m_xOutputStream;
    };

    //====================================================================
    //= StorageInputStream
    //====================================================================
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageInputStream
    {
    public:
        StorageInputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        virtual ~StorageInputStream();

        /** simply calls closeInput on our input stream, override to extend/modify this behavior
        */
        virtual void close();

    protected:
        const ::comphelper::ComponentContext&   getContext() const { return m_rContext; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >&
                                                getInputStream() const { return m_xInputStream; }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
              ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                                                m_xInputStream;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // STORAGESTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
