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



#ifndef SC_AUTOSTYL_HXX
#define SC_AUTOSTYL_HXX

#include <vcl/timer.hxx>
#include <tools/list.hxx>
#include <tools/string.hxx>

class ScDocShell;
class ScRange;

class ScAutoStyleList
{
private:
    ScDocShell*     pDocSh;
    Timer           aTimer;
    Timer           aInitTimer;
    sal_uLong           nTimerStart;
    List            aEntries;
    List            aInitials;

    void    ExecuteEntries();
    void    AdjustEntries(sal_uLong nDiff);
    void    StartTimer(sal_uLong nNow);
    DECL_LINK( TimerHdl, Timer* );
    DECL_LINK( InitHdl, Timer* );

public:
            ScAutoStyleList(ScDocShell* pShell);
            ~ScAutoStyleList();

    void    AddInitial( const ScRange& rRange, const String& rStyle1,
                        sal_uLong nTimeout, const String& rStyle2 );
    void    AddEntry( sal_uLong nTimeout, const ScRange& rRange, const String& rStyle );

    void    ExecuteAllNow();
};



#endif

