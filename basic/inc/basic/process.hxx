/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _PROCESS_HXX
#define _PROCESS_HXX

#include <tools/string.hxx>
#include <vos/process.hxx>

#include <map>

typedef std::map< String, String > Environment;
typedef Environment::value_type EnvironmentVariable;

class Process
{
    // Internal members and methods
    vos::OArgumentList *pArgumentList;
    vos::OEnvironment *pEnvList;
    vos::OProcess *pProcess;
    sal_Bool ImplIsRunning();
    long ImplGetExitCode();
    sal_Bool bWasGPF;
    sal_Bool bHasBeenStarted;

public:
    Process();
    ~Process();
    // Methoden
    void SetImage( const String &aAppPath, const String &aAppParams, const Environment *pEnv = NULL );
    sal_Bool Start();
    sal_uIntPtr GetExitCode();
    sal_Bool IsRunning();
    sal_Bool WasGPF();

    sal_Bool Terminate();
};

#endif
