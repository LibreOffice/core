/*************************************************************************
 *
 *  $RCSfile: Client.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 11:30:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_CLIENT_HXX
#define SD_CLIENT_HXX


#ifndef _SFX_CLIENTSH_HXX //autogen
#include <sfx2/clientsh.hxx>
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

class Client
    : public SfxInPlaceClient
{
public:
    Client (SdrOle2Obj* pObj, ViewShell* pSdViewShell, ::Window* pWindow);
    virtual ~Client (void);

    virtual void    RequestObjAreaPixel(const Rectangle& rRect);
    virtual void    ViewChanged(USHORT nAspect);
    virtual void    MakeViewData();
    virtual void    MakeVisible();

    SdrOle2Obj*     GetSdrOle2Obj() const { return pSdrOle2Obj; }

    void            SetSdrGrafObj(SdrGrafObj* pObj) { pSdrGrafObj = pObj; }
    SdrGrafObj*     GetSdrGrafObj() const { return pSdrGrafObj; }

    void            SetOutlinerParaObj (OutlinerParaObject* pObj) { pOutlinerParaObj = pObj; }
    OutlinerParaObject*     GetOutlinerParaObject () const { return pOutlinerParaObj; }

    BOOL            IsUIActive() const
                    { return aProt.IsUIActive(); }

protected:
    ViewShell*  pViewShell;
    SdrOle2Obj*     pSdrOle2Obj;
    SdrGrafObj*     pSdrGrafObj;
    OutlinerParaObject* pOutlinerParaObj;

    virtual void    UIActivate(BOOL bActivate);
};

} // end of namespace sd

#endif

