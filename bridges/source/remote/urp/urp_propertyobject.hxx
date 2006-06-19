/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_propertyobject.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:53:35 $
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
#include <stdio.h>

#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_CONDITN_H_
#include <osl/conditn.h>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _UNO_SEQUENCE2_H_
#include <uno/sequence2.h>
#endif

#ifndef _BRIDGES_REMOTE_REMOTE_H_
#include <bridges/remote/remote.h>
#endif

#ifndef _URP_PROPERTY_HXX_
#include "urp_property.hxx"
#endif

namespace bridges_urp {

struct urp_BridgeImpl;
const sal_Char g_NameOfUrpProtocolPropertiesObject[] = "UrpProtocolProperties";

// helper functions
void assignFromStringToStruct( const ::rtl::OUString & sProps , struct Properties *pProps );

class PropertyObject : public remote_Interface
{
private:
    ::osl::Mutex m_mutex;
    oslCondition m_commitChangeCondition;
    oslInterlockedCount m_nRefCount;
    urp_BridgeImpl *m_pBridgeImpl;
    struct Properties *m_pLocalSetting;
    struct Properties m_propsToBeApplied;

    uno_Environment *m_pEnvRemote;
    sal_Int32 m_nRandomNumberOfRequest;
    sal_Bool  m_bRequestChangeHasBeenCalled;
    sal_Bool  m_bClientWaitingForCommit;
    sal_Bool  m_bServerWaitingForCommit;
    sal_Bool  m_bApplyProperties;

public:
    PropertyObject(
        struct Properties *pLocalSetting , uno_Environment *pEnvRemote, urp_BridgeImpl *pImpl );
    ~PropertyObject();

    void SAL_CALL thisAcquire( )
        {
            osl_incrementInterlockedCount( &m_nRefCount );
        }

    void SAL_CALL thisRelease()
        {
            if( ! osl_decrementInterlockedCount( &m_nRefCount  ) )
            {
                delete this;
            }
        }

    void SAL_CALL thisDispatch( typelib_TypeDescription const * pMemberType,
                                void * pReturn,
                                void * pArgs[],
                                uno_Any ** ppException );

public: // local
    sal_Int32 SAL_CALL localRequestChange(  );
    void SAL_CALL localCommitChange( const ::rtl::OUString &properties, sal_Bool *pbExceptionThrown );
    void SAL_CALL localGetPropertiesFromRemote( struct Properties * );

    // returns 0, if nothing was commited.
    inline sal_Bool SAL_CALL changesHaveBeenCommited()
        { return m_bApplyProperties; }
    Properties SAL_CALL getCommitedChanges();

    inline sal_Bool SAL_CALL waitingForCommitChangeReply()
        { return m_bClientWaitingForCommit; }
    void SAL_CALL waitUntilChangesAreCommitted();

protected:
    // these methods are called by thisDispatch
    void SAL_CALL     implGetProperties( uno_Sequence **ppReturnValue );
    sal_Int32 SAL_CALL implRequestChange( sal_Int32 nRandomNumber, uno_Any **ppException );
    void SAL_CALL     implCommitChange( uno_Sequence *seqOfProperties, uno_Any **ppException );
};

}
