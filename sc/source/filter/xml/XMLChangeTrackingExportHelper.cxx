/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingExportHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-01-15 14:51:12 $
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

#ifndef _SC_XMLCHANGETRACKINGEXPORTHELPER_HXX
#include "XMLChangeTrackingExportHelper.hxx"
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#include <list>

#define SC_CHANGE_ID_PREFIX "ct"

using namespace ::com::sun::star;

ScChangeTrackingExportHelper::ScChangeTrackingExportHelper(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pChangeTrack(NULL),
    sChangeIDPrefix(RTL_CONSTASCII_USTRINGPARAM(SC_CHANGE_ID_PREFIX))
{
    pChangeTrack = rExport.GetDocument()->GetChangeTrack();
    if (pChangeTrack)
    {
        sTrackedChanges = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_tracked_changes)));
    }
}

ScChangeTrackingExportHelper::~ScChangeTrackingExportHelper()
{
}

rtl::OUString ScChangeTrackingExportHelper::GetChangeID(const sal_uInt32 nActionNumber)
{
    rtl::OUStringBuffer sBuffer(sChangeIDPrefix);
    SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nActionNumber));
    return sBuffer.makeStringAndClear();
}

void ScChangeTrackingExportHelper::WriteChangeInfo(ScChangeAction* pAction)
{
    rtl::OUString sAuthor(pAction->GetUser());
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_author, sAuthor);
    rtl::OUStringBuffer sDate;
    DateTime aDateTime(pAction->GetDateTimeUTC());
    util::DateTime aAPIDateTime;
    aAPIDateTime.Year = aDateTime.GetYear();
    aAPIDateTime.Month = aDateTime.GetMonth();
    aAPIDateTime.Day = aDateTime.GetDay();
    aAPIDateTime.Hours = aDateTime.GetHour();
    aAPIDateTime.Minutes = aDateTime.GetMin();
    aAPIDateTime.Seconds = aDateTime.GetSec();
    aAPIDateTime.HundredthSeconds = aDateTime.Get100Sec();
    SvXMLUnitConverter::convertDateTime(sDate, aAPIDateTime);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_date_time, sDate.makeStringAndClear());
    SvXMLElementExport aElemInfo (rExport, XML_NAMESPACE_OFFICE, sXML_change_info, sal_True, sal_True);
    rtl::OUString sComment(pAction->GetComment());
    if (sComment.getLength())
        rExport.GetDocHandler()->characters(sComment);
}

void ScChangeTrackingExportHelper::WriteEmptyCell()
{
    SvXMLElementExport aElemEmptyCell(rExport, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::WriteValueCell(const ScBaseCell* pCell)
{
    const ScValueCell* pValueCell = static_cast<const ScValueCell*>(pCell);
    if (pValueCell)
    {
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_float);
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, pValueCell->GetValue());
        rtl::OUString sValue(sBuffer.makeStringAndClear());
        rtl::OUString sText(pValueCell->GetStringData());
        if (sValue.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sValue);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
        if (sText.getLength())
        {
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
            sal_Bool bPrevCharWasSpace(sal_True);
            rExport.GetTextParagraphExport()->exportText(sText, bPrevCharWasSpace);
        }
    }
}

void ScChangeTrackingExportHelper::WriteStringEditCell(const rtl::OUString& rString)
{
    rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
    if (rString.getLength())
    {
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
        sal_Bool bPrevCharWasSpace(sal_True);
        rExport.GetTextParagraphExport()->exportText(rString, bPrevCharWasSpace);
    }
}

void ScChangeTrackingExportHelper::WriteStringCell(const ScBaseCell* pCell)
{
    const ScStringCell* pStringCell = static_cast<const ScStringCell*>(pCell);
    if (pStringCell)
    {
        String sString;
        pStringCell->GetString(sString);
        rtl::OUString sOUString(sString);
        WriteStringEditCell(sOUString);
    }
}

void ScChangeTrackingExportHelper::WriteEditCell(const ScBaseCell* pCell)
{
    const ScEditCell* pEditCell = static_cast<const ScEditCell*>(pCell);
    if (pEditCell)
    {
        String sString;
        pEditCell->GetString(sString);
        rtl::OUString sOUString(sString);
        WriteStringEditCell(sOUString);
    }
}

