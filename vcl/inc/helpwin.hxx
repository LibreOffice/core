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



#ifndef _SV_HELPWIN_HXX
#define _SV_HELPWIN_HXX

#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>

// ------------------
// - HelpTextWindow -
// ------------------

class HelpTextWindow : public FloatingWindow
{
private:
    Point               maPos;
    Rectangle           maHelpArea; // Wenn naechste Hilfe fuers gleiche Rectangle, gleicher Text, dann Fenster stehen lassen

    Rectangle           maTextRect; // Bei umgebrochenen Text in QuickHelp

    String              maHelpText;
    String              maStatusText;

    Timer               maShowTimer;
    Timer               maHideTimer;

    sal_uInt16              mnHelpWinStyle;
    sal_uInt16              mnStyle;

protected:
                        DECL_LINK( TimerHdl, Timer* );
    virtual void        Paint( const Rectangle& );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual String      GetText() const;
    void                ImplShow();

public:
                        HelpTextWindow( Window* pParent, const String& rText, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle );
                        ~HelpTextWindow();

    const String&       GetHelpText() const { return maHelpText; }
    void                SetHelpText( const String& rHelpText );
    sal_uInt16          GetWinStyle() const { return mnHelpWinStyle; }
    sal_uInt16          GetStyle() const { return mnStyle; }

    // Nur merken:
    void                SetStatusText( const String& rStatusText ) { maStatusText = rStatusText; }
    void                SetHelpArea( const Rectangle& rRect ) { maHelpArea = rRect; }

    void                ShowHelp( sal_uInt16 nDelayMode );

    Size                CalcOutSize() const;
    const Rectangle&    GetHelpArea() const { return maHelpArea; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    sal_Bool                RegisterAccessibleParent();
    void                RevokeAccessibleParent();
};

void ImplShowHelpWindow( Window* pParent, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle,
        const String& rHelpText, const String& rStatusText,
        const Point& rScreenPos, const Rectangle* pHelpArea = NULL );
void ImplDestroyHelpWindow( bool bUpdateHideTime );
void ImplSetHelpWindowPos( Window* pHelpWindow, sal_uInt16 nHelpWinStyle, sal_uInt16 nStyle,
                            const Point& rPos, const Rectangle* pHelpArea );

#endif // _SV_HELPWIN_HXX
