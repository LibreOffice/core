/*************************************************************************
 *
 *  $RCSfile: RtfReader.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-28 10:11:32 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTWEIGHT_HPP_
#include <com/sun/star/awt/FontWeight.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#include "WExtendPages.hxx"
#endif
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#include "WNameMatch.hxx"
#endif
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#include "WColumnSelect.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;

#define CONTAINER_ENTRY_NOTFOUND    ((ULONG)0xFFFFFFFF)
DBG_NAME(ORTFReader);
// ==========================================================================
// ORTFReader
// ==========================================================================
ORTFReader::ORTFReader( SvStream& rIn,
                        const Reference< XConnection >& _rxConnection,
                        const Reference< XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : SvRTFParser(rIn)
    ,ODatabaseExport(_rxConnection,_rxNumberF,_rM)
{
    DBG_CTOR(ORTFReader,NULL);
}
// ---------------------------------------------------------------------------
ORTFReader::ORTFReader(SvStream& rIn,
                       sal_Int32 nRows,
                       const ::std::vector<sal_Int32> &_rColumnPositions,
                       const Reference< XNumberFormatter >& _rxNumberF,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
   :SvRTFParser(rIn)
   ,ODatabaseExport(nRows,_rColumnPositions,_rxNumberF,_rM)
{
    DBG_CTOR(ORTFReader,NULL);
}
// ---------------------------------------------------------------------------
ORTFReader::~ORTFReader()
{
    DBG_DTOR(ORTFReader,NULL);
}
// ---------------------------------------------------------------------------
SvParserState ORTFReader::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    return SvRTFParser::CallParser();
}


// ---------------------------------------------------------------------------
void ORTFReader::NextToken( int nToken )
{
    if(m_bError || !m_nRows) // falls Fehler oder keine Rows mehr zur "Uberpr"ufung dann gleich zur"uck
        return;

    if(m_xConnection.is())    // gibt an welcher CTOR gerufen wurde und damit, ob eine Tabelle erstellt werden soll
    {
        switch(nToken)
        {
            case RTF_COLORTBL:
                {

                    int nTmpToken2 = GetNextToken();
                    sal_Bool bNext = TRUE;
                    do
                    {
                        Color aColor;
                        do
                        {
                            switch(nTmpToken2)
                            {
                                case RTF_RED:   aColor.SetRed((sal_uInt8)nTokenValue); break;
                                case RTF_BLUE:  aColor.SetBlue((sal_uInt8)nTokenValue); break;
                                case RTF_GREEN: aColor.SetGreen((sal_uInt8)nTokenValue); break;
                                default:
                                    bNext = sal_False;
                            }
                            nTmpToken2 = GetNextToken();
                        }
                        while(aToken.GetChar(0) != ';');
                        m_vecColor.push_back(aColor.GetRGBColor());
                        nTmpToken2 = GetNextToken();
                    }
                    while(nTmpToken2 == RTF_RED);
                    SkipToken();
                }
                break;

            case RTF_DEFLANG:
            case RTF_LANG: // Sprache abfragen
                m_nDefToken = (rtl_TextEncoding)nTokenValue;
                break;
            case RTF_TROWD:
                if(!m_xTable.is()) // erste Zeile als Header verwenden
                    m_bError = !CreateTable(nToken);
                else
                    m_xResultSetUpdate->moveToInsertRow(); // sonst neue Zeile anh"angen
                break;
            case RTF_INTBL:
                if(m_bInTbl)
                    m_sTextToken.Erase();
                m_bInTbl = TRUE; // jetzt befinden wir uns in einer Tabellenbeschreibung
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl) // wichtig, da wir sonst auch die Namen der Fonts bekommen
                    m_sTextToken += aToken;
                break;
            case RTF_CELL:
                {
                    insertValueIntoColumn();
                    m_nColumnPos++;
                    m_sTextToken.Erase();
                }
                break;
            case RTF_ROW:
                // es kann vorkommen, daß die letzte Celle nicht mit \cell abgeschlossen ist
                try
                {
                    insertValueIntoColumn();
                    m_nRowCount++;
                    if(m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
                        m_xRowUpdate->updateInt(1,m_nRowCount);
                    m_xResultSetUpdate->insertRow();
                }
                catch(SQLException& e)
                //////////////////////////////////////////////////////////////////////
                // UpdateFehlerbehandlung
                {
                    if(!m_bDontAskAgain)
                    {
                        String aMsg(e.Message);
                        aMsg += '\n';
                        aMsg += String(ModuleRes(STR_QRY_CONTINUE));
                        OSQLMessageBox aBox(NULL, String(ModuleRes(STR_STAT_WARNING)),
                            aMsg, WB_YES_NO | WB_DEF_NO, OSQLMessageBox::Warning);

                        if (aBox.Execute() == RET_YES)
                            m_bDontAskAgain = TRUE;
                        else
                            m_bError = TRUE;
                    }
                }
                m_nColumnPos = 0;
                break;
        }
    }
    else // Zweig nur f"ur Typpr"ufung g"ultig
    {
        switch(nToken)
        {
            case RTF_TROWD:
                // Der Spalten Kopf z"ahlt nicht mit
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != RTF_ROW);
                    m_bHead = sal_False;
                }
                break;
            case RTF_INTBL:
                m_bInTbl = TRUE;
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl)
                    m_sTextToken += aToken;
                break;
            case RTF_CELL:
                if(m_sTextToken.Len())
                {
                    if(m_vColumns[m_nColumnPos] != CONTAINER_ENTRY_NOTFOUND)
                    {
                        m_vFormatKey[m_vColumns[m_nColumnPos]] = CheckString(m_sTextToken,m_vFormatKey[m_vColumns[m_nColumnPos]]);
                        m_vColumnSize[m_vColumns[m_nColumnPos]] = ::std::max<sal_Int32>(m_vColumnSize[m_vColumns[m_nColumnPos]],(sal_Int32)m_sTextToken.Len());
                    }

                    m_sTextToken.Erase();
                }
                m_nColumnPos++;
                break;
            case RTF_ROW:
                if(m_sTextToken.Len())
                {
                    if(m_vColumns[m_nColumnPos] != CONTAINER_ENTRY_NOTFOUND)
                    {
                        m_vFormatKey[m_vColumns[m_nColumnPos]] = CheckString(m_sTextToken,m_vFormatKey[m_vColumns[m_nColumnPos]]);
                        m_vColumnSize[m_vColumns[m_nColumnPos]] = ::std::max<sal_Int32>(m_vColumnSize[m_vColumns[m_nColumnPos]],(sal_Int32)m_sTextToken.Len());
                    }
                    m_sTextToken.Erase();
                }
                m_nColumnPos = 0;
                m_nRows--;
                break;
        }
    }
}
// ---------------------------------------------------------------------------
sal_Bool ORTFReader::CreateTable(int nToken)
{
    String aTableName(ModuleRes(STR_TBL_TITLE));
    aTableName = aTableName.GetToken(0,' ');
    aTableName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTableName)));

    int nTmpToken2 = nToken;
    String aColumnName;
    sal_Bool bColorSet=sal_False;
    sal_Bool bBoldSet=sal_False;
    sal_Bool bUnderlineSet=sal_False;
    sal_Bool bStrikeoutSet=sal_False;
    sal_Bool bItalicSet=sal_False;

    FontDescriptor aFont;
    do
    {
        switch(nTmpToken2)
        {
            case RTF_UNKNOWNCONTROL:
            case RTF_UNKNOWNDATA:
                m_bInTbl = sal_False;
                aColumnName.Erase();
                break;
            case RTF_INTBL:
                if(m_bInTbl)
                    aColumnName.Erase();

                m_bInTbl = TRUE;
                break;
            case RTF_TEXTTOKEN:
            case RTF_SINGLECHAR:
                if(m_bInTbl)
                    aColumnName += aToken;
                break;
            case RTF_CELL:
                {
                    aColumnName.EraseLeadingChars();
                    aColumnName.EraseTrailingChars();
                    if (!aColumnName.Len())
                        aColumnName = String(ModuleRes(STR_COLUMN_NAME));

                    CreateDefaultColumn(aColumnName);
                    aColumnName.Erase();
                }
                break;
            case RTF_CF:
                //  if(nTokenValue < m_vecColor.size())
                    //  m_xTable->setPropertyValue(PROPERTY_TEXTCOLOR,makeAny(m_vecColor[nTokenValue]));
                break;
            case RTF_B:
                aFont.Weight = ::com::sun::star::awt::FontWeight::BOLD;
                break;
            case RTF_I:
                aFont.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
                break;
            case RTF_UL:
                aFont.Underline = ::com::sun::star::awt::FontUnderline::SINGLE;
                break;
            case RTF_STRIKE:
                aFont.Strikeout = ::com::sun::star::awt::FontStrikeout::SINGLE;
                break;
        }
    }
    while((nTmpToken2 = GetNextToken()) != RTF_ROW);




    if(aColumnName.Len())
        CreateDefaultColumn(aColumnName);

    m_bInTbl = sal_False;

    sal_Bool bError = sal_False;

    OCopyTableWizard aWizard(NULL,aTableName,m_aDestColumns,m_vDestVector,m_xConnection,m_xFormatter,m_xFactory);

    OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, sal_False,OCopyTableWizard::WIZARD_DEF_DATA);
    OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
    OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
    OWizRTFExtend*      pPage4 = new OWizRTFExtend(&aWizard,rInput);

    aWizard.AddWizardPage(pPage1);
    aWizard.AddWizardPage(pPage2);
    aWizard.AddWizardPage(pPage3);
    aWizard.AddWizardPage(pPage4);

    aWizard.ActivatePage();

    if (aWizard.Execute())
    {
        switch(aWizard.GetCreateStyle())
        {
            case OCopyTableWizard::WIZARD_DEF_DATA:
            case OCopyTableWizard::WIZARD_APPEND_DATA:
                {
                    m_xTable = aWizard.createTable();
                    bError = !m_xTable.is();
                    if(m_xTable.is())
                    {
                        m_xTable->setPropertyValue(PROPERTY_FONT,makeAny(aFont));
                        if(m_vecColor.size())
                            m_xTable->setPropertyValue(PROPERTY_TEXTCOLOR,makeAny(m_vecColor[0]));

                        m_bIsAutoIncrement  = aWizard.SetAutoincrement();
                        m_vColumns          = aWizard.GetColumnPositions();
                        m_vColumnTypes      = aWizard.GetColumnTypes();
                    }
                }
                break;
            default:
                bError = TRUE; // there is no error but I have nothing more to do
        }
    }
    else
        bError = TRUE;

    if(!bError)
        bError = createRowSet();

    return !bError && m_xTable.is();
}
// -----------------------------------------------------------------------------



