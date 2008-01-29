/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syschild.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:37:31 $
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

#ifndef _SV_SYSCHILD_HXX
#define _SV_SYSCHILD_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

struct SystemEnvData;
struct SystemWindowData;

// ---------------------
// - SystemChildWindow -
// ---------------------

class VCL_DLLPUBLIC SystemChildWindow : public Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInitSysChild( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow = TRUE );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          SystemChildWindow (const SystemChildWindow &);
    SAL_DLLPRIVATE          SystemChildWindow & operator= (const SystemChildWindow &);

public:
                            SystemChildWindow( Window* pParent, WinBits nStyle = 0 );
                            // create a SystemChildWindow using the given SystemWindowData
                            SystemChildWindow( Window* pParent, WinBits nStyle, SystemWindowData *pData, BOOL bShow = TRUE );
                            SystemChildWindow( Window* pParent, const ResId& rResId );
                            ~SystemChildWindow();

    const SystemEnvData*    GetSystemData() const;

    //  per default systemchildwindows erase their background for better plugin support
    //  however, this might not always be required
    void                    EnableEraseBackground( BOOL bEnable = TRUE );
    BOOL                    IsEraseBackgroundEnabled();
};

#endif // _SV_SYSCHILD_HXX
