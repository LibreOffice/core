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



#ifndef _BASICSTATUS_HXX
#define _BASICSTATUS_HXX

#include <svtools/taskbar.hxx>
#include <vcl/status.hxx>

#define ST_MESSAGE          1
#define ST_LINE             2
#define ST_PROF             3

class BasicFrame;

class StatusLine : public TaskBar
{
protected:
    BasicFrame* pFrame;
    DECL_LINK( ActivateTask, TaskToolBox* );

public:
    StatusLine( BasicFrame* );
    void Message( const String& );           // Show text
    void Pos( const String& s );             // Show text position
    void SetProfileName( const String& s );  // Current Profile
    void LoadTaskToolBox();
};

#endif
