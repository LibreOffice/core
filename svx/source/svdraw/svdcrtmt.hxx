/*************************************************************************
 *
 *  $RCSfile: svdcrtmt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#ifndef _SVDCRTMT_HXX
#define _SVDCRTMT_HXX

#ifndef _SVDHDL_HXX
#include "svdhdl.hxx"
#endif

#ifndef _SVDCRTV_HXX
#include "svdcrtv.hxx"
#endif

#ifndef _SVDDRGM1_HXX
#include "svddrgm1.hxx"
#endif

//************************************************************
//   ImpSdrCreateLibObjMove
//************************************************************

class ImpSdrCreateLibObjMove : public SdrDragMove
{
    Point                       aObjCenter;
    Rectangle                   aObjRect;

private:
    SdrCreateView& View() const { return *((SdrCreateView*)&rView); }
    SdrPageView& CreatePV() const { return *View().pCreatePV; }
    SdrObject& CreateObj() const { return *View().pAktCreate; }

public:
    TYPEINFO();
    ImpSdrCreateLibObjMove(SdrDragView& rNewView): SdrDragMove(rNewView) {}

    virtual void Draw() const;
    virtual void Show();
    virtual void Hide();

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovAllPoints();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
};

//************************************************************
//   ImpSdrCreateLibObjResize
//************************************************************

class ImpSdrCreateLibObjResize : public SdrDragResize
{
    Rectangle                   aObjRect;
    FASTBOOL                    bForceOrtho;
private:
    SdrCreateView& View() const { return *((SdrCreateView*)&rView); }
    SdrPageView& CreatePV() const { return *View().pCreatePV; }
    SdrObject& CreateObj() const { return *View().pAktCreate; }

public:
    TYPEINFO();
    ImpSdrCreateLibObjResize(SdrDragView& rNewView): SdrDragResize(rNewView) {}

    virtual void Draw() const;
    virtual void Show();
    virtual void Hide();

    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void MovAllPoints();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDCRTMT_HXX

