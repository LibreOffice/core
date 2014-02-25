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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DBEXCHANGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DBEXCHANGE_HXX

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

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

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
            const OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

        ODataClipboard(
            const OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const OUString&  _rCommand,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB
        );

        ODataClipboard(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& i_rAliveForm,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& i_rSelectedRows,
            const sal_Bool i_bBookmarkSelection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_rORB
        );

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
        virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBEXCHANGE_HXX







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
