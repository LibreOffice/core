/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: expl.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#include "expl.hxx"
#include "shapes.hxx"
#include "invader.hrc"
#include <vcl/outdev.hxx>

Explosion::Explosion(ResMgr* pRes) :
            ExplListe(0,1),
            pExpl1(0L),
            pExpl2(0L),
            pExpl3(0L)
{
    pExpl1 = ImplLoadImage( EXPLOS1, pRes );
    pExpl2 = ImplLoadImage( EXPLOS2, pRes );
    pExpl3 = ImplLoadImage( EXPLOS3, pRes );
}

Explosion::~Explosion()
{
    delete pExpl1;
    delete pExpl2;
    delete pExpl3;
}

void Explosion::Paint(OutputDevice& rDev)
{
    unsigned long i;
    for( i = 0; i < Count(); i++ )
    {
        switch(GetMode(i))
        {
            case EXPL1:
                rDev.DrawImage(GetPoint(i),*pExpl1);
                SetMode(i,EXPL2);
                break;
            case EXPL2:
                rDev.DrawImage(GetPoint(i),*pExpl2);
                SetMode(i,EXPL3);
                break;
            case EXPL3:
                rDev.DrawImage(GetPoint(i),*pExpl3);
                SetMode(i,EXPL4);
                break;
            case EXPL4:
                rDev.DrawImage(GetPoint(i),*pExpl2);
                SetMode(i,EXPL5);
                break;
            case EXPL5:
                rDev.DrawImage(GetPoint(i),*pExpl3);
                SetMode(i,EXPL6);
                break;
            case EXPL6:
                rDev.DrawImage(GetPoint(i),*pExpl2);
                SetMode(i,EXPL7);
                break;
            case EXPL7:
                rDev.DrawImage(GetPoint(i),*pExpl1);
                SetMode(i,EXPLNONE);
                break;
            case EXPLNONE:
                SetMode(i,EXPLDEL);
                break;
            case EXPL8:
                break;
            case EXPLDEL:
                break;
        }
    }

//  RemoveExpl();
}

BOOL Explosion::RemoveExpl()
{
    Expl_Impl* pWork;

    for(long i=Count()-1; i >= 0; i--)
    {
        if(GetMode(i) == EXPLDEL)
        {
            pWork = GetObject(i);
            Remove(pWork);
            delete pWork;
        }
    }

    if(Count())
        return FALSE;
    else
        return TRUE;
}

void Explosion::ClearAll()
{
    unsigned long i;
    for( i = 0;  i < Count(); i++ )
        delete GetObject(i);

    Clear();
}

void Explosion::InsertExpl(Point& rPoint)
{
    Expl_Impl* pWork = new Expl_Impl();

    pWork->aPos     = rPoint;
    pWork->eMode    = EXPL1;
    Insert(pWork);
}
