/*************************************************************************
 *
 *  $RCSfile: xmlexchg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 11:21:11 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_XMLEXCHG_HXX_
#include "xmlexchg.hxx"
#endif

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::datatransfer;

    //====================================================================
    //= OXFormsTransferable
    //====================================================================
    //--------------------------------------------------------------------
    OXFormsTransferable::OXFormsTransferable()
    {
    }
    //--------------------------------------------------------------------
    sal_uInt32 OXFormsTransferable::getDescriptorFormatId()
    {
        static sal_uInt32 s_nFormat = (sal_uInt32)-1;
        if ((sal_uInt32)-1 == s_nFormat)
        {
            s_nFormat = SotExchange::RegisterFormatName( String::CreateFromAscii("application/x-openoffice;windows_formatname=\"???\"") );
            OSL_ENSURE( (sal_uInt32)-1 != s_nFormat, "OXFormsTransferable::getDescriptorFormatId: bad exchange id!" );
        }
        return s_nFormat;
    }
    //--------------------------------------------------------------------
    void OXFormsTransferable::AddSupportedFormats()
    {
        AddFormat( SOT_FORMATSTR_ID_XFORMS );
    }
    //--------------------------------------------------------------------
    sal_Bool OXFormsTransferable::GetData( const DataFlavor& _rFlavor )
    {
        const sal_uInt32 nFormatId = SotExchange::GetFormat( _rFlavor );
        if ( SOT_FORMATSTR_ID_XFORMS == nFormatId )
        {
            return SetString( ::rtl::OUString( String::CreateFromAscii("XForms-Transferable") ), _rFlavor );
        }
        return sal_False;
    }


//........................................................................
}   // namespace svx
//........................................................................


