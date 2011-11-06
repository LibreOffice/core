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



#ifndef SC_PRIVSPLT_HXX
#define SC_PRIVSPLT_HXX

#include <vcl/ctrl.hxx>

enum SC_SPLIT_DIRECTION {SC_SPLIT_HORZ,SC_SPLIT_VERT };

class ScPrivatSplit : public Control
{
    private:

        Link                aCtrModifiedLink;
        sal_Bool                aMovingFlag;
        Pointer             aWinPointer;
        SC_SPLIT_DIRECTION  eScSplit;
        short               nOldX;
        short               nOldY;
        short               nNewX;
        short               nNewY;
        short               nMinPos;
        short               nMaxPos;
        Range               aXMovingRange;
        Range               aYMovingRange;
        short               nDeltaX;
        short               nDeltaY;

        using Control::ImplInitSettings;
        void                ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );



    protected:
        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& rMEvt);

    public:

        ScPrivatSplit( Window* pWindow, const ResId& rResId,
                        SC_SPLIT_DIRECTION eScSplit);

        virtual short   GetDeltaX();
        virtual short   GetDeltaY();

        virtual void    CtrModified();

        void            SetYRange(Range cRgeY);

        void            MoveSplitTo(Point aPos);

        virtual void    StateChanged( StateChangedType nType );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        void            SetCtrModifiedHdl( const Link& rLink ) { aCtrModifiedLink = rLink; }
        const Link&     GetCtrModifiedHdl() const { return aCtrModifiedLink; }
};


#endif
