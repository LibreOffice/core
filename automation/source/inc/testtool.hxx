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


#ifndef SVTOOLS_TESTTOOL_HXX
#define SVTOOLS_TESTTOOL_HXX

#include <tools/solar.h>
#include <tools/link.hxx>
#include <vcl/timer.hxx>

class Application;
class SvStream;

class StatementFlow;
class CommunicationManager;
class CommunicationLink;
#if OSL_DEBUG_LEVEL > 1
class EditWindow;
#endif
class ImplRC;

class ImplRemoteControl
{
    friend class StatementFlow;

    sal_Bool         m_bIdleInserted;
    AutoTimer    m_aIdleTimer;
    sal_Bool         m_bInsideExecutionLoop;
#if OSL_DEBUG_LEVEL > 1
    EditWindow *m_pDbgWin;
#endif

public:
    ImplRemoteControl();
    ~ImplRemoteControl();
    sal_Bool QueCommands( sal_uLong nServiceId, SvStream *pIn );
    SvStream* GetReturnStream();

    DECL_LINK( IdleHdl,   Application* );
    DECL_LINK( CommandHdl, Application* );

    DECL_LINK( QueCommandsEvent, CommunicationLink* );

protected:
    CommunicationManager *pServiceMgr;
    SvStream *pRetStream;
};

#endif // SVTOOLS_TESTTOOL_HXX
