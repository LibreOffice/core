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


#ifndef _editwin
#define _editwin

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif
#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>

class GHEditWindow : public FloatingWindow
{

protected:

    MultiLineEdit   aInhalt;

    virtual sal_Bool Close(); // derived
    void Resize();

public:

    GHEditWindow();
    GHEditWindow(Window * pParent, String aName = CUniString("Neues Fenster"), WinBits iWstyle = WB_STDWORK);

    void Clear();
    void AddText( String aNew, sal_Bool bMoveToEnd = sal_True);
};



class EditFileWindow : public GHEditWindow
{

    String  aFileName;
    virtual sal_Bool Close(); // derived
    void LoadFile();

public:
    EditFileWindow(Window * pParent, String aName = CUniString("Neue Datei"), WinBits iWstyle = WB_STDWORK);

};

#endif

