/*************************************************************************
 *
 *  $RCSfile: ftransl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tra $ $Date: 2001-02-27 07:38:03 $
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


#ifndef _FTRANSL_HXX_
#define _FTRANSL_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XDATAFORMATTRANSLATOR_HPP_
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _WINCLIP_HXX_
#include "..\misc\WinClip.hxx"
#endif

#include <vector>
#include <windows.h>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

struct FormatEntry
{
    com::sun::star::datatransfer::DataFlavor aDataFlavor;
    rtl::OUString                            aNativeFormatName;
    sal_Int32                                aStandardFormatId;
};

class CDataFormatTranslator : public
    cppu::WeakImplHelper2< com::sun::star::datatransfer::XDataFormatTranslator, \
    com::sun::star::lang::XServiceInfo >
{

public:
    CDataFormatTranslator( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSrvMgr );

    //------------------------------------------------
    // XDataFormatTranslator
    //------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL getSystemDataTypeFromDataFlavor( const ::com::sun::star::datatransfer::DataFlavor& aDataFlavor )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::datatransfer::DataFlavor SAL_CALL getDataFlavorFromSystemDataType( const ::com::sun::star::uno::Any& aSysDataType )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

private:
    void SAL_CALL initTranslationTable( );

    void SAL_CALL findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, com::sun::star::datatransfer::DataFlavor& aDataFlavor ) const;
    void SAL_CALL findDataFlavorForNativeFormatName( const rtl::OUString& aNativeFormatName, com::sun::star::datatransfer::DataFlavor& aDataFlavor ) const;
    void SAL_CALL findStandardFormatIdForCharset( const rtl::OUString& aCharset, com::sun::star::uno::Any& aAny ) const;
    void SAL_CALL setStandardFormatIdForNativeFormatName( const rtl::OUString& aNativeFormatName, com::sun::star::uno::Any& aAny ) const;
    void SAL_CALL findStdFormatIdOrNativeFormatNameForFullMediaType(
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentTypeFactory >& aRefXMimeFactory,
        const rtl::OUString& aFullMediaType, com::sun::star::uno::Any& aAny ) const;

    com::sun::star::datatransfer::DataFlavor SAL_CALL mkDataFlv( const rtl::OUString& cnttype, const rtl::OUString& hpname, ::com::sun::star::uno::Type dtype );

    sal_Bool isTextPlainMediaType( const rtl::OUString& fullMediaType ) const;

    FormatEntry SAL_CALL mkFormatEntry( const com::sun::star::datatransfer::DataFlavor& aDataFlavor,
                                        const rtl::OUString& aNativeFormatName,
                                        sal_Int32 aStandardFormatId );

    FormatEntry mkPrivateFormatEntry( const rtl::OUString& aNativeFormatName,
                                      CLIPFORMAT aClipformat = CF_INVALID,
                                      const rtl::OUString& aHpName = rtl::OUString::createFromAscii( "" ),
                                      com::sun::star::uno::Type aCppuType = ::getCppuType( (const com::sun::star::uno::Sequence< sal_Int8 >*) 0 ));

    FormatEntry mkPublicFormatEntry( const rtl::OUString& aContentType,
                                     const rtl::OUString& aNativeFormatName = rtl::OUString::createFromAscii( "" ),
                                     CLIPFORMAT aClipformat = CF_INVALID,
                                     const rtl::OUString& aHpName = rtl::OUString::createFromAscii( "" ),
                                     com::sun::star::uno::Type aCppuType = ::getCppuType( (const com::sun::star::uno::Sequence< sal_Int8 >*) 0 ));

private:
    std::vector< FormatEntry >  m_TranslTable;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;
};


#endif

