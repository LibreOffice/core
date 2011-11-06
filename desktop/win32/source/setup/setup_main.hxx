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




#ifndef SETUP_MAIN_HXX
#define SETUP_MAIN_HXX

class SetupApp
{
    DWORD           m_nOSVersion;
    DWORD           m_nMinorVersion;
    boolean         m_bIsWin9x      : 1;
    boolean         m_bNeedReboot   : 1;
    boolean         m_bAdministrative : 1;

public:
    UINT            m_uiRet;

                    SetupApp();
    virtual        ~SetupApp();

    virtual boolean Initialize( HINSTANCE hInst ) = 0;
    virtual boolean AlreadyRunning() const = 0;
    virtual boolean ReadProfile() = 0;
    virtual boolean GetPatches() = 0;
    virtual boolean ChooseLanguage( long& rLanguage ) = 0;
    virtual boolean CheckVersion() = 0;
    virtual boolean CheckForUpgrade() = 0;
    virtual boolean InstallRuntimes() = 0;
    virtual boolean Install( long nLanguage ) = 0;

    virtual UINT    GetError() const = 0;
    virtual void    DisplayError( UINT nErr ) const = 0;

    void            SetError( UINT nErr ) { m_uiRet = nErr; }
    boolean         IsWin9x() const { return m_bIsWin9x; }
    DWORD           GetOSVersion() const { return m_nOSVersion; }
    DWORD           GetMinorVersion() const { return m_nMinorVersion; }

    boolean         IsAdminInstall() { return m_bAdministrative; }
    void            SetAdminInstall( boolean bValue ) { m_bAdministrative = bValue; }

    void            SetRebootNeeded( boolean bNeedReboot ) { m_bNeedReboot = bNeedReboot; }
    boolean         NeedReboot() const { return m_bNeedReboot; }
};

SetupApp* Create_SetupAppA();
SetupApp* Create_SetupAppW();

#endif
