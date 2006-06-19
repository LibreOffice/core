/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 21:50:28 $
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
#ifndef _EXPL_HXX
#define _EXPL_HXX


#ifndef _SV_IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

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
