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

    void SAL_CALL waitUntilChangesAreCommitted();

protected:
    // these methods are called by thisDispatch
    void SAL_CALL     implGetProperties( uno_Sequence **ppReturnValue );
    sal_Int32 SAL_CALL implRequestChange( sal_Int32 nRandomNumber, uno_Any **ppException );
    void SAL_CALL     implCommitChange( uno_Sequence *seqOfProperties, uno_Any **ppException );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
