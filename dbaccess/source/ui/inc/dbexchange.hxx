/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_DBEXCHANGE_HXX
#define DBAUI_DBEXCHANGE_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svx/dbaexchange.hxx>
#include <comphelper/uno3.hxx>
#include <vector>

#include <rtl/ref.hxx>

namespace dbaui
{

    class ORTFImportExport;
    class OHTMLImportExport;

    class ODataClipboard : public ::svx::ODataAccessObjectTransferable

    {
        ::rtl::Reference< OHTMLImportExport >   m_pHtml;
        ::rtl::Reference< ORTFImportExport >    m_pRtf;

    public:
        ODataClipboard(
            const ::rtl::OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const ::rtl::OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        ODataClipboard(
            const ::rtl::OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const ::rtl::OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );

        ODataClipboard(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_rAliveForm,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& i_rSelectedRows,
            const sal_Bool i_bBookmarkSelection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_rORB
        );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
        virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    };
}

#endif //  DBAUI_DBEXCHANGE_HXX







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
