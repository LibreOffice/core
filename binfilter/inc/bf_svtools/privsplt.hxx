/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_PRIVSPLT_HXX
#define	_SV_PRIVSPLT_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

namespace binfilter
{

enum PRIVSPLIT_DIRECTION {PRIVSPLIT_HORZ,PRIVSPLIT_VERT };

class SvPrivatSplit : public FixedLine
{
    private:

        Link				aCtrModifiedLink;
        BOOL				aMovingFlag;
        Pointer				aWinPointer;		
        PRIVSPLIT_DIRECTION	eAdrSplit;
        short				nOldX;
        short				nOldY;
        short				nNewX;
        short				nNewY;
        short				nMinPos;
        short				nMaxPos;
        Range				aXMovingRange;
        Range				aYMovingRange;
        short				nDeltaX;
        short				nDeltaY;
        void				ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );

    protected:
        virtual void		MouseButtonDown( const MouseEvent& rMEvt );
        virtual void		Tracking( const TrackingEvent& rTEvt );

    public:

        SvPrivatSplit( Window* pWindow, const ResId& rResId,PRIVSPLIT_DIRECTION eAdrSplit);

        SvPrivatSplit( Window* pParent,PRIVSPLIT_DIRECTION eAdrSplit, WinBits nStyle = 0);

        virtual short	GetDeltaX();
        virtual short	GetDeltaY();
        
        virtual void    CtrModified();

        void			SetXRange(Range cRgeX);
        void			SetYRange(Range cRgeY);

        void			MoveSplitTo(Point aPos);

        virtual	void	StateChanged( StateChangedType nType );
        virtual	void	DataChanged( const DataChangedEvent& rDCEvt );

        void            SetCtrModifiedHdl( const Link& rLink ) { aCtrModifiedLink = rLink; }
        const Link&     GetCtrModifiedHdl() const { return aCtrModifiedLink; }
};

}

#endif
