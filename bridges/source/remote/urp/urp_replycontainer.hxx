/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_replycontainer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:49:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
