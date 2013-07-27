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



#ifndef _SV_WRKWIN_HXX
#define _SV_WRKWIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>

namespace com { namespace sun { namespace star { namespace uno { class Any; }}}}
struct SystemParentData;

// ----------------------
// - WorkWindow - Types -
// ----------------------

// Presentation Flags
#define PRESENTATION_HIDEALLAPPS    ((sal_uInt16)0x0001)
#define PRESENTATION_NOFULLSCREEN   ((sal_uInt16)0x0002)
#define PRESENTATION_NOAUTOSHOW     ((sal_uInt16)0x0004)

// --------------
// - WorkWindow -
// --------------

class VCL_DLLPUBLIC WorkWindow : public SystemWindow
{
private:
    sal_uInt16          mnPresentationFlags;
    sal_Bool            mbPresentationMode:1,
                    mbPresentationVisible:1,
                    mbPresentationFull:1,
                    mbFullScreenMode:1;

    SAL_DLLPRIVATE void ImplInitWorkWindowData();
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken );

private:
    SAL_DLLPRIVATE              WorkWindow( const WorkWindow& rWin );
    SAL_DLLPRIVATE WorkWindow&  operator =( const WorkWindow& rWin );

protected:
                        WorkWindow( WindowType nType );
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData = NULL );
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void ImplSetFrameState( sal_uLong aFrameState );

public:
    explicit        WorkWindow( Window* pParent, const ResId& );
    explicit        WorkWindow( Window* pParent, WinBits nStyle = WB_STDWORK );
    explicit        WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle = WB_STDWORK );
    explicit        WorkWindow( SystemParentData* pParent ); // Not in the REMOTE-Version
    virtual         ~WorkWindow();

    virtual sal_Bool    Close();

    /** The default value of nDisplay = -1 means "don't care" and
        allows to backends to use any screen [** or display? terminology!]
        they like (most probably the current one).

        NOTE: The default value cannot be 0, because 0 is a legitimate
        screen number.
     */
    void            ShowFullScreenMode( sal_Bool bFullScreenMode = sal_True, sal_Int32 nDisplay = -1 );
    void            EndFullScreenMode() { ShowFullScreenMode( sal_False ); }
    sal_Bool            IsFullScreenMode() const { return mbFullScreenMode; }

    void            StartPresentationMode( sal_Bool bPresentation = sal_True, sal_uInt16 nFlags = 0, sal_Int32 nDisplay = 0 );
    void            EndPresentationMode() {  StartPresentationMode( sal_False ); }
    sal_Bool            IsPresentationMode() const { return mbPresentationMode; }

    sal_Bool            IsMinimized() const;

    sal_Bool            SetPluginParent( SystemParentData* pParent );

    void            Minimize();
    void            Restore();

    void            Maximize( sal_Bool bMaximize = sal_True );
    sal_Bool            IsMaximized() const;
};

#endif // _SV_WRKWIN_HXX

