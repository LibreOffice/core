/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SW_PRINTDATA_HXX
#define _SW_PRINTDATA_HXX


#include <sal/types.h>
#include "rtl/ustring.hxx"
namespace binfilter {

struct SwPrintData
{
    sal_Bool bPrintGraphic, bPrintTable, bPrintDraw, bPrintControl, bPrintPageBackground, 
             bPrintBlackFont, bPrintLeftPage, bPrintRightPage, bPrintReverse, bPrintProspect, 
             bPrintSingleJobs, bPaperFromSetup, bModified;

    sal_Int16           nPrintPostIts;
    ::rtl::OUString       sFaxName;

    SwPrintData()
    {
        bPrintGraphic           =
        bPrintTable             =
        bPrintDraw              =
        bPrintControl           =
        bPrintLeftPage          =
        bPrintRightPage         =
        bPrintPageBackground    = sal_True;

        bPaperFromSetup         =
        bPrintReverse           =
        bPrintProspect          =
        bPrintSingleJobs        =
        bModified        		=
        bPrintBlackFont         = sal_False;

        nPrintPostIts           = 0;
    }

    sal_Bool operator==(const SwPrintData& rData)const
    {
        return
        bPrintGraphic       ==   rData.bPrintGraphic       &&
        bPrintTable         ==   rData.bPrintTable         &&
        bPrintDraw          ==   rData.bPrintDraw          &&
        bPrintControl       ==   rData.bPrintControl       &&
        bPrintPageBackground==   rData.bPrintPageBackground&&
        bPrintBlackFont     ==   rData.bPrintBlackFont     &&
        bPrintLeftPage      ==   rData.bPrintLeftPage      &&
        bPrintRightPage     ==   rData.bPrintRightPage     &&
        bPrintReverse       ==   rData.bPrintReverse       &&
        bPrintProspect      ==   rData.bPrintProspect      &&
        bPrintSingleJobs    ==   rData.bPrintSingleJobs    &&
        bPaperFromSetup     ==   rData.bPaperFromSetup     &&
        nPrintPostIts       ==   rData.nPrintPostIts       &&
        sFaxName            ==   rData.sFaxName;
    }
    sal_Bool IsPrintGraphic()	const { return bPrintGraphic; }
    sal_Bool IsPrintTable()		const { return bPrintTable; }
    sal_Bool IsPrintDraw()		const { return bPrintDraw; }
    sal_Bool IsPrintControl()	const { return bPrintControl; }
    sal_Bool IsPrintLeftPage()	const { return bPrintLeftPage; }
    sal_Bool IsPrintRightPage()	const { return bPrintRightPage; }
    sal_Bool IsPrintReverse()   const { return bPrintReverse; }
    sal_Bool IsPaperFromSetup()	const { return bPaperFromSetup; }
    sal_Bool IsPrintProspect()	const { return bPrintProspect; }
    sal_Bool IsPrintPageBackground() const { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const { return bPrintBlackFont;}
    sal_Bool IsPrintSingleJobs() const { return bPrintSingleJobs;}
    sal_Int16 GetPrintPostIts() const { return nPrintPostIts; }
    const ::rtl::OUString		GetFaxName() const{return sFaxName;}

    void SetPrintGraphic  ( sal_Bool b ) { doSetModified(); bPrintGraphic = b;}
    void SetPrintTable	  ( sal_Bool b ) { doSetModified(); bPrintTable = b;}
    void SetPrintDraw	  ( sal_Bool b ) { doSetModified(); bPrintDraw = b;}
    void SetPrintControl  ( sal_Bool b ) { doSetModified(); bPrintControl = b; }
    void SetPrintLeftPage ( sal_Bool b ) { doSetModified(); bPrintLeftPage = b;}
    void SetPrintRightPage( sal_Bool b ) { doSetModified(); bPrintRightPage = b;}
    void SetPrintReverse  ( sal_Bool b ) { doSetModified(); bPrintReverse = b;}
    void SetPaperFromSetup( sal_Bool b ) { doSetModified(); bPaperFromSetup = b;}
    void SetPrintPostIts    ( sal_Int16 n){ doSetModified(); nPrintPostIts = n; }
    void SetPrintProspect	( sal_Bool b ) { doSetModified(); bPrintProspect = b; }
    void SetPrintPageBackground(sal_Bool b){ doSetModified(); bPrintPageBackground = b;}
    void SetPrintBlackFont(sal_Bool b){ doSetModified(); bPrintBlackFont = b;}
    void SetPrintSingleJobs(sal_Bool b){ doSetModified(); bPrintSingleJobs = b;}
    void SetFaxName(const ::rtl::OUString& rSet){sFaxName = rSet;}
    virtual void doSetModified () { bModified = sal_True;}
    sal_Bool WasModified () { return bModified; }
};

} //namespace binfilter
#endif  //_SW_PRINTDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
