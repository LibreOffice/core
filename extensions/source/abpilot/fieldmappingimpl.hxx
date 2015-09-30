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

#ifndef INCLUDED_EXTENSIONS_SOURCE_ABPILOT_FIELDMAPPINGIMPL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_ABPILOT_FIELDMAPPINGIMPL_HXX

#include <rtl/ustring.hxx>
#include "abptypes.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include "addresssettings.hxx"

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace beans {
        class XPropertySet;
    }
} } }
namespace vcl { class Window; }


namespace abp
{



    namespace fieldmapping
    {



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
        bool invokeDialog(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            class vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxDataSource,
            AddressSettings& _rSettings
        );


        /** creates a default field mapping for usage with the address book SDBC driver
            <p>The column names as used by the SDBC driver for address books is stored in the configuration,
            and this function creates a mapping which uses this configuration information.</p>
        */
        void defaultMapping(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            MapString2String& /* [out] */ _rFieldAssignment
        );


        /** writes a field mapping for the template document address source
        */
        void writeTemplateAddressFieldMapping(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const MapString2String& _rFieldAssignment
        );


    }   // namespace fieldmapping



    namespace addressconfig
    {



        /** writes the data source / table name given into the configuration, to where the template documents
            expect it.
        */
        void writeTemplateAddressSource(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const OUString& _rDataSourceName,
            const OUString& _rTableName
        );

        /** writes the configuration entry which states the pilot has been completed successfully
        */
        void markPilotSuccess(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );


    }   // namespace addressconfig



}   // namespace abp


#endif // INCLUDED_EXTENSIONS_SOURCE_ABPILOT_FIELDMAPPINGIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
