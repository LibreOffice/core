/*************************************************************************
 *
 *  $RCSfile: dbexchange.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-15 14:50:06 $
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

#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SV_EXCHANGE_HXX
#include <vcl/exchange.hxx>
#endif

using namespace dbaui;

String ODataExchange::aDataExchangeFormat;
String ODataExchange::aRTFExchangeFormat;
String ODataExchange::aHTMLExchangeFormat;


TYPEINIT0( ODataExchange );
DBG_NAME(ODataExchange);
//------------------------------------------------------------------------
ODataExchange::ODataExchange(const String& _rExchangeStr)
{
    DBG_CTOR(ODataExchange,NULL);
    if(!aDataExchangeFormat.Len())
        aDataExchangeFormat = Exchange::GetFormatName(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE);
    if(!aRTFExchangeFormat.Len())
        aRTFExchangeFormat = Exchange::GetFormatName(SOT_FORMAT_RTF);
    if(!aHTMLExchangeFormat.Len())
        aHTMLExchangeFormat = Exchange::GetFormatName(SOT_FORMATSTR_ID_HTML);

    m_aDataExchange = _rExchangeStr;

    SvData* pData = new SvData( Exchange::RegisterFormatName(aDataExchangeFormat), MEDIUM_MEMORY );
    Append(pData);
    // TODO insert the format for rtf and html
}

//------------------------------------------------------------------------
ODataExchange::~ODataExchange()
{
    DBG_DTOR(ODataExchange,NULL);
}

//------------------------------------------------------------------------
void ODataExchange::SetBookmark( const String& rURL, const String& rLinkName )
{
}
//------------------------------------------------------------------------
BOOL ODataExchange::GetData( SvData* pData )
{
    ULONG nFormat = pData->GetFormat();

    //////////////////////////////////////////////////////////////////////
    // Wenn Daten in SBA_RTF_EXCHANGE_FORMAT, koennten wir diese verarbeiten
    // TODO enable rtf and html format again
    if(nFormat==FORMAT_RTF)
    {
        SvMemoryStream aStrm;
//      SbaRTFImportExport aExport(aStrm,*m_xSourceDef,m_aDataExchange);
//      BOOL bErr;
//      if(bErr = aExport.Write())
//          pData->SetData( const_cast<void*>((const void*)aStrm), aStrm.Tell() );
//      return bErr;
    }

    if(nFormat==SOT_FORMATSTR_ID_HTML)
    {

        SvMemoryStream aStrm;
//      SbaHTMLImportExport aExport(aStrm,*m_xSourceDef,m_aDataExchange);
//      BOOL bErr;
//      if(bErr = aExport.Write())
//          pData->SetData( const_cast<void*>((const void*)aStrm), aStrm.Tell() );
//      return bErr;
    }
    if( m_aDataExchange.Len() && (nFormat == Exchange::RegisterFormatName(aDataExchangeFormat)))
    {
        pData->SetData( m_aDataExchange );
        return TRUE;
    }
    //////////////////////////////////////////////////////////////////////
    // Alle anderen Formate werden an die Basisklasse weitergegeben
    return SvDataMemberObject::GetData( pData );
}
// -----------------------------------------------------------------------------




