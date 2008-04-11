/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fieldmappingimpl.hxx,v $
 * $Revision: 1.5 $
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

#ifndef EXTENSIONS_ABP_FIELDMAPPINGIMPL_HXX
#define EXTENSIONS_ABP_FIELDMAPPINGIMPL_HXX

#include <rtl/ustring.hxx>
#include "abptypes.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include "addresssettings.hxx"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace beans {
        class XPropertySet;
    }
} } }
class Window;

//.........................................................................
namespace abp
{
//.........................................................................

    //.....................................................................
    namespace fieldmapping
    {
    //.....................................................................

        //-----------------------------------------------------------------
        /** invokes the field mapping dialog
            @param _rxORB
                service factory to use for creating UNO services
            @param _pParent
                window to use as parent for the dialog and error messages
            @param _rSettings
                current settings. Upon return, the field mapping member of this
                structure will be filled with the settings the user did in the
                field mapping dialog.
        */
        sal_Bool invokeDialog(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            class Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDataSource,
            AddressSettings& _rSettings
        ) SAL_THROW ( ( ) );

        //-----------------------------------------------------------------
        /** creates a default field mapping for usage with the address book SDBC driver
            <p>The column names as used by the SDBC driver for address books is stored in the configuration,
            and this function creates a mapping which uses this configuration information.</p>
        */
        void defaultMapping(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            MapString2String& /* [out] */ _rFieldAssignment
        ) SAL_THROW ( ( ) );

        //-----------------------------------------------------------------
        /** writes a field mapping for the template document address source
        */
        void writeTemplateAddressFieldMapping(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const MapString2String& _rFieldAssignment
        ) SAL_THROW ( ( ) );

    //.....................................................................
    }   // namespace fieldmapping
    //.....................................................................

    //.....................................................................
    namespace addressconfig
    {
    //.....................................................................

        //-----------------------------------------------------------------
        /** writes the data source / table name given into the configuration, to where the template documents
            expect it.
        */
        void writeTemplateAddressSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rDataSourceName,
            const ::rtl::OUString& _rTableName
        ) SAL_THROW ( ( ) );

        /** writes the configuration entry which states the the pilot has been completed successfully
        */
        void markPilotSuccess(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        ) SAL_THROW ( ( ) );

    //.....................................................................
    }   // namespace addressconfig
    //.....................................................................

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_FIELDMAPPINGIMPL_HXX

