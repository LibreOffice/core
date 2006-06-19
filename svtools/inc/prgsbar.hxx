/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prgsbar.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:25:08 $
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

#ifndef _PRGSBAR_HXX
#define _PRGSBAR_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _WINDOW_HXX
#include <vcl/window.hxx>
#endif

/*************************************************************************

Beschreibung
============

class ProgressBar

Diese Klasse dient zur Anzeige einer Progress-Anzeige.

--------------------------------------------------------------------------

WinBits

WB_BORDER           Border um das Fenster
WB_3DLOOK           3D-Darstellung

--------------------------------------------------------------------------

Methoden

Mit SetValue() setzt man einen Prozent-Wert zwischen 0 und 100. Wenn Werte
groesser 100 gesetzt werden, faengt das letzte Rechteck an zu blinken.

*************************************************************************/

// -----------
// - WinBits -
// -----------

#define WB_STDPROGRESSBAR       WB_BORDER

// ---------------
// - ProgressBar -
// ---------------

class SVT_DLLPUBLIC ProgressBar : public Window
{
private:
    Point               maPos;
    long                mnPrgsWidth;
    long                mnPrgsHeight;
    USHORT              mnPercent;
    USHORT              mnPercentCount;
    BOOL                mbCalcNew;

#ifdef _SV_PRGSBAR_CXX
    using Window::ImplInit;
    SVT_DLLPRIVATE void             ImplInit();
    SVT_DLLPRIVATE void             ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SVT_DLLPRIVATE void             ImplDrawProgress( USHORT nOldPerc, USHORT nNewPerc );
#endif

public:
                        ProgressBar( Window* pParent, WinBits nWinBits = WB_STDPROGRESSBAR );
                        ProgressBar( Window* pParent, const ResId& rResId );
                        ~ProgressBar();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                SetValue( USHORT nNewPercent );
    USHORT              GetValue() const { return mnPercent; }
};

#endif  // _PRGSBAR_HXX
