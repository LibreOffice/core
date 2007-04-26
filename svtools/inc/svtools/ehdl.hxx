/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ehdl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:41:22 $
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

#ifndef _EHDL_HXX
#define _EHDL_HXX

#ifndef __RSC

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _EINF_HXX
#include <tools/errinf.hxx>
#endif

class Window;
class ResMgr;

class SVT_DLLPUBLIC SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            USHORT nCtxIdP, Window *pWin=0,
            USHORT nResIdP=USHRT_MAX, ResMgr *pMgrP=0);
    SfxErrorContext(
            USHORT nCtxIdP, const String &aArg1, Window *pWin=0,
            USHORT nResIdP=USHRT_MAX, ResMgr *pMgrP=0);
    virtual BOOL GetString(ULONG nErrId, String &rStr);

private:
    USHORT nCtxId;
    USHORT nResId;
    ResMgr *pMgr;
    String aArg1;
};

class SVT_DLLPUBLIC SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(USHORT nId, ULONG lStart, ULONG lEnd, ResMgr *pMgr=0);
    ~SfxErrorHandler();

protected:
    virtual BOOL     GetErrorString(ULONG lErrId, String &, USHORT&) const;
    virtual BOOL     GetMessageString(ULONG lErrId, String &, USHORT&) const;

private:

    ULONG            lStart;
    ULONG            lEnd;
    USHORT           nId;
    ResMgr          *pMgr;
    ResMgr          *pFreeMgr;

//#if 0 // _SOLAR__PRIVATE
    SVT_DLLPRIVATE BOOL             GetClassString(ULONG lErrId, String &) const;
//#endif
    virtual BOOL     CreateString(
                         const ErrorInfo *, String &, USHORT &) const;
};

#endif

#endif

