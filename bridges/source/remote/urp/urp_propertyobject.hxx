/*************************************************************************
 *
 *  $RCSfile: urp_propertyobject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-11-28 14:42:38 $
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
#include <stdio.h>

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <osl/conditn.h>

#include <rtl/string.hxx>
#include <uno/sequence2.h>

#include <bridges/remote/remote.h>

#include "urp_property.hxx"

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

    void SAL_CALL thisDispatch( typelib_TypeDescription * pMemberType,
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


    // static helper methods
public:
    static void SAL_CALL staticAcquire( remote_Interface *pRemoteI )
        {
            PropertyObject *pProperties = (PropertyObject *) pRemoteI;
            pProperties->thisAcquire();
        }

    static void SAL_CALL staticRelease( remote_Interface *pRemoteI )
        {
            PropertyObject *pProperties = (PropertyObject *) pRemoteI;
            pProperties->thisRelease();
        }

    static void SAL_CALL staticDispatch( remote_Interface * pRemoteI,
                                         typelib_TypeDescription * pMemberType,
                                         void * pReturn,
                                         void * pArgs[],
                                         uno_Any ** ppException )
        {
            PropertyObject *pProperties = (PropertyObject *) pRemoteI;
            pProperties->thisDispatch( pMemberType, pReturn, pArgs, ppException );
        }
};

}
