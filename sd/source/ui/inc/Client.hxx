/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Client.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:31:58 $
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

#ifndef SD_CLIENT_HXX
#define SD_CLIENT_HXX


#ifndef _SFX_CLIENTSH_HXX //autogen
#include <sfx2/ipclient.hxx>
#endif
class SdrGrafObj;
class SdrOle2Obj;
class OutlinerParaObject;

namespace sd {

class ViewShell;

/*************************************************************************
|*
|* Client
|*
\************************************************************************/

class Client : public SfxInPlaceClient
{
    ViewShell*      mpViewShell;
    SdrOle2Obj*     pSdrOle2Obj;
    SdrGrafObj*     pSdrGrafObj;
    OutlinerParaObject* pOutlinerParaObj;

    virtual void    ObjectAreaChanged();
    virtual void    RequestNewObjectArea( Rectangle& );
    virtual void    ViewChanged();
    virtual void    MakeVisible();

public:
    Client (SdrOle2Obj* pObj, ViewShell* pSdViewShell, ::Window* pWindow);
    virtual ~Client (void);

    SdrOle2Obj*     GetSdrOle2Obj() const { return pSdrOle2Obj; }
    void            SetSdrGrafObj(SdrGrafObj* pObj) { pSdrGrafObj = pObj; }
    SdrGrafObj*     GetSdrGrafObj() const { return pSdrGrafObj; }
    void            SetOutlinerParaObj (OutlinerParaObject* pObj) { pOutlinerParaObj = pObj; }
    OutlinerParaObject*     GetOutlinerParaObject () const { return pOutlinerParaObj; }
};

} // end of namespace sd

#endif

