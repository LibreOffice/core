/*************************************************************************
 *
 *  $RCSfile: privsplt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
