/*************************************************************************
 *
 *  $RCSfile: HtmlReader.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2001-02-27 15:31:17 $
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
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
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
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
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
#ifndef TOOLS_INETMIME_HXX
#include <tools/inetmime.hxx>
#endif
#ifndef _INETTYPE_HXX
#include <svtools/inettype.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
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
#define DBAUI_HTML_FONTSIZES 7      // wie Export, HTML-Options
#define HTML_META_NONE          0
#define HTML_META_AUTHOR        1
#define HTML_META_DESCRIPTION   2
#define HTML_META_KEYWORDS      3
#define HTML_META_REFRESH       4
#define HTML_META_CLASSIFICATION 5
#define HTML_META_CREATED       6
#define HTML_META_CHANGEDBY     7
#define HTML_META_CHANGED       8
#define HTML_META_GENERATOR     9
#define HTML_META_SDFOOTNOTE    10
#define HTML_META_SDENDNOTE     11
#define HTML_META_CONTENT_TYPE  12

static HTMLOptionEnum __READONLY_DATA aHTMLMetaNameTable[] =
{
    { sHTML_META_author,        HTML_META_AUTHOR        },
    { sHTML_META_changed,       HTML_META_CHANGED       },
    { sHTML_META_changedby,     HTML_META_CHANGEDBY     },
    { sHTML_META_classification,HTML_META_CLASSIFICATION},
    { sHTML_META_content_type,  HTML_META_CONTENT_TYPE  },
    { sHTML_META_created,       HTML_META_CREATED       },
    { sHTML_META_description,   HTML_META_DESCRIPTION   },
    { sHTML_META_keywords,      HTML_META_KEYWORDS      },
    { sHTML_META_generator,     HTML_META_GENERATOR     },
    { sHTML_META_refresh,       HTML_META_REFRESH       },
    { sHTML_META_sdendnote,     HTML_META_SDENDNOTE     },
    { sHTML_META_sdfootnote,    HTML_META_SDFOOTNOTE    },
    { 0,                        0                       }
};

// ==========================================================================
DBG_NAME(OHTMLReader);
// ==========================================================================
// OHTMLReader
// ==========================================================================
OHTMLReader::OHTMLReader(SvStream& rIn,const Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
                        const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : HTMLParser(rIn)
    ,ODatabaseExport(_rxConnection,_rxNumberF,_rM)
    ,m_nTableCount(0)
    ,m_nColumnWidth(87)
    ,m_bMetaOptions(sal_False)
{
    DBG_CTOR(OHTMLReader,NULL);
    // If the system encoding is ANSI, this encoding is used as default
    // source encoding. Otherwise ISO-8859-1 will be used, because this
    // is the real default encoding.
    SetSrcEncoding( RTL_TEXTENCODING_MS_1252 == gsl_getSystemTextEncoding()
                        ? RTL_TEXTENCODING_MS_1252
                        : RTL_TEXTENCODING_ISO_8859_1 );
}
// ---------------------------------------------------------------------------
OHTMLReader::OHTMLReader(SvStream& rIn,
                         sal_Int32 nRows,
                         const ::std::vector<sal_Int32> &_rColumnPositions,
                         const Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    : HTMLParser(rIn)
    ,ODatabaseExport(nRows,_rColumnPositions,_rxNumberF,_rM)
    ,m_nTableCount(0)
    ,m_nColumnWidth(87)
    ,m_bMetaOptions(sal_False)
{
    DBG_CTOR(OHTMLReader,NULL);
    // If the system encoding is ANSI, this encoding is used as default
    // source encoding. Otherwise ISO-8859-1 will be used, because this
    // is the real default encoding.
    SetSrcEncoding( RTL_TEXTENCODING_MS_1252 == gsl_getSystemTextEncoding()
                        ? RTL_TEXTENCODING_MS_1252
                        : RTL_TEXTENCODING_ISO_8859_1 );
}
// ---------------------------------------------------------------------------
OHTMLReader::~OHTMLReader()
{
    DBG_DTOR(OHTMLReader,NULL);
}
// ---------------------------------------------------------------------------
SvParserState OHTMLReader::CallParser()
{
    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();
    return HTMLParser::CallParser();
}
// -----------------------------------------------------------------------------
rtl_TextEncoding OHTMLReader::GetEncodingByMIME( const String& rMime )
{
    ByteString sType;
    ByteString sSubType;
    INetContentTypeParameterList aParameters;
    ByteString sMime( rMime, RTL_TEXTENCODING_ASCII_US );
    if (INetContentTypes::parse(sMime, sType, sSubType, &aParameters))
    {
        const INetContentTypeParameter * pCharset
            = aParameters.find("charset");
        if (pCharset != 0)
        {
            ByteString sValue( pCharset->m_sValue, RTL_TEXTENCODING_ASCII_US );
            return rtl_getTextEncodingFromMimeCharset( sValue.GetBuffer() );
        }
    }
    return RTL_TEXTENCODING_DONTKNOW;
}

// ---------------------------------------------------------------------------
void OHTMLReader::NextToken( int nToken )
{
    if(m_bError || !m_nRows) // falls Fehler oder keine Rows mehr zur "Uberpr"ufung dann gleich zur"uck
        return;
    if(!m_bMetaOptions)
        setTextEncoding();

    if(m_xConnection.is())    // gibt an welcher CTOR gerufen wurde und damit, ob eine Tabelle erstellt werden soll
    {
        switch(nToken)
        {
            case HTML_TABLE_ON:
                ++m_nTableCount;
                {   // es kann auch TD oder TH sein, wenn es vorher kein TABLE gab
                    const HTMLOptions* pOptions = GetOptions();
                    sal_Int16 nArrLen = pOptions->Count();
                    for ( sal_Int16 i = 0; i < nArrLen; i++ )
                    {
                        const HTMLOption* pOption = (*pOptions)[i];
                        switch( pOption->GetToken() )
                        {
                            case HTML_O_WIDTH:
                            {   // Prozent: von Dokumentbreite bzw. aeusserer Zelle
                                m_nColumnWidth = GetWidthPixel( pOption );
                            }
                            break;
                        }
                    }
                }
            case HTML_THEAD_ON:
            case HTML_TBODY_ON:
                if(!m_xTable.is()) // erste Zeile als Header verwenden
                    m_bError = !CreateTable(nToken);
                break;
            case HTML_TABLE_OFF:
                if(!--m_nTableCount)
                {
                    m_xTable = NULL;
                }
                break;
            case HTML_TABLEROW_ON:
                m_xResultSetUpdate->moveToInsertRow(); // sonst neue Zeile anh"angen
                break;
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if(m_bInTbl) // wichtig, da wir sonst auch die Namen der Fonts bekommen
                    m_sTextToken += aToken;
                break;
            case HTML_TABLEDATA_ON:
                m_bInTbl = TRUE;
                break;
            case HTML_TABLEDATA_OFF:
                {
                    insertValueIntoColumn();
                    m_nColumnPos++;
                    m_sTextToken.Erase();
                    m_bInTbl = sal_False;
                }
                break;
            case HTML_TABLEROW_OFF:
                try
                {
                    m_nRowCount++;
                    if (m_bIsAutoIncrement) // if bSetAutoIncrement then I have to set the autoincrement
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
            case HTML_THEAD_ON:
            case HTML_TBODY_ON:
                // Der Spalten Kopf z"ahlt nicht mit
                if(m_bHead)
                {
                    do
                    {}
                    while(GetNextToken() != HTML_TABLEROW_OFF);
                    m_bHead = sal_False;
                }
                break;
            case HTML_TABLEDATA_ON:
            case HTML_TABLEHEADER_ON:
                m_bInTbl = TRUE;
                break;
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if(m_bInTbl)
                    m_sTextToken += aToken;
                break;
            case HTML_TABLEDATA_OFF:
                if(m_sTextToken.Len())
                {
                    if(m_vColumns[m_nColumnPos] != CONTAINER_ENTRY_NOTFOUND)
                    {
                        m_vFormatKey[m_vColumns[m_nColumnPos]] = CheckString(m_sTextToken,m_vFormatKey[m_vColumns[m_nColumnPos]]);
                        m_vColumnSize[m_vColumns[m_nColumnPos]] = ::std::max<sal_Int32>((sal_Int32)m_vColumnSize[m_vColumns[m_nColumnPos]],(sal_Int32)m_sTextToken.Len());
                    }
                    m_sTextToken.Erase();
                }
                m_nColumnPos++;
                m_bInTbl = sal_False;
                break;
            case HTML_TABLEROW_OFF:
                if(m_sTextToken.Len())
                {
                    if(m_vColumns[m_nColumnPos] != CONTAINER_ENTRY_NOTFOUND)
                    {
                        m_vFormatKey[m_vColumns[m_nColumnPos]] = CheckString(m_sTextToken,m_vFormatKey[m_vColumns[m_nColumnPos]]);
                        m_vColumnSize[m_vColumns[m_nColumnPos]] = ::std::max<sal_Int32>((sal_Int32)m_vColumnSize[m_vColumns[m_nColumnPos]],(sal_Int32)m_sTextToken.Len());
                    }
                    m_sTextToken.Erase();
                }
                m_nColumnPos = 0;
                m_nRows--;
                break;
        }
    }
}
//---------------------------------------------------------------------------------
void OHTMLReader::TableDataOn(SvxCellHorJustify& eVal,String *pValue,int nToken)
{
    sal_Bool bHorJustifyCenterTH = (nToken == HTML_TABLEHEADER_ON);
    const HTMLOptions* pOptions = GetOptions();
    sal_Int16 nArrLen = pOptions->Count();
    for ( sal_Int16 i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
            case HTML_O_ALIGN:
            {
                bHorJustifyCenterTH = sal_False;
                const String& rOptVal = pOption->GetString();
                if (rOptVal.EqualsIgnoreCaseAscii( sHTML_AL_right ))
                    eVal = SVX_HOR_JUSTIFY_RIGHT;
                else if (rOptVal.EqualsIgnoreCaseAscii( sHTML_AL_center ))
                    eVal = SVX_HOR_JUSTIFY_CENTER;
                else if (rOptVal.EqualsIgnoreCaseAscii( sHTML_AL_left ))
                    eVal = SVX_HOR_JUSTIFY_LEFT;
                else
                    eVal = SVX_HOR_JUSTIFY_STANDARD;
            }
            break;
            case HTML_O_SDVAL:
            {
                pValue = new String( pOption->GetString() );
            }
            break;
            case HTML_O_SDNUM:
            {
                pValue = new String( pOption->GetString() );
            }
            break;
            case HTML_O_BGCOLOR:
                //m_aFont.SetFillColor(Color((sal_Int32)pOption->GetString()));
            break;
            case HTML_O_WIDTH:
            {
                m_nWidth = GetWidthPixel( pOption );
            }
            break;
        }
    }
}

//---------------------------------------------------------------------------------
void OHTMLReader::TableFontOn(FontDescriptor& _rFont,sal_Int32 &_rTextColor)
{
    const HTMLOptions* pOptions = GetOptions();
    sal_Int16 nArrLen = pOptions->Count();
    for ( sal_Int16 i = 0; i < nArrLen; i++ )
    {
        const HTMLOption* pOption = (*pOptions)[i];
        switch( pOption->GetToken() )
        {
        case HTML_O_COLOR:
            {
                Color aColor;
                pOption->GetColor( aColor );
                _rTextColor = aColor.GetRGBColor();
            }
            break;
        case HTML_O_FACE :
            {
                const String& rFace = pOption->GetString();
                String aFontName;
                xub_StrLen nPos = 0;
                while( nPos != STRING_NOTFOUND )
                {   // Fontliste, VCL: Semikolon als Separator, HTML: Komma
                    String aFName = rFace.GetToken( 0, ',', nPos );
                    aFName.EraseTrailingChars().EraseLeadingChars();
                    if( aFontName.Len() )
                        aFontName += ';';
                    aFontName += aFName;
                }
                if ( aFontName.Len() )
                    _rFont.Name = ::rtl::OUString(aFontName);
            }
            break;
        case HTML_O_SIZE :
            {
                sal_Int16 nSize = (sal_Int16) pOption->GetNumber();
                if ( nSize == 0 )
                    nSize = 1;
                else if ( nSize > DBAUI_HTML_FONTSIZES )
                    nSize = DBAUI_HTML_FONTSIZES;

                _rFont.Height = nSize;
            }
            break;
        }
    }
}
// ---------------------------------------------------------------------------
sal_Int16 OHTMLReader::GetWidthPixel( const HTMLOption* pOption )
{
    const String& rOptVal = pOption->GetString();
    if ( rOptVal.Search('%') != STRING_NOTFOUND )
    {   // Prozent
        DBG_ASSERT( m_nColumnWidth, "WIDTH Option: m_nColumnWidth==0 und Width%" );
        return (sal_Int16)((pOption->GetNumber() * m_nColumnWidth) / 100);
    }
    else
    {
        if ( rOptVal.Search('*') != STRING_NOTFOUND )
        {   // relativ zu was?!?
//2do: ColArray aller relativen Werte sammeln und dann MakeCol
            return 0;
        }
        else
            return (sal_Int16)pOption->GetNumber(); // Pixel
    }
}
// ---------------------------------------------------------------------------
sal_Bool OHTMLReader::CreateTable(int nToken)
{
    String aTempName(ModuleRes(STR_TBL_TITLE));
    aTempName = aTempName.GetToken(0,' ');
    aTempName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTempName )));

    int nTmpToken2 = nToken;
    sal_Bool bCaption = sal_False;
    sal_Bool bTableHeader = sal_False;
    String aColumnName;
    SvxCellHorJustify eVal;
    String *pValue=NULL;
    sal_Int16 nWidth = 0;
    sal_Int16 nHeight = 0;

    String aTableName;
    FontDescriptor aFont;
    sal_Int32 nTextColor = 0;
    do
    {
        switch(nTmpToken2)
        {
            case HTML_TEXTTOKEN:
            case HTML_SINGLECHAR:
                if(bTableHeader)
                    aColumnName += aToken;
                if(bCaption)
                    aTableName += aToken;
                break;
            case HTML_TABLEDATA_ON:
            case HTML_TABLEHEADER_ON:
                if(pValue)
                {       // HTML_TABLEHEADER_OFF oder HTML_TABLEDATA_OFF fehlte
                    aColumnName.EraseLeadingChars();
                    aColumnName.EraseTrailingChars();
                    if (!aColumnName.Len())
                        aColumnName = String(ModuleRes(STR_COLUMN_NAME));

                    CreateDefaultColumn(aColumnName);
//                  SbaColumn* pColumn = CreateColumn(aColumnName,aTableName,pDestList);
//                  pColumn->Put(SvxHorJustifyItem( eVal, SBA_ATTR_ALIGN_HOR_JUSTIFY) );
//
//                  pDestList->InsertAt(aColumnName,pColumn);
                    aColumnName.Erase();

                    DELETEZ(pValue);
                    eVal = SVX_HOR_JUSTIFY_STANDARD;
                }
                TableDataOn(eVal,pValue,nTmpToken2);
                bTableHeader = TRUE;
                break;
            case HTML_TABLEDATA_OFF:
            case HTML_TABLEHEADER_OFF:
                {
                    aColumnName.EraseLeadingChars();
                    aColumnName.EraseTrailingChars();
                    if (!aColumnName.Len())
                        aColumnName = String(ModuleRes(STR_COLUMN_NAME));

                    CreateDefaultColumn(aColumnName);
                    aColumnName.Erase();

                    DELETEZ(pValue);
                    eVal = SVX_HOR_JUSTIFY_STANDARD;
                    bTableHeader = sal_False;
                }
                break;

            case HTML_TITLE_ON:
            case HTML_CAPTION_ON:
                bCaption = TRUE;
                break;
            case HTML_TITLE_OFF:
            case HTML_CAPTION_OFF:
                aTableName.EraseLeadingChars();
                aTableName.EraseTrailingChars();
                if(aTableName.Len())
                    aTableName = String(::dbtools::createUniqueName(m_xTables,::rtl::OUString(aTableName)));
                else
                    aTableName = aTempName;
                bCaption = sal_False;
                break;
            case HTML_FONT_ON:
                TableFontOn(aFont,nTextColor);
                break;
            case HTML_BOLD_ON:
                aFont.Weight = ::com::sun::star::awt::FontWeight::BOLD;
                break;
            case HTML_ITALIC_ON:
                aFont.Slant = ::com::sun::star::awt::FontSlant_ITALIC;
                break;
            case HTML_UNDERLINE_ON:
                aFont.Underline = ::com::sun::star::awt::FontUnderline::SINGLE;
                break;
            case HTML_STRIKE_ON:
                aFont.Strikeout = ::com::sun::star::awt::FontStrikeout::SINGLE;
                break;
        }
    }
    while((nTmpToken2 = GetNextToken()) != HTML_TABLEROW_OFF);

    if(aColumnName.Len())
        CreateDefaultColumn(aColumnName);

    m_bInTbl = sal_False;

    sal_Bool bError = sal_False;
    OCopyTableWizard aWizard(NULL,aTableName,m_aDestColumns,m_vDestVector,m_xConnection,m_xFormatter,m_xFactory);

    OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, sal_False,OCopyTableWizard::WIZARD_DEF_DATA);
    OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
    OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
    OWizHTMLExtend*     pPage4 = new OWizHTMLExtend(&aWizard,rInput);

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
                        m_xTable->setPropertyValue(PROPERTY_TEXTCOLOR,makeAny(nTextColor));
                    }
                    m_bIsAutoIncrement  = aWizard.SetAutoincrement();
                    m_vColumns          = aWizard.GetColumnPositions();
                    m_vColumnTypes      = aWizard.GetColumnTypes();
//                  m_xTable = aWizard.GetTableDef();
//                  void *pTemp = (void *)&m_vColumns;
//                  map<String,String,SbaStringCompare> aTmp = aWizard.GetNameMapping();
//                  void* pTemp2 = (void *)&aTmp;
//                  m_bError = !SbaExplorerExchObj::CheckColumnMerge(xSourceDef->GetColumns(),m_xTable->GetOriginalColumns(),pTemp,pTemp2);
                }
                break;
            default:
                bError = TRUE; // there is no error but I have nothing more to do
        }
    }
    else
        bError = TRUE;

    if(!bError)
        bError = !createRowSet();

    return !bError && m_xTable.is();
}
// -----------------------------------------------------------------------------
void OHTMLReader::setTextEncoding()
{
    m_bMetaOptions = sal_True;
    USHORT nContentOption = HTML_O_CONTENT;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;
    String aName, aContent;
    USHORT nAction = HTML_META_NONE;
    BOOL bHTTPEquiv = FALSE, bChanged = FALSE;
    const HTMLOptions *pOptions = GetOptions(&nContentOption);
    for( USHORT i = pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*pOptions)[ --i ];
        switch( pOption->GetToken() )
        {
        case HTML_O_HTTPEQUIV:
            aName = pOption->GetString();
            pOption->GetEnum( nAction, aHTMLMetaNameTable );
            bHTTPEquiv = TRUE;
            break;
        case HTML_O_CONTENT:
            aContent = pOption->GetString();
            break;
        }
    }
    if( bHTTPEquiv || HTML_META_DESCRIPTION!=nAction )
    {
        // wenn's keine Description ist CRs und LFs aus dem CONTENT entfernen
        aContent.EraseAllChars( _CR );
        aContent.EraseAllChars( _LF );
    }
    else
    {
        // fuer die Beschreibung die Zeilen-Umbrueche entsprechen wandeln
        aContent.ConvertLineEnd();
    }
    switch( nAction )
    {
        case HTML_META_CONTENT_TYPE:
            if( aContent.Len() )
                SetSrcEncoding(GetEncodingByMIME( aContent ));
            break;
    }
}
// -----------------------------------------------------------------------------

