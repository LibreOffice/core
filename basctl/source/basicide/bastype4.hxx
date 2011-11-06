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


#ifndef _BASTYPE4_HXX
#define _BASTYPE4_HXX


#include <svtools/tabbar.hxx>

class EditEngine;
class EditView;

class ExtendedTabBar : public TabBar
{
    EditEngine*     pEditEngine;
    EditView*       pEditView;
    sal_Bool            bIsInKeyInput;
#if _SOLAR__PRIVATE
    void            ImpCheckEditEngine( sal_Bool bKeepNewText );
#endif
protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    LoseFocus();
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Paint( const Rectangle& );

    virtual sal_Bool    StartRenamingTab( sal_uInt16 nCurId );
    virtual sal_Bool    AllowRenamingTab( sal_uInt16 nCurId, const String& rNewName );
    virtual void    TabRenamed( sal_uInt16 nCurId, const String& rNewName );

public:
                    ExtendedTabBar( Window* pParent, WinBits nStyle );
                    ~ExtendedTabBar();

    void            RenameSelectedTab();
    sal_Bool            IsInEditMode() const { return pEditEngine ? sal_True : sal_False; }
    void            StopEditMode( sal_Bool bKeepCurText = sal_False );
};

#endif  //_BASTYPE4_HXX


