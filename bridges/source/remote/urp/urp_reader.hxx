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
#include <osl/thread.hxx>

#include "urp_unmarshal.hxx"

struct remote_Connection;
typedef struct _uno_Environment uno_Environment;

namespace bridges_urp
{

class OWriterThread;
struct MessageFlags;

class OReaderThread :
    public ::osl::Thread
{
public:
    OReaderThread( remote_Connection *pConnection ,
                   uno_Environment *pEnvRemote,
                   OWriterThread *pWriterThread );
    ~OReaderThread();

    // may only be called in the callstack of this thread !!!!!
    // run() -> disposeEnvironment() -> dispose() -> destroyYourself()
    void destroyYourself();

private:
    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();

    inline sal_Bool readBlock( sal_Int32 *pnMessageCount );
    inline sal_Bool readFlags( struct MessageFlags *pFlags );

    void disposeEnvironment();

    inline sal_Bool getMemberTypeDescription(
        typelib_InterfaceAttributeTypeDescription **ppAttributeType,
        typelib_InterfaceMethodTypeDescription **ppMethodType,
        sal_Bool *pbIsSetter,
        sal_uInt16 nMethodId ,
        typelib_TypeDescriptionReference *pITypeRef);

    remote_Connection *m_pConnection;
    uno_Environment *m_pEnvRemote;
    OWriterThread *m_pWriterThread;
    sal_Bool m_bDestroyMyself;
    sal_Bool m_bContinue;
    urp_BridgeImpl *m_pBridgeImpl;
    Unmarshal m_unmarshal;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
