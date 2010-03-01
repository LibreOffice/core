/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

