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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGESTREAM_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGESTREAM_HXX

#include <dbaccess/dbaccessdllapi.h>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace dbaccess
{

    // StorageOutputStream
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageOutputStream
    {
    public:
        StorageOutputStream(
            const css::uno::Reference< css::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        virtual ~StorageOutputStream();

        /** simply calls closeOutput on our output stream, override to extend/modify this behavior
        */
        virtual void close();

    protected:
        const css::uno::Reference< css::io::XOutputStream >&
                                                getOutputStream() const { return m_xOutputStream; }

    private:
              css::uno::Reference< css::io::XOutputStream >
                                                m_xOutputStream;
    };

    // StorageInputStream
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageInputStream
    {
    public:
        StorageInputStream(
            const css::uno::Reference< css::embed::XStorage >& i_rParentStorage,
            const OUString& i_rStreamName
        );
        virtual ~StorageInputStream();

    protected:
        const css::uno::Reference< css::io::XInputStream >&
                                                getInputStream() const { return m_xInputStream; }

    private:
              css::uno::Reference< css::io::XInputStream >
                                                m_xInputStream;
    };

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_RECOVERY_STORAGESTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
