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



#ifndef _SV_IMGCTRL_HXX
#define _SV_IMGCTRL_HXX

#include <vcl/dllapi.h>

#include <vcl/fixed.hxx>
#include <vcl/bitmapex.hxx>

// ----------------
// - ImageControl -
// ----------------

class VCL_DLLPUBLIC ImageControl : public FixedImage
{
private:
    ::sal_Int16     mnScaleMode;

public:
                    ImageControl( Window* pParent, WinBits nStyle = 0 );
                    ImageControl( Window* pParent, const ResId& rResId );

    // set/get the scale mode. This is one of the css.awt.ImageScaleMode constants
    void            SetScaleMode( const ::sal_Int16 _nMode );
    ::sal_Int16     GetScaleMode() const { return mnScaleMode; }

    virtual void    Resize();
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    GetFocus();
    virtual void    LoseFocus();

protected:
    void    ImplDraw( OutputDevice& rDev, sal_uLong nDrawFlags, const Point& rPos, const Size& rSize ) const;
private:
    using FixedImage::ImplDraw;
};

#endif  // _SV_IMGCTRL_HXX
