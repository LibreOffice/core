/*************************************************************************
 *
 *  $RCSfile: ViewElementListProvider.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_VIEWELEMENTLISTPROVIDER_HXX
#define _CHART2_VIEWELEMENTLISTPROVIDER_HXX

#ifndef _XTABLE_HXX
#include <svx/xtable.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

class FontList;
class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DrawModelWrapper;
class NumberFormatterWrapper;

class ViewElementListProvider
{
public:
    ViewElementListProvider( DrawModelWrapper* pDrawModelWrapper, NumberFormatterWrapper* pNumberFormatterWrapper );
    virtual ~ViewElementListProvider();

    XColorTable*    GetColorTable() const;//from class SdrModel
    XDashList*      GetDashList() const;//from class SdrModel
    XLineEndList*   GetLineEndList() const;//from class SdrModel
    XGradientList*  GetGradientList() const;//from class SdrModel
    XHatchList*     GetHatchList() const;//from class SdrModel
    XBitmapList*    GetBitmapList() const;//from class SdrModel

    //create chartspecific symbols for linecharts
    SdrObjList*     GetSymbolList() const;// in old chart called GetSdrObjList

    FontList*       getFontList() const;
    //SfxPrinter*   getPrinter();

        //infrastructure
    /** introduced for #101318#:

        The chart-internal number formatter is necessary to render values for
        the UI with a higher standard precision than the one you might get
        externally (from Calc).  As you should not modify the precision of an
        external number formatter, this one comes in handy.

        Note that for the dialog only the standard format is used, i.e., any
        user-defined formats that are only available in the external formatter
        are not required here.

        This method is used by SchAttribTabDlg::PageCreated (attrib.cxx)

        @returns the chart internal number formatter
     */
    SvNumberFormatter * GetOwnNumberFormatter() const;
    SvNumberFormatter * GetNumFormatter() const;

private:
    DrawModelWrapper*   m_pDrawModelWrapper;
    mutable FontList*   m_pFontList;
    mutable NumberFormatterWrapper* m_pNumberFormatterWrapper;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
