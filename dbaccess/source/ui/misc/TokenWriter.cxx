/*************************************************************************
 *
 *  $RCSfile: TokenWriter.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-03 14:15:03 $
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
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
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
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _RTFKEYWD_HXX
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _HTMLOUT_HXX
#include <svtools/htmlout.hxx>
#endif
#ifndef _SFXDOCINF_HXX
#include <sfx2/docinf.hxx>
#endif
#ifndef _FRMHTMLW_HXX
#include <sfx2/frmhtmlw.hxx>
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::awt;


#if defined(MAC)
const char __FAR_DATA ODatabaseImportExport::sNewLine = '\015';
#elif defined(UNX)
const char __FAR_DATA ODatabaseImportExport::sNewLine = '\012';
#else
const char __FAR_DATA ODatabaseImportExport::sNewLine[] = "\015\012";
#endif

const static char __FAR_DATA sMyBegComment[]    = "<!-- ";
const static char __FAR_DATA sMyEndComment[]    = " -->";
const static char __FAR_DATA sFontFamily[]      = "font-family: ";
const static char __FAR_DATA sFontSize[]        = "font-size: ";

#define SBA_FORMAT_SELECTION_COUNT  4

DBG_NAME(ODatabaseImportExport);
//======================================================================
ODatabaseImportExport::ODatabaseImportExport(const Sequence< PropertyValue >& _aSeq,
                                             const Reference< XMultiServiceFactory >& _rM,
                                             const Reference< XNumberFormatter >& _rxNumberF,
                                             const String& rExchange)
    :m_pReader(NULL)
    ,m_pRowMarker(NULL)
    ,m_xFormatter(_rxNumberF)
    ,m_xFactory(_rM)
    ,m_nCommandType(CommandType::TABLE)
    ,m_bDisposeConnection(sal_False)
{

    DBG_CTOR(ODatabaseImportExport,NULL);
    osl_incrementInterlockedCount( &m_refCount );
    // get the information we need
    const PropertyValue* pBegin = _aSeq.getConstArray();
    const PropertyValue* pEnd   = pBegin + _aSeq.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(pBegin->Name == PROPERTY_DATASOURCENAME)
            pBegin->Value >>= m_sDataSourceName;
        else if(pBegin->Name == PROPERTY_COMMANDTYPE)
            pBegin->Value >>= m_nCommandType;
        else if(pBegin->Name == PROPERTY_COMMAND)
            pBegin->Value >>= m_sName;
        else if(pBegin->Name == PROPERTY_ACTIVECONNECTION)
            pBegin->Value >>= m_xConnection;
    }

    xub_StrLen nCount = rExchange.GetTokenCount(char(11));
    if( nCount > SBA_FORMAT_SELECTION_COUNT && rExchange.GetToken(4).Len())
    {
        m_pRowMarker = new sal_Int32[nCount-SBA_FORMAT_SELECTION_COUNT];
        for(xub_StrLen i=SBA_FORMAT_SELECTION_COUNT;i<nCount;++i)
            m_pRowMarker[i-SBA_FORMAT_SELECTION_COUNT] = rExchange.GetToken(i,char(11)).ToInt32();
    }

    OSL_ENSURE(m_sDataSourceName.getLength(),"There must be a datsource name!");
    osl_decrementInterlockedCount( &m_refCount );
}
//-------------------------------------------------------------------
ODatabaseImportExport::~ODatabaseImportExport()
{
    acquire();

    disposing();

    if(m_pReader)
        ((SvRefBase*)m_pReader)->ReleaseRef();
    delete m_pRowMarker;
    DBG_DTOR(ODatabaseImportExport,NULL);
}
// -----------------------------------------------------------------------------
void ODatabaseImportExport::disposing()
{
    // remove me as listener
    Reference< XComponent >  xComponent(m_xConnection, UNO_QUERY);
    if (xComponent.is())
    {
        Reference< XEventListener> xEvt((::cppu::OWeakObject*)this,UNO_QUERY);
        xComponent->removeEventListener(xEvt);
    }
    if(m_bDisposeConnection)
        ::comphelper::disposeComponent(m_xConnection);

    ::comphelper::disposeComponent(m_xRow);

    m_xObject               = NULL;
    m_xConnection           = NULL;
    m_xResultSetMetaData    = NULL;
    m_xResultSet            = NULL;
    m_xRow                  = NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODatabaseImportExport::disposing( const EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XConnection> xCon(Source.Source,UNO_QUERY);
    if(m_xConnection.is() && m_xConnection == xCon)
    {
        disposing();
        initialize();
        m_bDisposeConnection = m_xConnection.is();
    }
}
// -----------------------------------------------------------------------------
void ODatabaseImportExport::initialize()
{
    if(!m_xConnection.is())
    {   // we need a connection
        Reference<XNameAccess> xDatabaseContext = Reference< XNameAccess >(m_xFactory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
        Reference< XEventListener> xEvt((::cppu::OWeakObject*)this,UNO_QUERY);
        SQLExceptionInfo aInfo = ::dbaui::createConnection(m_sDataSourceName,xDatabaseContext,m_xFactory,xEvt,m_xConnection);
        if(aInfo.isValid() && aInfo.getType() == SQLExceptionInfo::SQL_EXCEPTION)
            throw *static_cast<const SQLException*>(aInfo);
    }
    else
    {
        Reference< XEventListener> xEvt((::cppu::OWeakObject*)this,UNO_QUERY);
        Reference< XComponent >  xComponent(m_xConnection, UNO_QUERY);
        if (xComponent.is() && xEvt.is())
            xComponent->addEventListener(xEvt);
    }

    Reference<XNameAccess> xNameAccess;
    switch(m_nCommandType)
    {
        case CommandType::TABLE:
            {
                // only for tables
                Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY);
                if(xSup.is())
                    xNameAccess = xSup->getTables();
            }
            break;
        case CommandType::QUERY:
            {
                Reference<XQueriesSupplier> xSup(m_xConnection,UNO_QUERY);
                if(xSup.is())
                    xNameAccess = xSup->getQueries();
            }
            break;
    }
    if(xNameAccess.is() && xNameAccess->hasByName(m_sName))
    {
        Reference<XPropertySet> xSourceObject;
        xNameAccess->getByName(m_sName) >>= m_xObject;
    }

    if(m_xObject.is())
    {
        try
        {
            if(m_xObject->getPropertySetInfo()->hasPropertyByName(PROPERTY_FONT))
                m_xObject->getPropertyValue(PROPERTY_FONT) >>= m_aFont;

            m_xResultSet = Reference< XResultSet >(m_xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.RowSet")),UNO_QUERY);
            Reference<XPropertySet > xProp(m_xResultSet,UNO_QUERY);
            if(xProp.is())
            {
                xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(m_xConnection));
                xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(m_nCommandType));
                xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(m_sName));
                Reference<XRowSet> xRowSet(xProp,UNO_QUERY);
                xRowSet->execute();
                m_xRow = Reference<XRow>(xRowSet,UNO_QUERY);
                m_xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(m_xRow,UNO_QUERY)->getMetaData();
            }
            else
                OSL_ENSURE(sal_False, "ODatabaseImportExport::initialize: could not instantiate a rowset!");
        }
        catch(Exception& )
        {
            m_xRow = NULL;
            m_xResultSetMetaData = NULL;
            ::comphelper::disposeComponent(m_xResultSet);
            throw;
        }
    }
    if(!m_aFont.Name.getLength())
        m_aFont.Name = Application::GetSettings().GetStyleSettings().GetAppFont().GetName();
}
//======================================================================
BOOL ORTFImportExport::Write()
{
    (*m_pStream) << '{'     << sRTF_RTF;
#ifdef MAC
    (*m_pStream) << sRTF_MAC    << sNewLine;
#else
    (*m_pStream) << sRTF_ANSI   << sNewLine;
#endif

    /*
    // Access RTF Export Beispiel
    {\rtf1\ansi
        {\colortbl\red0\green0\blue0;\red255\green255\blue255;\red192\green192\blue192;}
        {\fonttbl\f0\fcharset0\fnil MS Sans Serif;\f1\fcharset0\fnil Arial;\f2\fcharset0\fnil Arial;}
        \trowd\trgaph40
                \clbrdrl\brdrs\brdrcf0\clbrdrt\brdrs\brdrcf0\clbrdrb\brdrs\brdrcf0\clbrdrr\brdrs\brdrcf0\clshdng10000\clcfpat2\cellx1437
                \clbrdrl\brdrs\brdrcf0\clbrdrt\brdrs\brdrcf0\clbrdrb\brdrs\brdrcf0\clbrdrr\brdrs\brdrcf0\clshdng10000\clcfpat2\cellx2874
        {
            \trrh-270\pard\intbl
                {\qc\fs20\b\f1\cf0\cb2 text\cell}
                \pard\intbl
                {\qc\fs20\b\f1\cf0\cb2 datum\cell}
                \pard\intbl\row
        }
        \trowd\trgaph40\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx1437\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx2874
        {\trrh-270\pard\intbl
            {\ql\fs20\f2\cf0\cb1 heute\cell}
            \pard\intbl
            {\qr\fs20\f2\cf0\cb1 10.11.98\cell}
            \pard\intbl\row
        }
        \trowd\trgaph40\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx1437\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx2874
        {\trrh-270\pard\intbl
            {\ql\fs20\f2\cf0\cb1 morgen\cell}
            \pard\intbl
            {\qr\fs20\f2\cf0\cb1 11.11.98\cell}
            \pard\intbl\row
        }
        \trowd\trgaph40\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx1437\clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx2874
        {\trrh-270\pard\intbl
            {\ql\fs20\f2\cf0\cb1 bruder\cell}
            \pard\intbl
            {\qr\fs20\f2\cf0\cb1 21.04.98\cell}
            \pard\intbl\row
        }
        \trowd\trgaph40
        \clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx
        \clbrdrl\brdrs\brdrcf2\clbrdrt\brdrs\brdrcf2\clbrdrb\brdrs\brdrcf2\clbrdrr\brdrs\brdrcf2\clshdng10000\clcfpat1\cellx2874
        {\trrh-270\pard\intbl
            {\ql\fs20\f2\cf0\cb1 vater\cell}
            \pard\intbl
            {\qr\fs20\f2\cf0\cb1 28.06.98\cell}
            \pard\intbl\row
        }
    }
    */

    BOOL bBold          = ( ::com::sun::star::awt::FontWeight::BOLD     == m_aFont.Weight );
    BOOL bItalic        = ( ::com::sun::star::awt::FontSlant_ITALIC     == m_aFont.Slant );
    BOOL bUnderline     = ( ::com::sun::star::awt::FontUnderline::NONE  != m_aFont.Underline );
    BOOL bStrikeout     = ( ::com::sun::star::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    Color aColor(nColor);

    ByteString aFonts(String(m_aFont.Name),gsl_getSystemTextEncoding());
    if(!aFonts.Len())
    {
        String aName = Application::GetSettings().GetStyleSettings().GetAppFont().GetName();;
        aFonts = ByteString (aName,gsl_getSystemTextEncoding());
    }
        // TODO : think about the encoding of the font name
    ::rtl::OString aFormat("\\fcharset0\\fnil ");
    ByteString aFontNr;

    (*m_pStream)    << "{\\fonttbl";
    for(xub_StrLen j=0;j<aFonts.GetTokenCount();++j)
    {
        (*m_pStream) << "\\f";
        (*m_pStream) << ::rtl::OString::valueOf((sal_Int32)j);
        (*m_pStream) << aFormat;
        (*m_pStream) << aFonts.GetToken(j).GetBuffer();
        (*m_pStream) << ';';
    }
    (*m_pStream) << '}' ;
    (*m_pStream) << sNewLine;
    (*m_pStream) << "{\\colortbl\\red"  << ::rtl::OString::valueOf((sal_Int32)aColor.GetRed())
                        << "\\green"    << ::rtl::OString::valueOf((sal_Int32)aColor.GetGreen())
                        << "\\blue"     << ::rtl::OString::valueOf((sal_Int32)aColor.GetBlue())
                        << ";\\red255\\green255\\blue255;\\red192\\green192\\blue192;}"     << sNewLine;

    sal_Int32 nCellx = 1437;
    ::rtl::OString aTRRH("\\trrh-270\\pard\\intbl");
    ::rtl::OString aFS("\\fs20\\f0\\cf0\\cb2");
    ::rtl::OString aFS2("\\fs20\\f1\\cf0\\cb1");
    ::rtl::OString aCell1("\\clbrdrl\\brdrs\\brdrcf0\\clbrdrt\\brdrs\\brdrcf0\\clbrdrb\\brdrs\\brdrcf0\\clbrdrr\\brdrs\\brdrcf0\\clshdng10000\\clcfpat2\\cellx");
    ::rtl::OString aCell2("\\clbrdrl\\brdrs\\brdrcf2\\clbrdrt\\brdrs\\brdrcf2\\clbrdrb\\brdrs\\brdrcf2\\clbrdrr\\brdrs\\brdrcf2\\clshdng10000\\clcfpat1\\cellx");

    ::rtl::OString s40 = ::rtl::OString::valueOf((sal_Int32)40);

    (*m_pStream) << sRTF_TROWD << sRTF_TRGAPH << s40 << sNewLine;

    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        Sequence< ::rtl::OUString> aNames(xColumns->getElementNames());
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();

        sal_Int32 nCount = aNames.getLength();

        sal_Int32 i;
        for(i=1;i<=nCount;++i)
        {
            (*m_pStream) << aCell1;
            (*m_pStream) << ::rtl::OString::valueOf(sal_Int32(i*nCellx));
            (*m_pStream) << sNewLine;
        }

        // Spaltenbeschreibung
        (*m_pStream) << '{' << sNewLine;
        (*m_pStream) << aTRRH;


        ::rtl::OString* pHorzChar = new ::rtl::OString[nCount];

        for(i=1;pBegin != pEnd;++pBegin,++i)
        {
            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pBegin) >>= xColumn;
            sal_Int32 nAlign = 0;
            xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;

            const char* pChar;
            switch( nAlign )
            {
                case 1: pChar = sRTF_QC;    break;
                case 2: pChar = sRTF_QR;    break;
                case 0:
                default:pChar = sRTF_QL;    break;
            }

            pHorzChar[i-1] = pChar; // um sp"ater nicht immer im ITEMSET zuw"uhlen

            (*m_pStream) << sNewLine;
            (*m_pStream) << '{';
            (*m_pStream) << sRTF_QC;   // column header always centered

            if ( bBold )        (*m_pStream) << sRTF_B;
            if ( bItalic )      (*m_pStream) << sRTF_I;
            if ( bUnderline )   (*m_pStream) << sRTF_UL;
            if ( bStrikeout )   (*m_pStream) << sRTF_STRIKE;

            (*m_pStream) << aFS;
            (*m_pStream) << ' ';
            (*m_pStream) << ::rtl::OString(*pBegin,pBegin->getLength(), gsl_getSystemTextEncoding());
                // TODO : think about the encoding of the column name
            (*m_pStream) << sRTF_CELL;
            (*m_pStream) << '}';
            (*m_pStream) << sNewLine;
            (*m_pStream) << sRTF_PARD   << sRTF_INTBL;
        }

        (*m_pStream) << sRTF_ROW;
        (*m_pStream) << sNewLine << '}';
        (*m_pStream) << sNewLine;

        sal_Int32 k=1;
        sal_Int32 kk=0;
        m_xResultSet->beforeFirst(); // set back before the first row
        while(m_xResultSet->next())
        {
            if(!m_pRowMarker || m_pRowMarker[kk] == k)
            {
                ++kk;
                (*m_pStream) << sRTF_TROWD << sRTF_TRGAPH << s40 << sNewLine;

                for(i=1;i<=nCount;++i)
                {
                    (*m_pStream) << aCell2;
                    (*m_pStream) << ::rtl::OString::valueOf((sal_Int32)(i*nCellx));
                    (*m_pStream) << sNewLine;
                }

                (*m_pStream) << '{';
                (*m_pStream) << aTRRH;
                for(sal_Int32 i=1;i<=nCount;++i)
                {
                    (*m_pStream) << sNewLine;
                    (*m_pStream) << '{';
                    (*m_pStream) << pHorzChar[i-1];

                    if ( bBold )        (*m_pStream) << sRTF_B;
                    if ( bItalic )      (*m_pStream) << sRTF_I;
                    if ( bUnderline )   (*m_pStream) << sRTF_UL;
                    if ( bStrikeout )   (*m_pStream) << sRTF_STRIKE;

                    (*m_pStream) << aFS2;
                    (*m_pStream) << ' ';

                    try
                    {
                        ::rtl::OUString sValue = m_xRow->getString(i);
                        if (!m_xRow->wasNull())
                        {
                            (*m_pStream) << ::rtl::OString(sValue,sValue.getLength(), gsl_getSystemTextEncoding());
                                // TODO : think about the encoding of the value string
                        }
                    }
                    catch (Exception&)
                    {
                        OSL_ENSURE(0,"RTF WRITE!");
                    }

                    (*m_pStream) << sRTF_CELL;
                    (*m_pStream) << '}';
                    (*m_pStream) << sNewLine;
                    (*m_pStream) << sRTF_PARD   << sRTF_INTBL;
                }
                (*m_pStream) << sRTF_ROW << sNewLine;
                (*m_pStream) << '}';
            }
            ++k;
        }

        delete [] pHorzChar;
    }

    (*m_pStream) << '}' << sNewLine;
    (*m_pStream) << (BYTE) 0;
    return ((*m_pStream).GetError() == SVSTREAM_OK);
}
//-------------------------------------------------------------------
BOOL ORTFImportExport::Read()
{
    m_pReader = new ORTFReader((*m_pStream),m_xConnection,m_xFormatter,m_xFactory);
    ((ORTFReader*)m_pReader)->AddRef();
    SvParserState eState = ((ORTFReader*)m_pReader)->CallParser();
    ((ORTFReader*)m_pReader)->ReleaseRef();

    return eState != SVPAR_ERROR;
}
//-------------------------------------------------------------------
//===================================================================
const sal_Int16 __FAR_DATA OHTMLImportExport::nDefaultFontSize[SBA_HTML_FONTSIZES] =
{
    HTMLFONTSZ1_DFLT, HTMLFONTSZ2_DFLT, HTMLFONTSZ3_DFLT, HTMLFONTSZ4_DFLT,
    HTMLFONTSZ5_DFLT, HTMLFONTSZ6_DFLT, HTMLFONTSZ7_DFLT
};

