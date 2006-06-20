/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftransl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:07:07 $
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


#ifndef _FTRANSL_HXX_
#define _FTRANSL_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
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

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// declarations
//------------------------------------------------------------------------

struct FormatEntry
{
   FormatEntry();

   FormatEntry(
        const char* mime_content_type,
        const char* human_presentable_name,
        const char* native_format_name,
        CLIPFORMAT std_clipboard_format_id,
        com::sun::star::uno::Type const & cppu_type
    );

    com::sun::star::datatransfer::DataFlavor aDataFlavor;
    rtl::OUString                            aNativeFormatName;
    sal_Int32                                aStandardFormatId;
};

//------------------------------------------------
// CDataFormatTranslator
//------------------------------------------------

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

    sal_Bool isTextPlainMediaType( const rtl::OUString& fullMediaType ) const;

    com::sun::star::datatransfer::DataFlavor SAL_CALL mkDataFlv( const rtl::OUString& cnttype, const rtl::OUString& hpname, ::com::sun::star::uno::Type dtype );

private:
    std::vector< FormatEntry >  m_TranslTable;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;
};


#endif

