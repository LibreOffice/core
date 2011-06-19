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

#ifndef _SVX_DBAEXCHANGE_HXX_
#define _SVX_DBAEXCHANGE_HXX_

#include <svtools/transfer.hxx>
#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "svx/svxdllapi.h"

//........................................................................
namespace svx
{
//........................................................................

// column transfer formats
#define CTF_FIELD_DESCRIPTOR        0x0001      // the field descriptor format
#define CTF_CONTROL_EXCHANGE        0x0002      // the control exchange format
#define CTF_COLUMN_DESCRIPTOR       0x0004      // data access descriptor for a column

    //====================================================================
    //= OColumnTransferable
    //====================================================================
    class SVX_DLLPUBLIC OColumnTransferable : public TransferableHelper
    {
    protected:
        ODataAccessDescriptor   m_aDescriptor;
        ::rtl::OUString         m_sCompatibleFormat;
        sal_Int32               m_nFormatFlags;

    public:
        /** construct the transferable
        */
        OColumnTransferable(
             const ::rtl::OUString& _rDatasource
            ,const ::rtl::OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const ::rtl::OUString& _rCommand
            ,const ::rtl::OUString& _rFieldName
            ,sal_Int32  _nFormats
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
            sal_Int32                    _nFormats
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
            const ::rtl::OUString& _rFieldName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            sal_Int32   _nFormats
        );

        /** checks whether or not a column descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static sal_Bool canExtractColumnDescriptor(const DataFlavorExVector& _rFlavors, sal_Int32 _nFormats);

        /** extracts a column descriptor from the transferable given
        */
        static sal_Bool extractColumnDescriptor(
             const TransferableDataHelper&  _rData
            ,::rtl::OUString&               _rDatasource
            ,::rtl::OUString&               _rDatabaseLocation
            ,::rtl::OUString&               _rConnectionResource
            ,sal_Int32&                     _nCommandType
            ,::rtl::OUString&               _rCommand
            ,::rtl::OUString&               _rFieldName
        );

        /** extracts a column descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractColumnDescriptor(const TransferableDataHelper& _rData);

        /** adds the data contained in the object to the given data container
            <p>This method helps you treating this class as simple container class:<br/>
            At the moment, it is a data container and a transferable.
            Using <method>addDataToContainer</method>, you can treat the class as dumb data container,
            doing the Drag'n'Drop with a <type>TransferDataContainer</type> instance (which may contain
            additional formats)</p>
            @TODO
                split this class into a two separate classes: one for the data container aspect, one for
                the transfer aspect
        */
        void addDataToContainer( TransferDataContainer* _pContainer );

    protected:
        // TransferableHelper overridables
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

        static sal_uInt32   getDescriptorFormatId();

    private:
        SVX_DLLPRIVATE void implConstruct(
            const ::rtl::OUString&  _rDatasource
            ,const ::rtl::OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const ::rtl::OUString& _rCommand
            ,const ::rtl::OUString& _rFieldName
        );
    };

    //====================================================================
    //= ODataAccessObjectTransferable
    //====================================================================
    /** class for transfering data access objects (tables, queries, statements ...)
    */
    class SVX_DLLPUBLIC ODataAccessObjectTransferable : public TransferableHelper
    {
        ODataAccessDescriptor   m_aDescriptor;
        ::rtl::OUString         m_sCompatibleObjectDescription;
            // needed to provide a SOT_FORMATSTR_ID_SBA_DATAEXCHANGE format

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
            const ::rtl::OUString&  _rDatasourceOrLocation
            ,const ::rtl::OUString& _rConnectionResource
            ,const sal_Int32            _nCommandType
            ,const ::rtl::OUString& _rCommand
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
            const ::rtl::OUString&  _rDatasourceOrLocation
            ,const ::rtl::OUString& _rConnectionResource
            ,const sal_Int32        _nCommandType
            ,const ::rtl::OUString& _rCommand
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
        static sal_Bool canExtractObjectDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts a object descriptor from the transferable given
        */
        static ODataAccessDescriptor
                        extractObjectDescriptor(const TransferableDataHelper& _rData);

    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();

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
        SVX_DLLPRIVATE void construct(  const ::rtl::OUString&  _rDatasourceOrLocation
                        ,const ::rtl::OUString& _rConnectionResource
                        ,const sal_Int32        _nCommandType
                        ,const ::rtl::OUString& _rCommand
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                        ,sal_Bool _bAddCommand
                        ,const ::rtl::OUString& _sActiveCommand);
    };

    //====================================================================
    //= OMultiColumnTransferable
    //====================================================================
    /** class for transfering multiple columns
    */
    class SVX_DLLPUBLIC OMultiColumnTransferable : public TransferableHelper
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aDescriptors;

    public:
        OMultiColumnTransferable(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _aDescriptors);

        void push_back(ODataAccessDescriptor& _aDescriptor);

        /** checks whether or not an object descriptor can be extracted from the data flavor vector given
            @param _rFlavors
                available flavors
            @param _nFormats
                formats to accept
        */
        static sal_Bool canExtractDescriptor(const DataFlavorExVector& _rFlavors);

        /** extracts a object descriptor from the transferable given
        */
        static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > extractDescriptor(const TransferableDataHelper& _rData);

    protected:
        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        ObjectReleased();
        static sal_uInt32   getDescriptorFormatId();
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // _SVX_DBAEXCHANGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
