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



#ifndef _PRGSBAR_HXX
#define _PRGSBAR_HXX

#include "svtools/svtdllapi.h"
#include <vcl/window.hxx>

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
    sal_uInt16              mnPercent;
    sal_uInt16              mnPercentCount;
    sal_Bool                mbCalcNew;

#ifdef _SV_PRGSBAR_CXX
    using Window::ImplInit;
    SVT_DLLPRIVATE void             ImplInit();
    SVT_DLLPRIVATE void             ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SVT_DLLPRIVATE void             ImplDrawProgress( sal_uInt16 nOldPerc, sal_uInt16 nNewPerc );
#endif

public:
                        ProgressBar( Window* pParent, WinBits nWinBits = WB_STDPROGRESSBAR );
                        ProgressBar( Window* pParent, const ResId& rResId );
                        ~ProgressBar();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    void                SetValue( sal_uInt16 nNewPercent );
    sal_uInt16              GetValue() const { return mnPercent; }
};

#endif  // _PRGSBAR_HXX
