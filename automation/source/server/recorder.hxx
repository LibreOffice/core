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



#include <tools/string.hxx>
#include <tools/link.hxx>
#include <vcl/timer.hxx>

class ToolBox;
class Window;
class VclSimpleEvent;

class MacroRecorder
{
private:
    Window* GetParentWithID( Window* pThis );
    rtl::OString GetParentID( Window* pThis );

    Link aEventListenerHdl;
    DECL_LINK( EventListener, VclSimpleEvent* );

    Window* pLastWin;
    Window* pEditModify;
    String aEditModifyString;

    ToolBox *pActionParent;      // toolbox from which a tearoff or OpenMenu might happen

    // record keys
    String aKeyString;
    rtl::OString aKeyUniqueID;     // has to be remembered separately since Window might be gone when needed
    Window* pKeyWin;
    sal_Bool bKeyFollowFocus;

    AutoTimer aHookRefresh;
    void AddEventHooks();
    void RemoveEventHooks();
    DECL_LINK( HookRefreshHdl, void* );

    void LogVCL( rtl::OString aParentID, sal_uInt16 nVCLWindowType, rtl::OString aID, String aMethod, sal_uInt16 aParam );
    void LogVCL( rtl::OString aParentID, sal_uInt16 nVCLWindowType, rtl::OString aID, String aMethod );

    static MacroRecorder *pMacroRecorder;

    MacroRecorder();
    ~MacroRecorder();
    void CheckDelete();

    // Actions to perform
    sal_Bool m_bRecord;
    sal_Bool m_bLog;

public:

    void SetActionRecord( sal_Bool bRecord = sal_True ) { m_bRecord = bRecord; CheckDelete(); };
    void SetActionLog( sal_Bool bLog = sal_True ) { m_bLog = bLog; CheckDelete(); };

    static MacroRecorder* GetMacroRecorder();
    static sal_Bool HasMacroRecorder();
};

