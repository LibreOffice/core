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



#include <appwin.hxx>
#include <msgedit.hxx>

class AppError : public AppWin
{
protected:
    DECL_LINK( EditChange, MsgEdit * );
public:
    AppError( BasicFrame*, String );
    ~AppError();
//  long Command( short nID );
    virtual long InitMenu( Menu* );
    virtual long DeInitMenu( Menu* );
    sal_uInt16 GetLineNr();
    FileType GetFileType();
    MsgEdit* GetMsgTree()           { return ((MsgEdit*)pDataEdit); }
    virtual sal_Bool ReloadAllowed(){ return !StarBASIC::IsRunning(); }
    virtual void LoadIniFile();     // (re)load ini file after change
    DirEntry aBaseDir;
};

