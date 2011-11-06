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


#ifndef _BASTYPE3_HXX
#define _BASTYPE3_HXX

#include <svheader.hxx>

#include <svtools/svmedit.hxx>

#include <iderid.hxx>

class EditorWindow;

#ifndef NO_SPECIALEDIT

class ExtendedEdit : public Edit
{
private:
    Accelerator     aAcc;
    Link            aAccHdl;
    Link            aGotFocusHdl;
    Link            aLoseFocusHdl;

protected:
    DECL_LINK( EditAccHdl, Accelerator * );
    DECL_LINK( ImplGetFocusHdl, Control* );
    DECL_LINK( ImplLoseFocusHdl, Control* );

public:
                    ExtendedEdit( Window* pParent, IDEResId nRes );

    void            SetAccHdl( const Link& rLink )          { aAccHdl = rLink; }
    void            SetLoseFocusHdl( const Link& rLink )    { aLoseFocusHdl = rLink; }
    void            SetGotFocusHdl( const Link& rLink )     { aGotFocusHdl = rLink; }
    Accelerator&    GetAccelerator()                        { return aAcc; }
};

#endif  //NO_SPECIALEDIT

#endif  // _BASTYPE3_HXX
