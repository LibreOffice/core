/*************************************************************************
 *
 *  $RCSfile: urp_bridgeimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-09-29 08:42:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _URP_BRIDGEIMPL_HXX_
#define _URP_BRIDGEIMPL_HXX_

#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

#include <bridges/remote/bridgeimpl.hxx>

#include "urp_cache.hxx"
#include "urp_marshal_decl.hxx"
#include "urp_replycontainer.hxx"
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

    ::osl::Mutex m_marshalingMutex;
    ::osl::Mutex m_disposingMutex;
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
    FILE *m_pLogFile;
    ::osl::Condition m_cndWaitForThreads;

    struct Properties m_properties;
    class PropertyObject *m_pPropertyObject;
};

}
#endif