sal_Int16 OHTMLImportExport::nFontSize[SBA_HTML_FONTSIZES] = { 0 };

const sal_Int16 OHTMLImportExport::nCellSpacing = 0;
const char __FAR_DATA OHTMLImportExport::sIndentSource[nIndentMax+1] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

//========================================================================
// Makros fuer HTML-Export
//========================================================================
#define OUT_PROLOGUE()      ((*m_pStream) << sHTML30_Prologue << sNewLine << sNewLine)
#define TAG_ON( tag )       HTMLOutFuncs::Out_AsciiTag( (*m_pStream), tag )
#define TAG_OFF( tag )      HTMLOutFuncs::Out_AsciiTag( (*m_pStream), tag, FALSE )
#define OUT_STR( str )      HTMLOutFuncs::Out_String( (*m_pStream), str )
#define OUT_LF()            (*m_pStream) << sNewLine << GetIndentStr()
#define lcl_OUT_LF()        (*m_pStream) << sNewLine
#define TAG_ON_LF( tag )    (TAG_ON( tag ) << sNewLine << GetIndentStr())
#define TAG_OFF_LF( tag )   (TAG_OFF( tag ) << sNewLine << GetIndentStr())
#define OUT_HR()            TAG_ON_LF( sHTML_horzrule )
#define OUT_COMMENT( comment )  ((*m_pStream) << sMyBegComment, OUT_STR( comment ) << sMyEndComment << sNewLine << GetIndentStr())
#define lcl_OUT_COMMENT( comment )  ((*m_pStream) << sMyBegComment, OUT_STR( comment ) << sMyEndComment << sNewLine)

