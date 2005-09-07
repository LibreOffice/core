/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_reader.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:48:53 $
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

    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();

    // may only be called in the callstack of this thread !!!!!
    // run() -> disposeEnvironment() -> dispose() -> destroyYourself()
    void destroyYourself();

    inline sal_Bool readBlock( sal_Int32 *pnMessageCount );
    inline sal_Bool readFlags( struct MessageFlags *pFlags );
private:
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
