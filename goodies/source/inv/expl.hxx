/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: expl.hxx,v $
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
#ifndef _EXPL_HXX
#define _EXPL_HXX


#include <vcl/image.hxx>
#include <vcl/outdev.hxx>
#include <tools/list.hxx>
#include <tools/gen.hxx>

enum ExplMode { EXPL1, EXPL2, EXPL3, EXPL4, EXPL5, EXPL6, EXPL7, EXPL8,
                EXPLNONE, EXPLDEL };

struct Expl_Impl
{
    Point           aPos;
    ExplMode        eMode;
};

DECLARE_LIST(ExplListe, Expl_Impl*)

class Explosion : public ExplListe
{
    private:
        Image*      pExpl1;
        Image*      pExpl2;
        Image*      pExpl3;

    public:
                    Explosion(ResMgr* pRes);
                    ~Explosion();

        void        Paint(OutputDevice& rDev);
        ExplMode    GetMode(long nWert) { return GetObject(nWert)->eMode; }
        Point&      GetPoint(long nWert) { return GetObject(nWert)->aPos; }
        void        SetMode(long nWert, enum ExplMode nMode)
                        { GetObject(nWert)->eMode = nMode; }
        BOOL        RemoveExpl();
        void        ClearAll();
        void        InsertExpl(Point& rPoint);
};

#endif
