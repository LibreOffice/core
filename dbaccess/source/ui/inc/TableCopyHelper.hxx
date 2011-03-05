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
#ifndef DBUI_TABLECOPYHELPER_HXX
#define DBUI_TABLECOPYHELPER_HXX

#include "AppElementType.hxx"
#include "commontypes.hxx"
#include <svx/dataaccessdescriptor.hxx>
#include <sot/storage.hxx>
#include <svtools/transfer.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <functional>

class SvLBoxEntry;
//........................................................................
namespace dbaui
{
//........................................................................
    class OGenericUnoController;
    /// unary_function Functor object for class DataFlavorExVector::value_type returntype is bool
    struct TAppSupportedSotFunctor : ::std::unary_function<DataFlavorExVector::value_type,bool>
    {
        ElementType eEntryType;
        sal_Bool    bQueryDrop;
        TAppSupportedSotFunctor(const ElementType& _eEntryType,sal_Bool _bQueryDrop)
            : eEntryType(_eEntryType)
            , bQueryDrop(_bQueryDrop)
        {
        }

        inline bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
                case SOT_FORMAT_RTF:                    // RTF data descriptions
                case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
                case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                    return (E_TABLE == eEntryType);
                case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
                case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                    return ((E_QUERY == eEntryType) || ( !bQueryDrop && E_TABLE == eEntryType));
            }
            return false;
        }
    };

    class OTableCopyHelper
    {
    private:
        OGenericUnoController*  m_pController;
        ::rtl::OUString         m_sTableNameForAppend;

    public:
        // is needed to describe the drop target
        struct DropDescriptor
        {
            ::svx::ODataAccessDescriptor    aDroppedData;

            //for transfor the tablename
            ::rtl::OUString                 sDefaultTableName;

            String                          aUrl;
            SotStorageStreamRef             aHtmlRtfStorage;
            ElementType                     nType;
            SvLBoxEntry*                    pDroppedAt;
            sal_Int8                        nAction;
            sal_Bool                        bHtml;
            sal_Bool                        bError;

            DropDescriptor() : nType(E_TABLE),pDroppedAt(NULL),nAction(DND_ACTION_NONE) { }
        };

        OTableCopyHelper(OGenericUnoController* _pControler);

        /** pastes a table into the data source
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( const TransferableDataHelper& _rTransData
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const SharedConnection& _xConnection);

        /** pastes a table into the data source
            @param  _nFormatId
                The format which should be copied.
            @param  _rPasteData
                The data helper.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable( SotFormatStringId _nFormatId
                        ,const TransferableDataHelper& _rTransData
                        ,const ::rtl::OUString& _sDestDataSourceName
                        ,const SharedConnection& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _rDesc
                The Drop descriptor
            @param  _bCheck
                If set to <TRUE/> than the controller checks only if a copy is possible.
            @param  _xConnection
                The connection
        */
        sal_Bool copyTagTable(  DropDescriptor& _rDesc,
                                sal_Bool _bCheck,
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
                                ,const ::rtl::OUString& _sDestDataSourceName
                                ,const SharedConnection& _xConnection);

        /** copies a table which was constructed by tags like HTML or RTF
            @param  _aDroppedData
                The dropped data
            @param  _rDesc
                IN/OUT parameter
            @param  _xConnection
                The connection
        */
        sal_Bool copyTagTable(const TransferableDataHelper& _aDroppedData,
                              DropDescriptor& _rAsyncDrop,
                              const SharedConnection& _xConnection);

        /// returns <TRUE/> if the clipboard supports a table format, otherwise <FALSE/>.
        sal_Bool isTableFormat(const TransferableDataHelper& _rClipboard) const;

        inline void                     SetTableNameForAppend( const ::rtl::OUString& _rDefaultTableName ) { m_sTableNameForAppend = _rDefaultTableName; }
        inline void                     ResetTableNameForAppend() { SetTableNameForAppend( ::rtl::OUString() ); }
        inline const ::rtl::OUString&   GetTableNameForAppend() const { return m_sTableNameForAppend ;}

    private:
        /** pastes a table into the data source
            @param  _rPasteData
                The data descriptor.
            @param  _sDestDataSourceName
                The name of the dest data source.
        */
        void pasteTable(
            const ::svx::ODataAccessDescriptor& _rPasteData,
            const ::rtl::OUString& _sDestDataSourceName,
            const SharedConnection& _xDestConnection
        );

        /** insert a table into the data source. The source can eihter be a table or a query
        */
        void insertTable(
            const ::rtl::OUString& i_rSourceDataSource,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& i_rSourceConnection,
            const ::rtl::OUString& i_rCommand,
            const sal_Int32 i_nCommandType,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >& i_rSourceRows,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& i_rSelection,
            const sal_Bool i_bBookmarkSelection,
            const ::rtl::OUString& i_rDestDataSource,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& i_rDestConnection
        );

    };
//........................................................................
}   // namespace dbaui
//........................................................................
#endif // DBUI_TABLECOPYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
