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



#ifndef _SV_HELP_HXX
#define _SV_HELP_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class Point;
class Rectangle;
class Window;

// --------------
// - Help-Types -
// --------------

#define QUICKHELP_LEFT              ((sal_uInt16)0x0001)
#define QUICKHELP_CENTER            ((sal_uInt16)0x0002)
#define QUICKHELP_RIGHT             ((sal_uInt16)0x0004)
#define QUICKHELP_TOP               ((sal_uInt16)0x0008)
#define QUICKHELP_VCENTER           ((sal_uInt16)0x0010)
#define QUICKHELP_BOTTOM            ((sal_uInt16)0x0020)
#define QUICKHELP_NOAUTOPOS         (QUICKHELP_LEFT | QUICKHELP_CENTER | QUICKHELP_RIGHT | QUICKHELP_TOP | QUICKHELP_VCENTER | QUICKHELP_BOTTOM)
#define QUICKHELP_CTRLTEXT          ((sal_uInt16)0x0040)
/// force the existent tip window to be re-positioned, even if the previous incarnation has the same text. Applies to ShowBallon and ShowQuickHelp.
#define QUICKHELP_FORCE_REPOSITION  ((sal_uInt16)0x0080)
/// no delay when opening the quick help. Applies to ShowBallon and ShowQuickHelp
#define QUICKHELP_NO_DELAY          ((sal_uInt16)0x0100)
/// force balloon-style in ShowTip
#define QUICKHELP_TIP_STYLE_BALLOON ((sal_uInt16)0x0200)
#define QUICKHELP_NOEVADEPOINTER    ((sal_uInt16)0x4000)
#define QUICKHELP_BIDI_RTL          ((sal_uInt16)0x8000)

// By changes you must also change: rsc/vclrsc.hxx
#define OOO_HELP_INDEX          ".help:index"
#define OOO_HELP_HELPONHELP     ".help:helponhelp"

// --------
// - Help -
// --------

class VCL_DLLPUBLIC Help
{
private:
    String              maHelpFile;

public:
                        Help();
    virtual             ~Help();

    void                SetHelpFile( const String& rFileName ) { maHelpFile = rFileName; }
    const String&       GetHelpFile() const { return maHelpFile; }

    virtual sal_Bool    Start( const XubString& rHelpId, const Window* pWindow );
    virtual sal_Bool    SearchKeyword( const XubString& rKeyWord );
    virtual void        OpenHelpAgent( const rtl::OString& rHelpId );
    virtual XubString   GetHelpText( const String& aHelpURL, const Window* pWindow );

    static void         EnableContextHelp();
    static void         DisableContextHelp();
    static sal_Bool     IsContextHelpEnabled();
    static sal_Bool     StartContextHelp();

    static void         EnableExtHelp();
    static void         DisableExtHelp();
    static sal_Bool     IsExtHelpEnabled();
    static sal_Bool     StartExtHelp();
    static sal_Bool     EndExtHelp();
    static sal_Bool     IsExtHelpActive();

    static void         EnableBalloonHelp();
    static void         DisableBalloonHelp();
    static sal_Bool     IsBalloonHelpEnabled();
    static sal_Bool     ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const XubString& rHelpText );
    static sal_Bool     ShowBalloon( Window* pParent,
                                     const Point& rScreenPos,
                                     const Rectangle&,
                                     const XubString& rHelpText );

    static void         EnableQuickHelp();
    static void         DisableQuickHelp();
    static sal_Bool     IsQuickHelpEnabled();
    static sal_Bool     ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       const XubString& rLongHelpText,
                                       sal_uInt16 nStyle = 0 );
    static sal_Bool     ShowQuickHelp( Window* pParent,
                                       const Rectangle& rScreenRect,
                                       const XubString& rHelpText,
                                       sal_uInt16 nStyle = 0 )
                            { return Help::ShowQuickHelp( pParent, rScreenRect, rHelpText, XubString(), nStyle ); }

    static void         HideBalloonAndQuickHelp();

    static sal_uLong    ShowTip( Window* pParent,
                                 const Rectangle& rScreenRect,
                                 const XubString& rText, sal_uInt16 nStyle = 0 );
    static void         UpdateTip( sal_uLong nId,
                                   Window* pParent,
                                   const Rectangle& rScreenRect,
                                   const XubString& rText );
    static void         HideTip( sal_uLong nId );
};

#endif  // _SV_HELP_HXX
