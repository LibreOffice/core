/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: privsplt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:07:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_PRIVSPLT_HXX
#define _SV_PRIVSPLT_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

enum PRIVSPLIT_DIRECTION {PRIVSPLIT_HORZ,PRIVSPLIT_VERT };

class SvPrivatSplit : public FixedLine
{
    private:

        Link                aCtrModifiedLink;
        BOOL                aMovingFlag;
        Pointer             aWinPointer;
        PRIVSPLIT_DIRECTION eAdrSplit;
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
        void                ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        Tracking( const TrackingEvent& rTEvt );

    public:

        SvPrivatSplit( Window* pWindow, const ResId& rResId,PRIVSPLIT_DIRECTION eAdrSplit);

        SvPrivatSplit( Window* pParent,PRIVSPLIT_DIRECTION eAdrSplit, WinBits nStyle = 0);

        virtual short   GetDeltaX();
        virtual short   GetDeltaY();

        virtual void    CtrModified();

        void            SetXRange(Range cRgeX);
        void            SetYRange(Range cRgeY);

        void            MoveSplitTo(Point aPos);

        virtual void    StateChanged( StateChangedType nType );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        void            SetCtrModifiedHdl( const Link& rLink ) { aCtrModifiedLink = rLink; }
        const Link&     GetCtrModifiedHdl() const { return aCtrModifiedLink; }
};


#endif
