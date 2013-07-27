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



#ifndef _SV_MOREBTN_HXX
#define _SV_MOREBTN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/button.hxx>

struct ImplMoreButtonData;

// --------------
// - MoreButton -
// --------------

class VCL_DLLPUBLIC MoreButton : public PushButton
{
private:
    ImplMoreButtonData* mpMBData;
    sal_uLong               mnDelta;
    MapUnit             meUnit;
    sal_Bool                mbState;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      MoreButton( const MoreButton & );
    SAL_DLLPRIVATE      MoreButton& operator=( const MoreButton & );
    SAL_DLLPRIVATE void ShowState();

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );

public:
    explicit            MoreButton( Window* pParent, WinBits nStyle = 0 );
    explicit            MoreButton( Window* pParent, const ResId& );
    virtual             ~MoreButton();

    void                Click();

    void                AddWindow( Window* pWindow );
    void                RemoveWindow( Window* pWindow );

    void                SetDelta( sal_uLong nNewDelta ) { mnDelta = nNewDelta; }
    sal_uLong               GetDelta() const { return mnDelta; }

    void                SetMapUnit( MapUnit eNewUnit = MAP_PIXEL ) { meUnit = eNewUnit; }
    MapUnit             GetMapUnit() const { return meUnit; }

    using PushButton::SetState;
    void                SetState( sal_Bool bNewState = sal_True );
    sal_Bool                GetState() const { return mbState; }

    void                SetText( const XubString& rNewText );
    XubString           GetText() const;

    void                SetMoreText( const XubString& rNewText );
    void                SetLessText( const XubString& rNewText );
    XubString           GetMoreText() const;
    XubString           GetLessText() const;
};

inline void MoreButton::SetState( sal_Bool bNewState )
{
    if ( mbState != bNewState )
        Click();
}

#endif  // _SV_MOREBTN_HXX

