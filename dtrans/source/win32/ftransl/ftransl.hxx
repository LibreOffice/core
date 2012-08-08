/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#ifndef _FTRANSL_HXX_
#define _FTRANSL_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XDataFormatTranslator.hpp>
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include "../misc/WinClip.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
