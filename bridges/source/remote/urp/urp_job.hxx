/*************************************************************************
 *
 *  $RCSfile: urp_job.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:50 $
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
#include <list>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>
#include <uno/environment.h>

#include "urp_threadid.hxx"
#include "urp_unmarshal.hxx"
#include <bridges/remote/bridgeimpl.hxx>

const sal_Int32 MAX_ENTRIES_IN_MULTIJOB = 60;
const sal_Int32 g_nInitialMemorySize = 6192;


namespace bridges_urp
{
class Unmarshal;
struct urp_BridgeImpl;

template < class t >
inline t mymax( const t &t1 , const t &t2 )
{
    return t1 > t2 ? t1 : t2;
}

class Job
{
public:
      Job( uno_Environment *pEnvRemote,
           sal_Sequence *pTid,
           struct urp_BridgeImpl *pBridgeImpl,
           Unmarshal *pUnmarshal );

    Job( uno_Environment *pEnvRemote,
           struct urp_BridgeImpl *pBridgeImpl,
         ::bridges_remote::RemoteThreadCounter_HoldEnvWeak value )
        : m_counter( pEnvRemote , value )
        , m_pBridgeImpl( pBridgeImpl )
        , m_pTid( 0 )
        {}

    virtual ~Job();

    // doit method is used only for ServerJobs, calls execute and pack
    static void  SAL_CALL doit( void *pThreadSpecificData );

    // is called from the dispatcher thread
    virtual sal_Bool extract( ) = 0;
    virtual void initiate() = 0;
    virtual void execute() = 0;

    inline void setThreadId( sal_Sequence *pId )
        { rtl_byte_sequence_assign( &m_pTid , pId ); }
       inline void setUnmarshal( Unmarshal *p )
           { m_pUnmarshal = p; }

protected:
      Unmarshal *m_pUnmarshal;
    struct urp_BridgeImpl *m_pBridgeImpl;
    sal_Sequence          *m_pTid;
    ::bridges_remote::RemoteThreadCounter m_counter;
};

class ClientJob : public Job
{
public:
    ClientJob( uno_Environment *pEnvRemote, struct urp_BridgeImpl *pBridgeImpl )
        : Job( pEnvRemote , pBridgeImpl, ::bridges_remote::RTC_HOLDENVWEAK )
        , m_ppException( 0 )
        , m_ppArgs( 0 )
        , m_pReturn( 0 )
        , m_pMethodType( 0 )
        , m_pAttributeType( 0 )
        {}

    // ~ClientJob
    // no release for method type and attribute type necessary, because
    // it was acquired by the caller of urp_sendRequest. The lifetime
    // of the ClientJob object is always shorter than the urp_sendRequest call.
    ~ClientJob()
        {}

    virtual sal_Bool extract( );
    virtual void initiate();
    virtual void execute()
        {}

public:
    void     **m_ppArgs;
    void     *m_pReturn;
    typelib_InterfaceMethodTypeDescription    *m_pMethodType;
    typelib_InterfaceAttributeTypeDescription *m_pAttributeType;

    uno_Any  **m_ppException;
    sal_Bool m_bExceptionOccured;
};


struct MemberTypeInfo
{
    typelib_InterfaceTypeDescription          *m_pInterfaceType;
    typelib_InterfaceMethodTypeDescription    *m_pMethodType;
    typelib_InterfaceAttributeTypeDescription *m_pAttributeType;
    sal_Int32 m_nArgCount;
    sal_Bool  m_bIsReleaseCall;
    sal_Bool  *m_pbIsIn;
    sal_Bool  *m_pbIsOut;
    typelib_TypeDescription *m_pReturnType;
    typelib_TypeDescription **m_ppArgType;
};


struct ServerJobEntry
{
    rtl_uString           *m_pOid;
    remote_Interface      *m_pRemoteI;
    typelib_TypeDescriptionReference          *m_pInterfaceTypeRef;
    void                  **m_ppArgs;
    void                  *m_pReturn;
    uno_Any               m_exception;
     uno_Any               *m_pException;

};

class ServerMultiJob : public Job
{
public:
    ServerMultiJob( uno_Environment *pEnvRemote,
                    sal_Sequence *pTid,
                    struct urp_BridgeImpl *pBridgeImpl,
                    Unmarshal *pUnmarshal);
    ~ServerMultiJob();
public:
    virtual sal_Bool extract( );
    virtual void initiate();
    virtual void execute();

public:
    // setMethodType or setAttributeType MUST be called before extract
    inline void setMethodType( typelib_InterfaceMethodTypeDescription *pMethodType, sal_Bool bIsReleaseCall)
        {
            m_aTypeInfo[m_nCalls].m_pMethodType = pMethodType;
            m_aTypeInfo[m_nCalls].m_pAttributeType = 0;
            m_aTypeInfo[m_nCalls].m_nArgCount = pMethodType->nParams;
            m_aTypeInfo[m_nCalls].m_bIsReleaseCall = bIsReleaseCall;
        }

    inline void setAttributeType( typelib_InterfaceAttributeTypeDescription *pAttributeType, sal_Bool bIsSetter )
        {
            m_aTypeInfo[m_nCalls].m_pAttributeType = pAttributeType;
            m_aTypeInfo[m_nCalls].m_pMethodType = 0;
            m_aTypeInfo[m_nCalls].m_nArgCount = bIsSetter ? 1 : 0;
            m_aTypeInfo[m_nCalls].m_bIsReleaseCall = sal_False;
        }

    inline void setType( const ::com::sun::star::uno::Type &type )
        {
            m_aEntries[m_nCalls].m_pInterfaceTypeRef = type.getTypeLibType();
            typelib_typedescriptionreference_acquire( m_aEntries[m_nCalls].m_pInterfaceTypeRef );
            TYPELIB_DANGER_GET(
                (typelib_TypeDescription ** )&(m_aTypeInfo[m_nCalls].m_pInterfaceType) ,
                type.getTypeLibType() );
        }
    // setOid or setInterface MUST be called before extract
      inline void setOid(   const ::rtl::OUString & sOid )
        {
            m_aEntries[m_nCalls].m_pOid = sOid.pData;
            rtl_uString_acquire( m_aEntries[m_nCalls].m_pOid );
            m_aEntries[m_nCalls].m_pRemoteI = 0;
        }

    // setOid or setInterface MUST be called
    inline void setInterface( remote_Interface *pRemoteI )
        {
            m_aEntries[m_nCalls].m_pRemoteI = pRemoteI;
            pRemoteI->acquire( pRemoteI );
            m_aEntries[m_nCalls].m_pOid = 0;
        }

    inline sal_Bool isFull()
        { return m_nCalls >= MAX_ENTRIES_IN_MULTIJOB; }

    inline sal_Int8 *getHeap( sal_Int32 nSizeToAlloc )
        {
            if( nSizeToAlloc + m_nCurrentMemPosition > g_nInitialMemorySize )
            {
                m_lstMem.push_back( m_pCurrentMem );
                m_pCurrentMem = (sal_Int8*)
                    rtl_allocateMemory( mymax( nSizeToAlloc , g_nInitialMemorySize ) );
                m_nCurrentMemPosition = 0;
            }
            sal_Int8 *pHeap = m_pCurrentMem + m_nCurrentMemPosition;
            m_nCurrentMemPosition += nSizeToAlloc;

            // care for alignment
            if( m_nCurrentMemPosition & 0x7 )
            {
                m_nCurrentMemPosition = ( ((sal_uInt32)m_nCurrentMemPosition) & ( 0xffffffff - 0x7 )) + 8;
            }
            return pHeap;
        }
      void prepareRuntimeException( const ::rtl::OUString &sMessage, sal_Int32 nCall );

private:
    uno_Environment *m_pEnvRemote;
    sal_Int32 m_nCalls;
    ServerJobEntry m_aEntries[MAX_ENTRIES_IN_MULTIJOB];
    MemberTypeInfo m_aTypeInfo[MAX_ENTRIES_IN_MULTIJOB];

    sal_Int8 *m_pCurrentMem;
    sal_Int32 m_nCurrentMemPosition;

    // list of memory pointers, that must be freed
    ::std::list< sal_Int8 * > m_lstMem;
};

}
