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

#ifndef INCLUDED_EXTENSIONS_SOURCE_LOGGING_METHODGUARD_HXX
#define INCLUDED_EXTENSIONS_SOURCE_LOGGING_METHODGUARD_HXX


namespace logging
{
    template < class COMPONENT >
    class ComponentMethodGuard
    {
    private:
        COMPONENT&  m_rHandler;

    public:
        class Access
        {
        private:
            friend class ComponentMethodGuard;
            Access() { }
        };

    public:
        explicit ComponentMethodGuard( COMPONENT& _rHandler )
            :m_rHandler( _rHandler )
        {
            m_rHandler.enterMethod( Access() );
        }
        ~ComponentMethodGuard()
        {
            m_rHandler.leaveMethod( Access() );
        }
    };


} // namespace logging


#endif // INCLUDED_EXTENSIONS_SOURCE_LOGGING_METHODGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
