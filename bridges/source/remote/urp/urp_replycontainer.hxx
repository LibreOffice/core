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
#include <hash_map>
#include <list>

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include "urp_threadid.hxx"

namespace bridges_urp
{
    class ClientJob;
    typedef ::std::hash_map< ::rtl::ByteSequence ,
                             ::std::list < ClientJob * > ,
                             HashThreadId ,
                             EqualThreadId > Id2ClientJobStackMap;

    class urp_ClientJobContainer
    {
    public:
        void add( const ::rtl::ByteSequence &id , ClientJob *p )
        {
            ::osl::MutexGuard guard( m_mutex );
            m_map[id].push_back( p );
        }

        ClientJob *remove( const ::rtl::ByteSequence & id )
        {
            ::osl::MutexGuard guard( m_mutex );
            Id2ClientJobStackMap::iterator ii = m_map.find( id );

            ClientJob *p = 0;
            if( ii != m_map.end() )
            {
                p = (*ii).second.back();
                (*ii).second.pop_back();
                if( (*ii).second.empty() )
                {
                    m_map.erase( ii );
                }
            }

            return p;
        }

    private:
        ::osl::Mutex m_mutex;
        Id2ClientJobStackMap m_map;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
