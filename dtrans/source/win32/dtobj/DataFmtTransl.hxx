/*************************************************************************
 *
 *  $RCSfile: DataFmtTransl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-02 15:45:42 $
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


#ifndef _DATAFMTTRANSL_HXX_
#define _DATAFMTTRANSL_HXX_

//-----------------------------------------------
// includes
//-----------------------------------------------

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XDATAFORMATTRANSLATOR_HPP_
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#include <windows.h>

//-----------------------------------------------
// declaration
//-----------------------------------------------

class CFormatEtc;

class CDataFormatTranslator
{
public:
    CDataFormatTranslator( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&  aServiceManager );

    CFormatEtc getFormatEtcFromDataFlavor( const com::sun::star::datatransfer::DataFlavor& aDataFlavor ) const;
    com::sun::star::datatransfer::DataFlavor getDataFlavorFromFormatEtc(
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& refXTransferable, const FORMATETC& aFormatEtc ) const;

    CFormatEtc SAL_CALL getFormatEtcForClipformat( CLIPFORMAT cf ) const;

private:
    rtl::OUString SAL_CALL getClipboardFormatName( CLIPFORMAT aClipformat ) const;
    sal_Bool      SAL_CALL isOemOrAnsiTextClipformat( CLIPFORMAT aClipformat ) const;

    rtl::OUString SAL_CALL getCodePageFromLocaleId( LCID locale, LCTYPE lcType ) const;
    LCID          SAL_CALL getCurrentLocaleFromClipboard( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& refXTransferable ) const;
    rtl::OUString SAL_CALL makeMimeCharsetFromLocaleId( LCID lcid, LCTYPE lcType, const rtl::OUString& aCharsetValuePrefix ) const;
    rtl::OUString SAL_CALL getTextCharsetFromClipboard(
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& refXTransferable, CLIPFORMAT aClipformat ) const;

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;
    com::sun::star::uno::Reference< com::sun::star::datatransfer::XDataFormatTranslator >   m_XDataFormatTranslator;
};

#endif