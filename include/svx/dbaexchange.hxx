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
#include <svtools/transfer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <svx/svxdllapi.h>


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

    class SVX_DLLPUBLIC SAL_WARN_UNUSED OColumnTransferable : public TransferableHelper
    {
    protected:
        ODataAccessDescriptor      m_aDescriptor;
        OUString                   m_sCompatibleFormat;
        ColumnTransferFormatFlags  m_nFormatFlags;

    public:
        /** construct the transferable
        */
        OColumnTransferable(
             const OUString& _rDatasource
            ,const OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const OUString& _rCommand
            ,const OUString& _rFieldName
            ,ColumnTransferFormatFlags  _nFormats
        );

        /** construct the transferable from a data access descriptor

            Note that some of the aspects, in particular all which cannot be represented
            as string, can only be transported via the CTF_COLUMN_DESCRIPTOR format.

        @param _rDescriptor
            The descriptor for the column. It must contain at least
            <ul><li>information sufficient to create a connection, that is, either one of DataSource, DatabaseLocation,
                    ConnectionResource, and daConnection</li>
                <li>a Command</li>
                <li>a CommandType</li>
                <li>a ColumnName or ColumnObject</li>
            </ul>
        */
        OColumnTransferable(
            const ODataAccessDescriptor& _rDescriptor,
            ColumnTransferFormatFlags    _nFormats
        );

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
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxForm,
            const OUString& _rFieldName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
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

        /** adds the data contained in the object to the given data container
            <p>This method helps you treating this class as simple container class:<br/>
            At the moment, it is a data container and a transferable.
            Using <method>addDataToContainer</method>, you can treat the class as dumb data container,
            doing the Drag'n'Drop with a TransferDataContainer instance (which may contain
            additional formats)</p>
            @TODO
                split this class into a two separate classes: one for the data container aspect, one for
                the transfer aspect
        */
        void addDataToContainer( TransferDataContainer* _pContainer );

    protected:
        // TransferableHelper overridables
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

        static SotClipboardFormatId  getDescriptorFormatId();

    private:
        SVX_DLLPRIVATE void implConstruct(
            const OUString&  _rDatasource
            ,const OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const OUString& _rCommand
            ,const OUString& _rFieldName
        );
    };


    //= ODataAccessObjectTransferable

    /** class for transferring data access objects (tables, queries, statements ...)
    */
    class SVX_DLLPUBLIC SAL_WARN_UNUSED ODataAccessObjectTransferable : public TransferableHelper
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
            ,const OUString& _rConnectionResource
            ,const sal_Int32            _nCommandType
            ,const OUString& _rCommand
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
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
            ,const OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const OUString& _rCommand
        );

        /** with this ctor, only the object descriptor format will be provided
        */
        ODataAccessObjectTransferable(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxLivingForm
        );

        /** checks whether or not an object descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static bool canExtractObjectDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts a object descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractObjectDescriptor(const TransferableDataHelper& _rData);

    protected:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        ObjectReleased() override;

    protected:
        const   ODataAccessDescriptor&  getDescriptor() const   { return m_aDescriptor; }
                ODataAccessDescriptor&  getDescriptor()         { return m_aDescriptor; }
    protected:
        void    addCompatibleSelectionDescription(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& _rSelRows
        );
            // normally, a derived class could simply access getDescriptor[daSelection] and place the sequence therein
            // but unfortunately, we have this damned compatible format, and this can't be accessed in
            // derived classes (our class is the only one which should be contaminated with this)

    private:
        SVX_DLLPRIVATE void construct(  const OUString&  _rDatasourceOrLocation
                        ,const OUString& _rConnectionResource
                        ,const sal_Int32        _nCommandType
                        ,const OUString& _rCommand
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                        ,bool _bAddCommand
                        ,const OUString& _sActiveCommand);
    };


    //= OMultiColumnTransferable

    /** class for transferring multiple columns
    */
    class SVX_DLLPUBLIC SAL_WARN_UNUSED OMultiColumnTransferable : public TransferableHelper
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aDescriptors;

    public:
        OMultiColumnTransferable(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aDescriptors);

        /** checks whether or not an object descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static bool canExtractDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts a object descriptor from the transferable given
        */
        static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > extractDescriptor(const TransferableDataHelper& _rData);

    protected:
        virtual void        AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void        ObjectReleased() override;
        static SotClipboardFormatId  getDescriptorFormatId();
    };


}


#endif // INCLUDED_SVX_DBAEXCHANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
