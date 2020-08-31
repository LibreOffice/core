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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECOPYHELPER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECOPYHELPER_HXX

#include "AppElementType.hxx"
#include "commontypes.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <sot/storage.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

namespace dbaui
{
    class OGenericUnoController;
    /// Functor object for class DataFlavorExVector::value_type returntype is bool
    struct TAppSupportedSotFunctor
    {
        ElementType eEntryType;
        TAppSupportedSotFunctor(const ElementType& _eEntryType)
            : eEntryType(_eEntryType)
        {
        }

        bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
                case SotClipboardFormatId::RTF:                    // RTF data descriptions
                case SotClipboardFormatId::HTML:             // HTML data descriptions
                case SotClipboardFormatId::DBACCESS_TABLE:   // table descriptor
                    return (E_TABLE == eEntryType);
                case SotClipboardFormatId::DBACCESS_QUERY:   // query descriptor
                case SotClipboardFormatId::DBACCESS_COMMAND: // SQL command
                    return E_QUERY == eEntryType;
                default: break;
            }
            return false;
        }
    };

    class OTableCopyHelper
    {
    private:
        OGenericUnoController*  m_pController;
        OUString         m_sTableNameForAppend;

    public:
        // is needed to describe the drop target
        struct DropDescriptor
        {
            svx::ODataAccessDescriptor    aDroppedData;

            //for transfor the tablename
            OUString                 sDefaultTableName;

            OUString                        aUrl;
            tools::SvRef<SotStorageStream>             aHtmlRtfStorage;
            ElementType                     nType;
            std::unique_ptr<weld::TreeIter> xDroppedAt;
            sal_Int8                        nAction;
            bool                        bHtml;
            bool                        bError;

            DropDescriptor()
                : nType(E_TABLE)
                , nAction(DND_ACTION_NONE)
                , bHtml(false)
                , bError(false)
                { }
        };

        OTableCopyHelper(OGenericUnoController* _pController);

        /** pastes a table into the data source
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( const TransferableDataHelper& _rTransData
                        ,const OUString& _sDestDataSourceName
                        ,const SharedConnection& _xConnection);

        /** pastes a table into the data source
            @param  _nFormatId
                The format which should be copied.
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( SotClipboardFormatId _nFormatId
                        ,const TransferableDataHelper& _rTransData
                        ,const OUString& _sDestDataSourceName
                        ,const SharedConnection& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
            @param  _xConnection
                The connection
        */
        bool copyTagTable(  DropDescriptor const & _rDesc,
                            bool _bCheck,
                            const SharedConnection& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
            @param  _xConnection
                The connection
        */
        void asyncCopyTagTable(  DropDescriptor& _rDesc
                                ,const OUString& _sDestDataSourceName
                                ,const SharedConnection& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _aDroppedData
                The dropped data
            @param  _rDesc
                IN/OUT parameter
            @param  _xConnection
                The connection
        */
        bool copyTagTable(const TransferableDataHelper& _aDroppedData,
                          DropDescriptor& _rAsyncDrop,
                          const SharedConnection& _xConnection);

        /// returns <TRUE/> if the clipboard supports a table format, otherwise <FALSE/>.
        static bool isTableFormat(const TransferableDataHelper& _rClipboard);

        void                     SetTableNameForAppend( const OUString& _rDefaultTableName ) { m_sTableNameForAppend = _rDefaultTableName; }
        void                     ResetTableNameForAppend() { SetTableNameForAppend( OUString() ); }
        const OUString&   GetTableNameForAppend() const { return m_sTableNameForAppend ;}

    private:
        /** pastes a table into the data source
            @param  _rPasteData
                The data descriptor.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable(
            const svx::ODataAccessDescriptor& _rPasteData,
            const OUString& _sDestDataSourceName,
            const SharedConnection& _xDestConnection
        );

        /** insert a table into the data source. The source can either be a table or a query
        */
        void insertTable(
            const OUString& i_rSourceDataSource,
            const css::uno::Reference< css::sdbc::XConnection>& i_rSourceConnection,
            const OUString& i_rCommand,
            const sal_Int32 i_nCommandType,
            const css::uno::Reference< css::sdbc::XResultSet >& i_rSourceRows,
            const css::uno::Sequence< css::uno::Any >& i_rSelection,
            const bool i_bBookmarkSelection,
            const OUString& i_rDestDataSource,
            const css::uno::Reference< css::sdbc::XConnection>& i_rDestConnection
        );

    };
}   // namespace dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLECOPYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
