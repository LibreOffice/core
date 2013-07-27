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



#ifndef _SV_DOCKINGAREA_HXX
#define _SV_DOCKINGAREA_HXX

#include <vcl/sv.h>
#include <vcl/window.hxx>

// ------------------------------------------
//  A simple container for docked toolbars
//  - its main purpose is theming support
// ------------------------------------------

class VCL_DLLPUBLIC DockingAreaWindow : public Window
{
    class ImplData;

private:
    ImplData*       mpImplData;

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE  DockingAreaWindow (const DockingAreaWindow &);
    SAL_DLLPRIVATE  DockingAreaWindow & operator= (const DockingAreaWindow &);

public:
    explicit        DockingAreaWindow( Window* pParent );
    virtual         ~DockingAreaWindow();

    void            SetAlign( WindowAlign eNewAlign );
    WindowAlign     GetAlign() const;
    sal_Bool            IsHorizontal() const;

    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual void    StateChanged( StateChangedType nType );

};

#endif // _SV_SYSWIN_HXX