void ScChangeTrackingExportHelper::WriteFormulaCell(const ScBaseCell* pCell)
{
}

void ScChangeTrackingExportHelper::WriteCell(const ScBaseCell* pCell)
{
    if (pCell)
    {
        switch (pCell->GetCellType())
        {
            case CELLTYPE_NONE:
                WriteEmptyCell();
                break;
            case CELLTYPE_VALUE:
                WriteValueCell(pCell);
                break;
            case CELLTYPE_STRING:
                WriteStringCell(pCell);
                break;
            case CELLTYPE_EDIT:
                WriteEditCell(pCell);
                break;
            case CELLTYPE_FORMULA:
                WriteFormulaCell(pCell);
                break;
        }
    }
    else
        WriteEmptyCell();
}

void ScChangeTrackingExportHelper::WriteContentChange(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_cell_content_changing, sal_True, sal_True);
    WriteChangeInfo(pAction);
    {
        ScChangeActionContent* pPrevAction = static_cast<ScChangeActionContent*>(pAction)->GetPrevContent();
        if (pPrevAction)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pPrevAction->GetActionNumber()));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, sXML_previous, sal_True, sal_True);
        WriteCell(static_cast<ScChangeActionContent*>(pAction)->GetOldCell());
    }
}

void ScChangeTrackingExportHelper::WriteInsertion(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_insertion, sal_True, sal_True);
    WriteChangeInfo(pAction);
}

void ScChangeTrackingExportHelper::WriteDeletion(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_deletion, sal_True, sal_True);
    WriteChangeInfo(pAction);
    if (pAction->HasDependent())
    {
        ScChangeActionTable aDependings;
        pChangeTrack->GetDependents(pAction, aDependings);
        if (aDependings.Count())
        {
            ScChangeAction* pDependAction = aDependings.First();
            while (pDependAction != NULL)
            {
                pDependAction = aDependings.Next();
            }
        }
    }
}

void ScChangeTrackingExportHelper::WriteMovement(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_moving, sal_True, sal_True);
    WriteChangeInfo(pAction);
    if (pAction->HasDependent())
    {
        ScChangeActionTable aDependings;
        pChangeTrack->GetDependents(pAction, aDependings);
        if (aDependings.Count())
        {
            ScChangeAction* pDependAction = aDependings.First();
            while (pDependAction != NULL)
            {
                pDependAction = aDependings.Next();
            }
        }
    }
}

void ScChangeTrackingExportHelper::WorkWithChangeAction(ScChangeAction* pAction)
{
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pAction->GetActionNumber()));
    if (pAction->GetType() == SC_CAT_CONTENT)
        WriteContentChange(pAction);
    else if (pAction->IsInsertType())
        WriteInsertion(pAction);
    else if (pAction->IsDeleteType())
        WriteDeletion(pAction);
    else if (pAction->GetType() == SC_CAT_MOVE)
        WriteMovement(pAction);
    else
        DBG_ERROR("not a writeable type");
    rExport.CheckAttrList();
}

void ScChangeTrackingExportHelper::StartChangeActionList()
{
    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    rExport.GetDocHandler()->startElement( sTrackedChanges, rExport.GetXAttrList());
    rExport.ClearAttrList();
}

void ScChangeTrackingExportHelper::EndChangeActionList()
{
    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    rExport.GetDocHandler()->endElement(sTrackedChanges);
}

void ScChangeTrackingExportHelper::CollectAndWriteChanges()
{
    if (pChangeTrack)
    {
        sal_uInt32 nCount (pChangeTrack->GetActionMax());
        if (nCount)
        {
            StartChangeActionList();
            ScChangeAction* pAction = pChangeTrack->GetFirst();
            WorkWithChangeAction(pAction);
            ScChangeAction* pLastAction = pChangeTrack->GetLast();
            while (pAction != pLastAction)
            {
                pAction = pAction->GetNext();
                WorkWithChangeAction(pAction);
            }
            EndChangeActionList();
        }
    }
}
