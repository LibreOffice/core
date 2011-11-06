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



#ifndef _DDECTRL_HXX
#define _DDECTRL_HXX

#include <tools/link.hxx>
#ifndef _SBERRORS_HXX
#include <basic/sberrors.hxx>
#endif
#include <tools/string.hxx>

class DdeConnection;
class DdeConnections;
class DdeData;

class SbiDdeControl
{
private:
    DECL_LINK( Data, DdeData* );
    SbError GetLastErr( DdeConnection* );
    sal_Int16 GetFreeChannel();
    DdeConnections* pConvList;
    String aData;

public:

    SbiDdeControl();
    ~SbiDdeControl();

    SbError Initiate( const String& rService, const String& rTopic,
                     sal_Int16& rnHandle );
    SbError Terminate( sal_Int16 nChannel );
    SbError TerminateAll();
    SbError Request( sal_Int16 nChannel, const String& rItem, String& rResult );
    SbError Execute( sal_Int16 nChannel, const String& rCommand );
    SbError Poke( sal_Int16 nChannel, const String& rItem, const String& rData );
};

#endif
