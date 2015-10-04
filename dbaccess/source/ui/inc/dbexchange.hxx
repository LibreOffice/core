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

    class ODataClipboard : public svx::ODataAccessObjectTransferable

    {
        ::rtl::Reference< OHTMLImportExport >   m_pHtml;
        ::rtl::Reference< ORTFImportExport >    m_pRtf;

    public:
        ODataClipboard(
            const OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const OUString&  _rCommand,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::util::XNumberFormatter >& _rxFormatter,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        ODataClipboard(
            const OUString&  _rDatasource,
            const sal_Int32         _nCommandType,
            const OUString&  _rCommand,
            const css::uno::Reference< css::util::XNumberFormatter >& _rxFormatter,
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB
        );

        ODataClipboard(
            const css::uno::Reference< css::beans::XPropertySet >& i_rAliveForm,
            const css::uno::Sequence< css::uno::Any >& i_rSelectedRows,
            const bool i_bBookmarkSelection,
            const css::uno::Reference< css::uno::XComponentContext >& i_rORB
        );

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        ObjectReleased() override;
        virtual bool        WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, SotClipboardFormatId nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBEXCHANGE_HXX







/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
