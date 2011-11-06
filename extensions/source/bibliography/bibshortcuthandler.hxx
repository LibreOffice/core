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



#ifndef _BIBSHORTCUTHANDLER_HXX
#define _BIBSHORTCUTHANDLER_HXX

#include <vcl/window.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/tabpage.hxx>

// additional classes to handle shortcuts
// code in bibcont.cxx


class BibShortCutHandler
{
private:
    Window*                 pBaseClass;     // in cases, where BibShortCutHandler also has to be a window

protected:
    inline                  BibShortCutHandler( Window* pBaseClass );

public:
    virtual                 ~BibShortCutHandler();
    virtual sal_Bool            HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled

    inline Window*          GetWindow( void );
};

inline BibShortCutHandler::BibShortCutHandler( Window* _pBaseClass ) : pBaseClass( _pBaseClass )
{
}

inline Window* BibShortCutHandler::GetWindow( void )
{
    return pBaseClass;
}


class BibWindow : public Window, public BibShortCutHandler
{
public:
                            BibWindow( Window* pParent,WinBits nStyle = WB_3DLOOK);
    virtual                 ~BibWindow();
};


class BibSplitWindow : public SplitWindow, public BibShortCutHandler
{
public:
                            BibSplitWindow( Window* pParent,WinBits nStyle = WB_3DLOOK);
    virtual                 ~BibSplitWindow();
};


class BibTabPage : public TabPage, public BibShortCutHandler
{
public:
                            BibTabPage( Window* pParent, const ResId& rResId );
    virtual                 ~BibTabPage();
};

#endif
