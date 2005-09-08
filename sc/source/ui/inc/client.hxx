/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: client.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:15:11 $
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

#ifndef SC_CLIENT_HXX
#define SC_CLIENT_HXX

#ifndef _SFX_CLIENTSH_HXX //autogen
#include <sfx2/ipclient.hxx>
#endif

class ScDocument;
class ScTabViewShell;
class SdrOle2Obj;
class SdrGrafObj;
class SdrModel;

class ScClient : public SfxInPlaceClient
{
private:
    SdrModel*       pModel;
    SdrGrafObj*     pGrafEdit;

    virtual void    ObjectAreaChanged();
    virtual void    RequestNewObjectArea( Rectangle& );
    virtual void    ViewChanged();
    virtual void    MakeVisible();

public:
                    ScClient( ScTabViewShell* pViewShell, Window* pDraw, SdrModel* pSdrModel, SdrOle2Obj* pObj );
    virtual         ~ScClient();

    SdrGrafObj*     GetGrafEdit() const             { return pGrafEdit; }
    void            SetGrafEdit(SdrGrafObj* pNew)   { pGrafEdit = pNew; }
    SdrOle2Obj*     GetDrawObj();
};



#endif

