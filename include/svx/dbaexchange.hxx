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

#ifndef INCLUDED_SVX_DBAEXCHANGE_HXX
#define INCLUDED_SVX_DBAEXCHANGE_HXX

#include <sal/config.h>

#include <o3tl/typed_flags_set.hxx>
#include <vcl/transfer.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <svx/svxdllapi.h>

namespace com::sun::star::sdbc { class XConnection; }
namespace com::sun::star::beans { class XPropertySet; }

// column transfer formats
enum class ColumnTransferFormatFlags
{
    FIELD_DESCRIPTOR        = 0x01,    // the field descriptor format
    CONTROL_EXCHANGE        = 0x02,    // the control exchange format
    COLUMN_DESCRIPTOR       = 0x04,    // data access descriptor for a column
};
namespace o3tl
{
    template<> struct typed_flags<ColumnTransferFormatFlags> : is_typed_flags<ColumnTransferFormatFlags, 0x07> {};
}


namespace svx
{


    //= OColumnTransferable

    class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC OColumnTransferable final : public TransferDataContainer
    {
    public:
        OColumnTransferable(ColumnTransferFormatFlags nFormats);

        /** construct the transferable from a data access descriptor

            Note that some of the aspects, in particular all which cannot be represented
            as string, can only be transported via the CTF_COLUMN_DESCRIPTOR format.

        @param rDescriptor
            The descriptor for the column. It must contain at least
            <ul><li>information sufficient to create a connection, that is, either one of DataSource, DatabaseLocation,
                    ConnectionResource, and DataAccessDescriptorProperty::Connection</li>
                <li>a Command</li>
                <li>a CommandType</li>
                <li>a ColumnName or ColumnObject</li>
            </ul>
        */
        void setDescriptor(const ODataAccessDescriptor& rDescriptor);


        /** construct the transferable from a DatabaseForm component and a field name

            @param _rxForm
                the form which is bound to the data source which's field is to be dragged

            @param _rFieldName
                the name of the field to be dragged

            @param _rxColumn
                the column object. Won't be used if <arg>_nFormats</arg> does not include the CTF_COLUMN_DESCRIPTOR
                flag.<br/>
                May be <NULL/>.

            @param _rxConnection
                the connection the column belongs to. Won't be used if <arg>_nFormats</arg> does not include the CTF_COLUMN_DESCRIPTOR
                flag.<br/>
                May be <NULL/>.

            @param _nFormats
                supported formats. Must be a combination of the CTF_XXX flags
        */
        OColumnTransferable(
            const css::uno::Reference< css::beans::XPropertySet >& _rxForm,
            const OUString& _rFieldName,
            const css::uno::Reference< css::beans::XPropertySet >& _rxColumn,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            ColumnTransferFormatFlags  _nFormats
        );

        /** checks whether or not a column descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static bool canExtractColumnDescriptor(const DataFlavorExVector& _rFlavors, ColumnTransferFormatFlags _nFormats);

        /** extracts a column descriptor from the transferable given
        */
        static bool extractColumnDescriptor(
             const TransferableDataHelper&  _rData
            ,OUString&               _rDatasource
            ,OUString&               _rDatabaseLocation
            ,OUString&               _rConnectionResource
            ,sal_Int32&                     _nCommandType
            ,OUString&               _rCommand
            ,OUString&               _rFieldName
        );

        /** extracts a column descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractColumnDescriptor(const TransferableDataHelper& _rData);

    private:
        // TransferableHelper overridables
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

        static SotClipboardFormatId  getDescriptorFormatId();

        SVX_DLLPRIVATE void implConstruct(
            const OUString&  _rDatasource
            ,const OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const OUString& _rCommand
            ,const OUString& _rFieldName
        );

        ODataAccessDescriptor      m_aDescriptor;
        OUString                   m_sCompatibleFormat;
        ColumnTransferFormatFlags  m_nFormatFlags;
    };


    //= ODataAccessObjectTransferable

    /** class for transferring data access objects (tables, queries, statements ...)
    */
    class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC ODataAccessObjectTransferable : public TransferDataContainer
    {
        ODataAccessDescriptor   m_aDescriptor;
        OUString         m_sCompatibleObjectDescription;
            // needed to provide a SotClipboardFormatId::SBA_DATAEXCHANGE format

    public:
        /** should be used copying and the connection is needed.
            @param  _rDatasource
                The data source name.
            @param  _nCommandType
                The kind of command. @see com.sun.star.sdbc.CommandType
            @param  _rCommand
                The command, either a name of a table or query or a SQL statement.
        */
        ODataAccessObjectTransferable(
            const OUString&  _rDatasourceOrLocation
            ,const sal_Int32 _nCommandType
            ,const OUString& _rCommand
            ,const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
        );

        void Update(
            const OUString&  _rDatasourceOrLocation,
            const sal_Int32 _nCommandType,
            const OUString& _rCommand,
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
        );

        /** should be used when copying a query object and no connection is available.
            @param  _rDatasource
                The data source name.
            @param  _nCommandType
                The kind of command. @see com.sun.star.sdbc.CommandType
            @param  _rCommand
                The command, either a name of a table or query or a SQL statement.
        */
        ODataAccessObjectTransferable(
            const OUString&  _rDatasourceOrLocation
            ,const sal_Int32 _nCommandType
            ,const OUString& _rCommand
        );

        void Update(
            const OUString& _rDatasourceOrLocation,
            const sal_Int32 _nCommandType,
            const OUString& _rCommand
        );

        /** with this ctor, only the object descriptor format will be provided
        */
        ODataAccessObjectTransferable(
            const css::uno::Reference< css::beans::XPropertySet >& _rxLivingForm
        );

        ODataAccessObjectTransferable();

        /** checks whether or not an object descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static bool canExtractObjectDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts an object descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractObjectDescriptor(const TransferableDataHelper& _rData);

    protected:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        ObjectReleased() override;

    protected:
        const   ODataAccessDescriptor&  getDescriptor() const   { return m_aDescriptor; }
                ODataAccessDescriptor&  getDescriptor()         { return m_aDescriptor; }
    protected:
        void    addCompatibleSelectionDescription(
            const css::uno::Sequence< css::uno::Any >& _rSelRows
        );
            // normally, a derived class could simply access getDescriptor[DataAccessDescriptorProperty::Selection] and place the sequence therein
            // but unfortunately, we have this damned compatible format, and this can't be accessed in
            // derived classes (our class is the only one which should be contaminated with this)

    private:
        SVX_DLLPRIVATE void construct(  const OUString&  _rDatasourceOrLocation
                        ,const OUString& _rConnectionResource
                        ,const sal_Int32        _nCommandType
                        ,const OUString& _rCommand
                        ,const css::uno::Reference< css::sdbc::XConnection >& _rxConnection
                        ,bool _bAddCommand
                        ,const OUString& _sActiveCommand);
    };


    //= OMultiColumnTransferable

    /** class for transferring multiple columns
    */
    class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC OMultiColumnTransferable final : public TransferDataContainer
    {
    public:
        OMultiColumnTransferable();

        void setDescriptors(const css::uno::Sequence< css::beans::PropertyValue >& rDescriptors);

        /** checks whether or not an object descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static bool canExtractDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts an object descriptor from the transferable given
        */
        static css::uno::Sequence< css::beans::PropertyValue > extractDescriptor(const TransferableDataHelper& _rData);

    private:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        ObjectReleased() override;
        static SotClipboardFormatId  getDescriptorFormatId();

        css::uno::Sequence< css::beans::PropertyValue >   m_aDescriptors;
    };


}


#endif // INCLUDED_SVX_DBAEXCHANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
