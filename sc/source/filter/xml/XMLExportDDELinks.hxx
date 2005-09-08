/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLExportDDELinks.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:53:56 $
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

#ifndef _SC_XMLEXPORTDDELINKS_HXX
#define _SC_XMLEXPORTDDELINKS_HXX

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

class String;
class ScXMLExport;

class ScXMLExportDDELinks
{
    ScXMLExport&        rExport;

    sal_Bool            CellsEqual(const sal_Bool bPrevEmpty, const sal_Bool bPrevString, const String& sPrevValue, const double& fPrevValue,
                                    const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue);
    void                WriteCell(const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue, const sal_Int32 nRepeat);
    void                WriteTable(const sal_Int32 nPos);
public:
    ScXMLExportDDELinks(ScXMLExport& rExport);
    ~ScXMLExportDDELinks();
    void WriteDDELinks(::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
};

#endif


