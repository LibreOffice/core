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



#ifndef _SV_EXPANDER_HXX
#define _SV_EXPANDER_HXX

#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>

enum SvExpanderStateType
{
    EST_MIN=1,
    EST_PLUS=2,
    EST_MIN_DOWN=3,
    EST_PLUS_DOWN=4,
    EST_NONE=5,
    EST_MIN_DIS=6,
    EST_PLUS_DIS=7,
    EST_MIN_DOWN_DIS=8,
    EST_PLUS_DOWN_DIS=9
};

class SvExpander: public Control
{
private:
        Point                   aImagePos;
        Point                   aTextPos;
        Image                   aActiveImage;
        Rectangle               maFocusRect;
        ImageList               maExpanderImages;
        sal_Bool                    mbIsExpanded;
        sal_Bool                    mbHasFocusRect;
        sal_Bool                    mbIsInMouseDown;
        Link                    maToggleHdl;
        SvExpanderStateType eType;

protected:

        virtual long    PreNotify( NotifyEvent& rNEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Paint( const Rectangle& rRect );
        virtual void    KeyInput( const KeyEvent& rKEvt );
        virtual void    KeyUp( const KeyEvent& rKEvt );

        virtual void    Click();
        virtual void    Resize();

public:
        SvExpander( Window* pParent, WinBits nStyle = 0 );
        SvExpander( Window* pParent, const ResId& rResId );

        sal_Bool            IsExpanded() {return mbIsExpanded;}

        void            SetToExpanded(sal_Bool bFlag=sal_True);

        void            SetExpanderImage( SvExpanderStateType eType);
        Image           GetExpanderImage(SvExpanderStateType eType);
        Size            GetMinSize() const;

        void            SetToggleHdl( const Link& rLink ) { maToggleHdl = rLink; }
        const Link&     GetToggleHdl() const { return maToggleHdl; }
};



#endif
