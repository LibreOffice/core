/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuinsert.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:28:45 $
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


#ifndef SC_FUINSERT_HXX
#define SC_FUINSERT_HXX

#include "fupoor.hxx"

class FuInsertGraphic : public FuPoor
{
 public:
    FuInsertGraphic( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertGraphic();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertOLE : public FuPoor
{
 public:
    FuInsertOLE( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertOLE();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertChart : public FuPoor
{
 public:
    FuInsertChart( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertChart();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

/************************************************************************/

class FuInsertMedia : public FuPoor
{
 public:
    FuInsertMedia( ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuInsertMedia();

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren
};

#endif      // _SD_FUINSERT_HXX

