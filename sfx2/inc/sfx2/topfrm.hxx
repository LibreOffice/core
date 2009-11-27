/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: topfrm.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_TOPFRM_HXX
#define _SFX_TOPFRM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Reference.h>
#include <vcl/dockwin.hxx>
#include <vcl/wrkwin.hxx>

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>

class SfxViewShell;
namespace svtools { class AsynchronLink; }

// class SfxTopViewFrame -------------------------------------------------

class SfxTopViewFrame_Impl;
class SFX2_DLLPUBLIC SfxTopViewFrame : public SfxViewFrame
{
friend class SfxTopWindow_Impl;
friend class SfxTopViewFrame_Impl;

    SfxTopViewFrame_Impl*   pImp;
    svtools::AsynchronLink* pCloser;

protected:

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void            Activate( BOOL bUI );
    virtual void            Deactivate( BOOL bUI );
    virtual BOOL            Close();
    virtual                 ~SfxTopViewFrame();

public:
                            TYPEINFO();
                            SFX_DECL_INTERFACE(SFX_INTERFACE_SFXTOPFRM)

                            SfxTopViewFrame( SfxFrame* pFrame, SfxObjectShell *pDoc=NULL, USHORT nViewId=0 );

    virtual void            SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY );
    SAL_DLLPRIVATE virtual BOOL SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    virtual String          UpdateTitle();

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE void Exec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void GetState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE void INetExecute_Impl(SfxRequest &);
    SAL_DLLPRIVATE void INetState_Impl(SfxItemSet &);
//#endif
};

#endif // #ifndef _SFX_TOPFRM_HXX

