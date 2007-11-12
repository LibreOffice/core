/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sysplug.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-12 15:32:07 $
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
#include <cstdarg>

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <osl/thread.h>

#include <plugin/impl.hxx>

int MacPluginComm::nConnCounter = 0;

MacPluginComm::MacPluginComm(
                             const String& mimetype,
                             const String& library,
                             WindowRef aParent,
                             int nDescriptor1,
                             int nDescriptor2
                             ) :
PluginComm( ::rtl::OUStringToOString( library, osl_getThreadTextEncoding() ) )/*,
PluginConnector( nDescriptor2 )*/
{
    //char pDesc[32];
//  char pWindow[32];
//  sprintf( pWindow, "%d", aParent );
//    sprintf( pDesc, "%d", nDescriptor1 );
//  ByteString aLib( library, osl_getThreadTextEncoding() );
//
//  char* pArgs[5];
//  pArgs[0] = "pluginapp.bin";
//  pArgs[1] = pDesc;
//  pArgs[2] = const_cast<char*>(aLib.GetBuffer());
//  pArgs[3] = pWindow;
//  pArgs[4] = NULL;
//
//#if OSL_DEBUG_LEVEL > 1
//  m_nCommPID = 10;
//  fprintf( stderr, "Try to launch: %s %s %s %s, descriptors are %d, %d\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3], nDescriptor1, nDescriptor2 );
//#endif
//
//  if( ! ( m_nCommPID = fork() ) )
//      {
//      execvp( pArgs[0], pArgs );
//          fprintf( stderr, "Error: could not exec %s\n", pArgs[0] );
//          _exit(255);
//      }
//
//  if( m_nCommPID != -1 )
//  {
//      // wait for pluginapp.bin to start up
//      if( ! WaitForMessage( 5000 ) )
//      {
//          fprintf( stderr, "Timeout on command: %s %s %s %s\n", pArgs[0], pArgs[1], pArgs[2], pArgs[3] );
//          invalidate();
//      }
//      else
//      {
//          MediatorMessage* pMessage = GetNextMessage( TRUE );
//          Respond( pMessage->m_nID,
//                   "init ack",8,
//                   NULL );
//          delete pMessage;
//          NPP_Initialize();
//      }
//  }
}

MacPluginComm::~MacPluginComm()
{
    NPP_Shutdown();
    if( m_nCommPID != -1 && m_nCommPID != 0 )
    {
        int status = 16777216;
        pid_t nExit = waitpid( m_nCommPID, &status, WUNTRACED );
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "child %d (plugin app child %d) exited with status %d\n", nExit, m_nCommPID, WEXITSTATUS(status) );
#endif
    }
}

NPError MacPluginComm::NPP_Destroy( NPP instance,
                                    NPSavedData** save )
{
    return 0;
}

NPError MacPluginComm::NPP_DestroyStream( NPP instance,
                                          NPStream* stream,
                                          NPError reason )
{
    return 0;
}

void* MacPluginComm::NPP_GetJavaClass()
{
    return 0;
}

NPError MacPluginComm::NPP_Initialize()
{
    return 0;
}

NPError MacPluginComm::NPP_New( NPMIMEType pluginType,
                                NPP instance,
                                uint16 mode,
                                int16 argc,
                                char* argn[],
                                char* argv[],
                                NPSavedData *saved )
{
    return 0;
}

NPError MacPluginComm::NPP_NewStream( NPP instance,
                                      NPMIMEType type,
                                      NPStream* stream,
                                      NPBool seekable,
                                      uint16* stype )
{
    return 0;
}

void MacPluginComm::NPP_Print( NPP instance,
                               NPPrint* platformPrint )
{

}

NPError MacPluginComm::NPP_SetWindow( NPP instance,
                                      NPWindow* window )
{
    return 0;
}

void MacPluginComm::NPP_Shutdown()
{

}

void MacPluginComm::NPP_StreamAsFile( NPP instance,
                                      NPStream* stream,
                                      const char* fname )
{

}

void MacPluginComm::NPP_URLNotify( NPP instance,
                                   const char* url,
                                   NPReason reason,
                                   void* notifyData )
{

}

int32 MacPluginComm::NPP_Write( NPP instance,
                                NPStream* stream,
                                int32 offset,
                                int32 len,
                                void* buffer )
{
    return 0;
}

int32 MacPluginComm::NPP_WriteReady( NPP instance,
                                     NPStream* stream )
{
    return 0;
}

char* MacPluginComm::NPP_GetMIMEDescription()
{
    return "";
}

NPError MacPluginComm::NPP_GetValue( NPP instance, NPPVariable variable, void* value )
{
    return 0;
}

NPError MacPluginComm::NPP_SetValue( NPP instance,
                                     NPNVariable variable,
                                     void *value)
{
    return 0;
}

