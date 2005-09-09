/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cookiedg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:18:27 $
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

#ifndef UUI_COOKIEDG_HXX
#define UUI_COOKIEDG_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif

struct CntHTTPCookieRequest;

//============================================================================
class CookiesDialog : public ModalDialog
{
    FixedBitmap             maCookieFB;
    FixedText               maCookieFT;
    FixedLine               maInFutureLine;
    RadioButton             maInFutureSendBtn;
    RadioButton             maInFutureIgnoreBtn;
    RadioButton             maInFutureInteractiveBtn;
    GroupBox                maInFutureGB;
    PushButton              maIgnoreBtn;
    PushButton              maSendBtn;

    CntHTTPCookieRequest*   mpCookieRequest;

    DECL_LINK( ButtonHdl_Impl, PushButton * );

public:
    CookiesDialog( Window* pParent, CntHTTPCookieRequest* pRequest,
                   ResMgr* pResMgr );

    virtual short Execute();
};

#endif // UUI_COOKIEDG_HXX

