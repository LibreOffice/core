/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_bridgeimpl.hxx,v $
 * $Revision: 1.10 $
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
#ifndef _URP_BRIDGEIMPL_HXX_
#define _URP_BRIDGEIMPL_HXX_

#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

#ifndef _UNO_THREADPOOL_H_
#include <uno/threadpool.h>
#endif
#include <bridges/remote/bridgeimpl.hxx>

#ifndef _URP_CACHE_HXX_
#include "urp_cache.hxx"
#endif
#include "urp_marshal_decl.hxx"

#ifndef _URP_REPLYCONTAINER_HXX_
#include "urp_replycontainer.hxx"
#endif
#include "urp_property.hxx"


namespace bridges_urp
{

class PropertyObject;

struct equalOUString
{
    sal_Int32 operator() ( const ::rtl::OUString &s1, const ::rtl::OUString &s2 ) const
        {
            return s1 == s2;
        }
};

struct equalType
{
    sal_Int32 operator() ( const ::com::sun::star::uno::Type &t1,
                           const ::com::sun::star::uno::Type &t2 ) const
        {
            return t1 == t2;
        }
};

class OWriterThread;
class OReaderThread;

struct urp_BridgeImpl :
    public remote_BridgeImpl
{
    urp_BridgeImpl( sal_Int32 nCacheSize , sal_uInt32 nInitialMarshalerSize );
    ~urp_BridgeImpl();

    void applyProtocolChanges( const Properties & );

    void startBlockBridge();
    void stopBlockBridge();
    void addError( char const *pError );
    void addError( const ::rtl::OUString &anError );
    void dumpErrors( FILE *f );
    ::rtl::OUString getErrorsAsString();

    ::osl::Mutex m_marshalingMutex;
    ::osl::Mutex m_disposingMutex;
    ::osl::Mutex m_errorListMutex;
    Marshal m_blockMarshaler;
    sal_Int32 m_nMarshaledMessages;

      // Caches for vars, that go from local process to the remote process
      Cache < ::rtl::OUString , equalOUString >             m_oidCacheOut;
       Cache < ::rtl::ByteSequence , EqualThreadId >         m_tidCacheOut;
       Cache < ::com::sun::star::uno::Type , equalType >     m_typeCacheOut;

    ::com::sun::star::uno::Type m_lastOutType;
    ::rtl::ByteSequence m_lastOutTid;
    ::rtl::OUString m_lastOutOid;

      // Caches for vars, that come from the remote process to the local process
    ::rtl::OUString             *m_pOidIn;
    ::rtl::ByteSequence         *m_pTidIn;
    ::com::sun::star::uno::Type *m_pTypeIn;

    ::com::sun::star::uno::Type m_lastInType;
    ::rtl::ByteSequence m_lastInTid;
    ::rtl::OUString m_lastInOid;

    urp_ClientJobContainer m_clientJobContainer;

    OWriterThread *m_pWriter;
    OReaderThread *m_pReader;
    ::rtl::OString m_sLogFileName;
    FILE          *m_pLogFile;
    ::osl::Condition m_initialized;
    ::osl::Condition m_cndWaitForThreads;

    struct Properties m_properties;
    class PropertyObject *m_pPropertyObject;
    ::std::list< ::rtl::OUString > m_lstErrors;
    uno_ThreadPool m_hThreadPool;
};


}
#endif
