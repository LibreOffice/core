/*************************************************************************
 *
 *  $RCSfile: prtopt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:41 $
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
#ifndef _PRTOPT_HXX
#define _PRTOPT_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

class SwPrintOptions : public utl::ConfigItem
{
    sal_Bool
         bPrintGraphic       :1,
         bPrintTable         :1,
         bPrintDraw          :1,
         bPrintControl       :1,
         bPrintPageBackground:1,
         bPrintBlackFont     :1,

         bPrintLeftPage      :1,
         bPrintRightPage     :1,
         bReverse            :1,
         bPrintProspect      :1,
         bPrintSingleJobs    :1,

         bPaperFromSetup     :1;

    sal_uInt32  nPrintPostIts;
    rtl::OUString       sFaxName;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwPrintOptions(sal_Bool bWeb);
    virtual ~SwPrintOptions();

    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    virtual void            Commit();

    sal_Bool IsPrintGraphic()   const { return bPrintGraphic; }
    sal_Bool IsPrintTable()     const { return bPrintTable; }
    sal_Bool IsPrintDraw()      const { return bPrintDraw; }
    sal_Bool IsPrintControl()   const { return bPrintControl; }
    sal_Bool IsPrintLeftPage()  const { return bPrintLeftPage; }
    sal_Bool IsPrintRightPage() const { return bPrintRightPage; }
    sal_Bool IsPrintReverse()   const { return bReverse; }
    sal_Bool IsPaperFromSetup() const { return bPaperFromSetup; }
    sal_Bool IsPrintProspect()  const { return bPrintProspect; }
    sal_Bool IsPrintPageBackground() const { return bPrintPageBackground; }
    sal_Bool IsPrintBlackFont() const { return bPrintBlackFont;}
    sal_Bool IsPrintSingleJobs() const { return bPrintSingleJobs;}
    sal_uInt32 GetPrintPostIts() const { return nPrintPostIts; }
    const rtl::OUString     GetFaxName() const{return sFaxName;}

    void SetPrintGraphic  ( sal_Bool b ) { SetModified(); bPrintGraphic = b;}
    void SetPrintTable    ( sal_Bool b ) { SetModified(); bPrintTable = b;}
    void SetPrintDraw     ( sal_Bool b ) { SetModified(); bPrintDraw = b;}
    void SetPrintControl  ( sal_Bool b ) { SetModified(); bPrintControl = b; }
    void SetPrintLeftPage ( sal_Bool b ) { SetModified(); bPrintLeftPage = b;}
    void SetPrintRightPage( sal_Bool b ) { SetModified(); bPrintRightPage = b;}
    void SetPrintReverse  ( sal_Bool b ) { SetModified(); bReverse = b;}
    void SetPaperFromSetup( sal_Bool b ) { SetModified(); bPaperFromSetup = b;}
    void SetPrintPostIts    ( sal_uInt32 n){ SetModified(); nPrintPostIts = n; }
    void SetPrintProspect   ( sal_Bool b ) { SetModified(); bPrintProspect = b; }
    void SetPrintPageBackground(sal_Bool b){ SetModified(); bPrintPageBackground = b;}
    void SetPrintBlackFont(sal_Bool b){ SetModified(); bPrintBlackFont = b;}
    void SetPrintSingleJobs(sal_Bool b){ SetModified(); bPrintSingleJobs = b;}
    void SetFaxName(const rtl::OUString& rSet){sFaxName = rSet;}
};

#endif

