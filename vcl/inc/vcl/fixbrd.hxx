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



#ifndef _SV_FIXBRD_HXX
#define _SV_FIXBRD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/decoview.hxx>
#include <vcl/ctrl.hxx>

// ---------------------
// - FixedBorder-Types -
// ---------------------

#define FIXEDBORDER_TYPE_IN                     (FRAME_DRAW_IN)
#define FIXEDBORDER_TYPE_OUT                    (FRAME_DRAW_OUT)
#define FIXEDBORDER_TYPE_GROUP                  (FRAME_DRAW_GROUP)
#define FIXEDBORDER_TYPE_DOUBLEIN               (FRAME_DRAW_DOUBLEIN)
#define FIXEDBORDER_TYPE_DOUBLEOUT              (FRAME_DRAW_DOUBLEOUT)

// ---------------
// - FixedBorder -
// ---------------

class VCL_DLLPUBLIC FixedBorder : public Control
{
private:
    sal_uInt16          mnType;
    sal_Bool            mbTransparent;

private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                              const Point& rPos, const Size& rSize );

public:
                    FixedBorder( Window* pParent, WinBits nStyle = 0 );
                    FixedBorder( Window* pParent, const ResId& rResId );
                    ~FixedBorder();

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SetTransparent( sal_Bool bTransparent );
    sal_Bool            IsTransparent() const { return mbTransparent; }
    void            SetBorderType( sal_uInt16 nType );
    sal_uInt16          GetBorderType() const { return mnType; }
};

#endif  // _SV_FIXBRD_HXX
