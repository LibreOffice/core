/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLExportDataPilot.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:54:37 $
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

#ifndef _SC_XMLEXPORTDATAPILOT_HXX
#define _SC_XMLEXPORTDATAPILOT_HXX

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScXMLExport;
class ScDocument;
class ScDPSaveDimension;
class ScDPSaveData;
class ScDPDimensionSaveData;
class ScDPSaveGroupDimension;
class ScDPSaveNumGroupDimension;
struct ScDPNumGroupInfo;

class ScXMLExportDataPilot
{
    ScXMLExport&        rExport;
    ScDocument*         pDoc;

    rtl::OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
                                    const sal_Bool bIsString, const double dVal, const String& sVal) const;
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteDPFilter(const ScQueryParam& aQueryParam);

    void WriteFieldReference(ScDPSaveDimension* pDim);
    void WriteSortInfo(ScDPSaveDimension* pDim);
    void WriteAutoShowInfo(ScDPSaveDimension* pDim);
    void WriteLayoutInfo(ScDPSaveDimension* pDim);
    void WriteSubTotals(ScDPSaveDimension* pDim);
    void WriteMembers(ScDPSaveDimension* pDim);
    void WriteLevels(ScDPSaveDimension* pDim);
    void WriteDatePart(sal_Int32 nPart);
    void WriteNumGroupInfo(const ScDPNumGroupInfo& pGroupInfo);
    void WriteGroupDimAttributes(const ScDPSaveGroupDimension* pGroupDim);
    void WriteGroupDimElements(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteNumGroupDim(const ScDPSaveNumGroupDimension* pNumGroupDim);
    void WriteDimension(ScDPSaveDimension* pDim, const ScDPDimensionSaveData* pDimData);
    void WriteDimensions(ScDPSaveData* pDPSave);

public:
    ScXMLExportDataPilot(ScXMLExport& rExport);
    ~ScXMLExportDataPilot();
    void WriteDataPilots(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreaDoc);
};

#endif