//-------------------------------------------------------------------
OHTMLImportExport::OHTMLImportExport(const Sequence< PropertyValue >& _aSeq,
                                     const Reference< XMultiServiceFactory >& _rM,
                                     const Reference< XNumberFormatter >& _rxNumberF,
                                     const String& rExchange)
        : ODatabaseImportExport(_aSeq,_rM,_rxNumberF,rExchange)
    ,m_nIndent(0)
#if DBG_UTIL
    ,m_bCheckFont(FALSE)
#endif
{
    strcpy( sIndent, sIndentSource );
    sIndent[0] = 0;
}
//-------------------------------------------------------------------
BOOL OHTMLImportExport::Write()
{
    if(m_xObject.is())
    {
        (*m_pStream) << '<' << sHTML_doctype << ' ' << sHTML_doctype32 << '>' << sNewLine << sNewLine;
        TAG_ON_LF( sHTML_html );
        WriteHeader();
        OUT_LF();
        WriteBody();
        OUT_LF();
        TAG_OFF_LF( sHTML_html );

        return ((*m_pStream).GetError() == SVSTREAM_OK);
    }
    return sal_False;
}
//-------------------------------------------------------------------
BOOL OHTMLImportExport::Read()
{
    m_pReader = new OHTMLReader((*m_pStream),m_xConnection,m_xFormatter,m_xFactory);
    ((OHTMLReader*)m_pReader)->AddRef();
    SvParserState eState = ((OHTMLReader*)m_pReader)->CallParser();
    ((OHTMLReader*)m_pReader)->ReleaseRef();

    return eState != SVPAR_ERROR;
}
//-------------------------------------------------------------------
void OHTMLImportExport::WriteHeader()
{
    SfxDocumentInfo rInfo;
    rInfo.SetTitle(m_sName);
    String  aStrOut;

    IncIndent(1); TAG_ON_LF( sHTML_head );

    SfxFrameHTMLWriter::Out_DocInfo( (*m_pStream), &rInfo, sIndent );
    OUT_LF();
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( sHTML_head );
}
//-----------------------------------------------------------------------
void OHTMLImportExport::WriteBody()
{

    IncIndent(1); TAG_ON_LF( sHTML_style );

    (*m_pStream) << sMyBegComment; OUT_LF();
    (*m_pStream) << sHTML_body << " { " << sFontFamily << '\"' << ::rtl::OString(m_aFont.Name,m_aFont.Name.getLength(), gsl_getSystemTextEncoding()) << '\"';
        // TODO : think about the encoding of the font name
    (*m_pStream) << "; " << sFontSize << ::rtl::OString::valueOf((sal_Int32)m_aFont.Height) << '}';
    OUT_LF();
    (*m_pStream) << sMyEndComment;
    IncIndent(-1); OUT_LF(); TAG_OFF_LF( sHTML_style );
    OUT_LF();

    // default Textfarbe schwarz
    (*m_pStream) << '<' << sHTML_body << ' ' << sHTML_O_text << '=';
    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    Color aColor(nColor);
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    ::rtl::OString sOut( ' ' );
    sOut = sOut + sHTML_O_bgcolor;
    sOut = sOut + "=";
    (*m_pStream) << sOut;
    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );

    (*m_pStream) << '>'; OUT_LF();

    WriteTables();

    TAG_OFF_LF( sHTML_body );
}
//-----------------------------------------------------------------------
void OHTMLImportExport::WriteTables()
{
    ::rtl::OString aStrOut  = sHTML_table;
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + sHTML_frame;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + sHTML_TF_void;

    Sequence< ::rtl::OUString> aNames;
    Reference<XNameAccess> xColumns;
    if(m_xObject.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xObject,UNO_QUERY);
        xColumns = xColSup->getColumns();
        aNames = xColumns->getElementNames();
    }

    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + sHTML_O_align;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + sHTML_AL_left;
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + sHTML_O_cellspacing;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + ::rtl::OString::valueOf((sal_Int32)nCellSpacing);
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + sHTML_O_cols;
    aStrOut = aStrOut + "=";
    aStrOut = aStrOut + ::rtl::OString::valueOf(aNames.getLength());
    aStrOut = aStrOut + " ";
    aStrOut = aStrOut + sHTML_O_border;
    aStrOut = aStrOut + "=1";

    IncIndent(1);
    TAG_ON( aStrOut );

    FontOn();

    TAG_ON( sHTML_caption );
    TAG_ON( sHTML_bold );

    (*m_pStream)    << ::rtl::OString(m_sName,m_sName.getLength(), gsl_getSystemTextEncoding());
        // TODO : think about the encoding of the name
    TAG_OFF( sHTML_bold );
    TAG_OFF( sHTML_caption );

    FontOff();
    OUT_LF();
    // </FONT>

    IncIndent(1);
    TAG_ON_LF( sHTML_thead );

    IncIndent(1);
    TAG_ON_LF( sHTML_tablerow );

    if(m_xObject.is())
    {
        sal_Int32* pFormat = new sal_Int32[aNames.getLength()];

        char **pHorJustify = new char*[aNames.getLength()];
        sal_Int32 *pColWidth = new sal_Int32[aNames.getLength()];


        sal_Int32 nHeight = 0;
        m_xObject->getPropertyValue(PROPERTY_ROW_HEIGHT) >>= nHeight;

        // 1. die Spaltenbeschreibung rauspusten
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();

        for(sal_Int32 i=0;pBegin != pEnd;++pBegin,++i)
        {

            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pBegin) >>= xColumn;
            sal_Int32 nAlign = 0;
            xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;

            pColWidth[i] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_WIDTH));

            switch( nAlign )
            {
                case 1:     pHorJustify[i] = sHTML_AL_center;   break;
                case 2:     pHorJustify[i] = sHTML_AL_right;    break;
                default:    pHorJustify[i] = sHTML_AL_left;     break;
            }

            pFormat[i] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_FORMATKEY));
            if(i == aNames.getLength()-1)
                IncIndent(-1);

            WriteCell(pFormat[i],pColWidth[i],nHeight,pHorJustify[i],*pBegin,sHTML_tableheader);
        }

        IncIndent(-1);
        TAG_OFF_LF( sHTML_tablerow );
        TAG_OFF_LF( sHTML_thead );

        IncIndent(1);
        TAG_ON_LF( sHTML_tbody );

        // 2. und jetzt die Daten

        sal_Int32 j=1;
        sal_Int32 kk=0;
        m_xResultSet->beforeFirst(); // set back before the first row
        while(m_xResultSet->next())
        {
            IncIndent(1);
            TAG_ON_LF( sHTML_tablerow );

            if(!m_pRowMarker || m_pRowMarker[kk] == j)
            {
                ++kk;
                for(sal_Int32 i=1;i<=aNames.getLength();++i)
                {
                    if(i == aNames.getLength())
                        IncIndent(-1);

                    String aValue;
                    try
                    {
                        ::rtl::OUString sValue = m_xRow->getString(i);
                        if (!m_xRow->wasNull())
                        {
                            aValue = sValue;
                        }
                    }
                    catch ( Exception& )
                    {
                        OSL_ENSURE(sal_False, "OHTMLImportExport::WriteTables: caught an exception!");
                    }
                    WriteCell(pFormat[i-1],pColWidth[i-1],nHeight,pHorJustify[i-1],aValue,sHTML_tabledata);
                }
            }
            ++j;
            TAG_OFF_LF( sHTML_tablerow );
        }

        delete [] pFormat;
        delete [] pHorJustify;
        delete [] pColWidth;
    }
    else
    {
        IncIndent(-1);
        TAG_OFF_LF( sHTML_tablerow );
        TAG_OFF_LF( sHTML_thead );

        IncIndent(1);
        TAG_ON_LF( sHTML_tbody );
    }

    IncIndent(-1); OUT_LF(); TAG_OFF_LF( sHTML_tbody );
    IncIndent(-1); TAG_OFF_LF( sHTML_table );
}
//-----------------------------------------------------------------------
void OHTMLImportExport::WriteCell( sal_Int32 nFormat,sal_Int32 nWidthPixel,sal_Int32 nHeightPixel,const char* pChar,const String& rValue,const char* pHtmlTag)
{
    BOOL bValueData;
    bValueData = FALSE;

    ::rtl::OString aStrTD = pHtmlTag;

    nWidthPixel  = nWidthPixel  ? nWidthPixel   : 86;
    nHeightPixel = nHeightPixel ? nHeightPixel  : 17;

    // trotz der <TABLE COLS=n> und <COL WIDTH=x> Angaben noetig,
    // da die nicht von Netscape beachtet werden..
    // Spaltenbreite
    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + sHTML_O_width;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + ::rtl::OString::valueOf((sal_Int32)nWidthPixel);
    // Zeilenhoehe
    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + sHTML_O_height;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + ::rtl::OString::valueOf((sal_Int32)nHeightPixel);

    aStrTD = aStrTD + " ";
    aStrTD = aStrTD + sHTML_O_align;
    aStrTD = aStrTD + "=";
    aStrTD = aStrTD + pChar;

    double fVal = 0.0;

    Reference< XNumberFormatsSupplier >  xSupplier = m_xFormatter->getNumberFormatsSupplier();

    Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
    SvNumberFormatsSupplierObj* pSupplierImpl = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());
    SvNumberFormatter* pFormatter = pSupplierImpl ? pSupplierImpl->GetNumberFormatter() : NULL;
    if(pFormatter)
    {
        try
        {
            fVal = m_xFormatter->convertStringToNumber(nFormat,rValue);
            HTMLOutFuncs::CreateTableDataOptionsValNum( ByteString(aStrTD), bValueData, fVal,nFormat, *pFormatter );
        }
        catch(Exception&)
        {
            HTMLOutFuncs::CreateTableDataOptionsValNum( ByteString(aStrTD), bValueData, fVal,nFormat, *pFormatter );
        }
    }

    TAG_ON( aStrTD );

    FontOn();

    BOOL bBold          = ( ::com::sun::star::awt::FontWeight::BOLD     == m_aFont.Weight );
    BOOL bItalic        = ( ::com::sun::star::awt::FontSlant_ITALIC     == m_aFont.Slant );
    BOOL bUnderline     = ( ::com::sun::star::awt::FontUnderline::NONE  != m_aFont.Underline );
    BOOL bStrikeout     = ( ::com::sun::star::awt::FontStrikeout::NONE  != m_aFont.Strikeout );

    if ( bBold )        TAG_ON( sHTML_bold );
    if ( bItalic )      TAG_ON( sHTML_italic );
    if ( bUnderline )   TAG_ON( sHTML_underline );
    if ( bStrikeout )   TAG_ON( sHTML_strike );

    if ( !rValue.Len() )
        TAG_ON( sHTML_linebreak );      // #42573# keine komplett leere Zelle
    else
        OUT_STR( rValue );

    if ( bStrikeout )   TAG_OFF( sHTML_strike );
    if ( bUnderline )   TAG_OFF( sHTML_underline );
    if ( bItalic )      TAG_OFF( sHTML_italic );
    if ( bBold )        TAG_OFF( sHTML_bold );

    FontOff();

    TAG_OFF_LF( pHtmlTag );
}
//-----------------------------------------------------------------------
void OHTMLImportExport::FontOn()
{
#if DBG_UTIL
        m_bCheckFont = TRUE;
#endif

    // <FONT FACE="xxx">
    ::rtl::OString aStrOut  = "<";
    aStrOut  = aStrOut + sHTML_font;
    aStrOut  = aStrOut + " ";
    aStrOut  = aStrOut + sHTML_O_face;
    aStrOut  = aStrOut + "=";
    aStrOut  = aStrOut + "\"";
    aStrOut  = aStrOut + ::rtl::OString(m_aFont.Name,m_aFont.Name.getLength(),gsl_getSystemTextEncoding());
        // TODO : think about the encoding of the font name
    aStrOut  = aStrOut + "\"";
    aStrOut  = aStrOut + " ";
    aStrOut  = aStrOut + sHTML_O_color;
    aStrOut  = aStrOut + "=";
    (*m_pStream) << aStrOut;

    sal_Int32 nColor = 0;
    if(m_xObject.is())
        m_xObject->getPropertyValue(PROPERTY_TEXTCOLOR) >>= nColor;
    Color aColor(nColor);

    HTMLOutFuncs::Out_Color( (*m_pStream), aColor );
    (*m_pStream) << ">";
}
//-----------------------------------------------------------------------
inline void OHTMLImportExport::FontOff()
{
    DBG_ASSERT(m_bCheckFont,"Kein FontOn() gerufen");
    TAG_OFF( sHTML_font );
#if DBG_UTIL
    m_bCheckFont = FALSE;
#endif
}
//-----------------------------------------------------------------------
void OHTMLImportExport::IncIndent( sal_Int16 nVal )
{
    sIndent[m_nIndent] = '\t';
    m_nIndent += nVal;
    if ( m_nIndent < 0 )
        m_nIndent = 0;
    else if ( m_nIndent > nIndentMax )
        m_nIndent = nIndentMax;
    sIndent[m_nIndent] = 0;
}
// -----------------------------------------------------------------------------

