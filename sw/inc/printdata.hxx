/*************************************************************************
 *
 *  $RCSfile: printdata.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2001-08-08 21:40:10 $
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
#ifndef _SW_PRINTDATA_HXX
#define _SW_PRINTDATA_HXX


#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

struct SwPrintData
{
    sal_Bool bPrintGraphic, bPrintTable, bPrintDraw, bPrintControl, bPrintPageBackground,
             bPrintBlackFont, bPrintLeftPage, bPrintRightPage, bPrintReverse, bPrintProspect,
             bPrintSingleJobs, bPaperFromSetup, bModified;

    sal_Int16           nPrintPostIts;
    rtl::OUString       sFaxName;

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
        bModified               =
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
    sal_Bool IsPrintGraphic()   const { return bPrintGraphic; }
    sal_Bool IsPrintTable()     const { return bPrintTable; }
    sal_Bool IsPrintDraw()      const { return bPrintDraw; }
    sal_Bool IsPrintControl()   const { return bPrintControl; }
    sal_Bool IsPrintLeftPage()  const { return bPrintLeftPage; }
    sal_Bool IsPrintRightPage() const { return bPrintRightPage; }
    sal_Bool IsPrintReverse()   const { return bPrintReverse; }
    sal_Bool IsPaperFromSetup() const { return bPaperFromSetup; }
    sal_Bool IsPrintProspect()  const { return bPrintProspect; }
    sal_Bool IsPrintPageBackground() const { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const { return bPrintBlackFont;}
    sal_Bool IsPrintSingleJobs() const { return bPrintSingleJobs;}
    sal_Int16 GetPrintPostIts() const { return nPrintPostIts; }
    const rtl::OUString     GetFaxName() const{return sFaxName;}

    void SetPrintGraphic  ( sal_Bool b ) { doSetModified(); bPrintGraphic = b;}
    void SetPrintTable    ( sal_Bool b ) { doSetModified(); bPrintTable = b;}
    void SetPrintDraw     ( sal_Bool b ) { doSetModified(); bPrintDraw = b;}
    void SetPrintControl  ( sal_Bool b ) { doSetModified(); bPrintControl = b; }
    void SetPrintLeftPage ( sal_Bool b ) { doSetModified(); bPrintLeftPage = b;}
    void SetPrintRightPage( sal_Bool b ) { doSetModified(); bPrintRightPage = b;}
    void SetPrintReverse  ( sal_Bool b ) { doSetModified(); bPrintReverse = b;}
    void SetPaperFromSetup( sal_Bool b ) { doSetModified(); bPaperFromSetup = b;}
    void SetPrintPostIts    ( sal_Int16 n){ doSetModified(); nPrintPostIts = n; }
    void SetPrintProspect   ( sal_Bool b ) { doSetModified(); bPrintProspect = b; }
    void SetPrintPageBackground(sal_Bool b){ doSetModified(); bPrintPageBackground = b;}
    void SetPrintBlackFont(sal_Bool b){ doSetModified(); bPrintBlackFont = b;}
    void SetPrintSingleJobs(sal_Bool b){ doSetModified(); bPrintSingleJobs = b;}
    void SetFaxName(const rtl::OUString& rSet){sFaxName = rSet;}
    virtual void doSetModified () { bModified = sal_True;}
    sal_Bool WasModified () { return bModified; }
};

#endif  //_SW_PRINTDATA_HXX

