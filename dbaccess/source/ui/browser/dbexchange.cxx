/*************************************************************************
 *
 *  $RCSfile: dbexchange.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-23 15:13:25 $
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
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        if(nUserObjectId == SOT_FORMAT_RTF || nUserObjectId == SOT_FORMATSTR_ID_HTML)
        {
            ODatabaseImportExport* pExport = reinterpret_cast<ODatabaseImportExport*>(pUserObject);
            if(pExport)
            {
                pExport->setStream(&rxOStm);
                return pExport->Write();
            }
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void ODataClipboard::AddSupportedFormats()
    {
        AddFormat(SOT_FORMAT_RTF);
        AddFormat(SOT_FORMATSTR_ID_HTML);
        if(m_aSeq.getLength())
            AddFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE);
    }
    // -----------------------------------------------------------------------------
    sal_Bool ODataClipboard::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
    {
        ULONG nFormat = SotExchange::GetFormat(rFlavor);
        if(nFormat == SOT_FORMAT_RTF)
        {
            return SetObject(m_pRtf,SOT_FORMAT_RTF,rFlavor);
        }
        else if(nFormat == SOT_FORMATSTR_ID_HTML)
        {
            return SetObject(m_pHtml,SOT_FORMATSTR_ID_HTML,rFlavor);
        }
        else if(nFormat == SOT_FORMATSTR_ID_SBA_DATAEXCHANGE)
        {
            return SetAny(makeAny(m_aSeq),rFlavor);
        }
        return sal_False;
    }
    // -----------------------------------------------------------------------------
    void ODataClipboard::ObjectReleased()
    {
        m_xHtml = NULL;
        m_xRtf  = NULL;
        m_aSeq  = Sequence< PropertyValue >();
    }
    // -----------------------------------------------------------------------------
}








