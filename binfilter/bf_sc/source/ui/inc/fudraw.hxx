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

#ifndef SC_FUDRAW_HXX
#define SC_FUDRAW_HXX

#ifndef _SC_FUPOOR_HXX
#include "fupoor.hxx"
#endif

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

class FuDraw : public FuPoor
{
 protected:
    Pointer aNewPointer;
    Pointer aOldPointer;

 public:
    FuDraw(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuDraw();






    // #97016# II

};



} //namespace binfilter
#endif		// _SD_FUDRAW_HXX
