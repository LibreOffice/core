/*************************************************************************
 *
 *  $RCSfile: sysplug.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:18:10 $
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
#include <cstdarg>

#include <sys/types.h>
#include <signal.h>
#include <osl/thread.h>

#include <plugin/impl.hxx>

int UnxPluginComm::nConnCounter = 0;

UnxPluginComm::UnxPluginComm(
                             const String& mimetype,
                             const String& library,
                             XLIB_Window aParent,
                             int nDescriptor1,
                             int nDescriptor2
                             ) :
        PluginComm( ::rtl::OUStringToOString( library, osl_getThreadTextEncoding() ) ),
        PluginConnector( nDescriptor2 )
{
    char pDesc[32];
    char pWindow[32];
    sprintf( pWindow, "%d", aParent );
    sprintf( pDesc, "%d", nDescriptor1 );
    ByteString aLib( library, osl_getThreadTextEncoding() );

    char* pArgs[5];
    pArgs[0] = "pluginapp.bin";
    pArgs[1] = pDesc;
    pArgs[2] = const_cast<char*>(aLib.GetBuffer());
    pArgs[3] = pWindow;
    pArgs[4] = NULL;

#if OSL_DEBUG_LEVEL > 1
    m_nCommPID = 10;
    fprintf( stderr, "Try to launch: %s %s %s %s, descriptors are %d, %d\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3], nDescriptor1, nDescriptor2 );
#endif

    if( ! ( m_nCommPID = fork() ) )
      {
          execvp( pArgs[0], pArgs );
          fprintf( stderr, "Error: could not exec %s\n", pArgs[0] );
          exit(255);
      }

    if( m_nCommPID != -1 )
    {
        // wait for pluginapp.bin to start up
        if( ! WaitForMessage( 5000 ) )
        {
            fprintf( stderr, "Timeout on command: %s %s %s %s\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3] );
            invalidate();
        }
        else
        {
            MediatorMessage* pMessage = GetNextMessage( TRUE );
            Respond( pMessage->m_nID,
                     "init ack",8,
                     NULL );
            delete pMessage;
            NPP_Initialize();
        }
    }
}

UnxPluginComm::~UnxPluginComm()
{
    NPP_Shutdown();
    if( m_nCommPID != -1 && m_nCommPID != 0 )
        kill( m_nCommPID, 9 );
}
