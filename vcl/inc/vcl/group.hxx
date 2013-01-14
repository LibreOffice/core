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



#ifndef _SV_GROUP_HXX
#define _SV_GROUP_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>

// ------------
// - GroupBox -
// ------------

class VCL_DLLPUBLIC GroupBox : public Control
{
private:
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE WinBits  ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void     ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void     ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                                      const Point& rPos, const Size& rSize, bool bLayout = false );

    virtual void    FillLayoutData() const;
    virtual const Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

public:
    explicit        GroupBox( Window* pParent, WinBits nStyle = 0 );
    explicit        GroupBox( Window* pParent, const ResId& );

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    Resize();
    virtual void    StateChanged( StateChangedType nType );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

#endif  // _SV_GROUP_HXX

