/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: process.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:51:27 $
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

#ifndef _PROCESS_HXX
#define _PROCESS_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#include <map>

typedef std::map< String, String > Environment;
typedef Environment::value_type EnvironmentVariable;

class Process
{
    // Interne Member und Methoden
    NAMESPACE_VOS(OArgumentList) *pArgumentList;
    NAMESPACE_VOS(OEnvironment) *pEnvList;
    NAMESPACE_VOS(OProcess) *pProcess;
    BOOL ImplIsRunning();
    long ImplGetExitCode();
    BOOL bWasGPF;
    BOOL bHasBeenStarted;

public:
    Process();
    ~Process();
    // Methoden
    void SetImage( const String &aAppPath, const String &aAppParams, const Environment *pEnv = NULL );
    BOOL Start();
    ULONG GetExitCode();
    BOOL IsRunning();
    BOOL WasGPF();

    BOOL Terminate();
};

#endif
