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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILID_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILID_HXX

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>

namespace fileaccess {

    class TaskManager;

    class FileContentIdentifier :
        public cppu::WeakImplHelper<css::ucb::XContentIdentifier>
    {

        // This implementation has to be reworked
    public:
        FileContentIdentifier( const OUString& aUnqPath,
                               bool IsNormalized = true );

        virtual ~FileContentIdentifier() override;

        // XContentIdentifier
        virtual OUString SAL_CALL
        getContentIdentifier() override;

        virtual OUString SAL_CALL
        getContentProviderScheme() override;

    private:
        OUString m_aContentId;              // The URL string
        OUString m_aProviderScheme;
    };

} // end namespace fileaccess


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
