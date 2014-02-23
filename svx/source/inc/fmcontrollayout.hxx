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

#ifndef INCLUDED_SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX
#define INCLUDED_SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

#include <com/sun/star/beans/XPropertySet.hpp>

#include "fmdocumentclassification.hxx"
#include <unotools/confignode.hxx>


namespace svxform
{



    //= ControlLayouter

    class ControlLayouter
    {
    public:
        /** initializes the layout of a newly created form control (model)
        */
        static void initializeControlLayout(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                        DocumentType _eDocType
                    );

        /** determines whether for the given document type, dynamic control border coloring is enabled
        */
        static bool useDynamicBorderColor( DocumentType _eDocType );

        /** determines whether for the given document type, form controls should use the document's reference device
            for text rendering
        */
        static bool useDocumentReferenceDevice( DocumentType _eDocType );

        /** gets the "default" style in a document which can be used if some default text format is needed

            It depends on the type document type which concrete kind of style is returned, but it is expected to support
            the css.style.CharacterProperties service.

            @param _rxModel
                a form component.
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    getDefaultDocumentTextStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    private:
        ControlLayouter();    // never implemented

    private:
        static ::utl::OConfigurationNode
            getLayoutSettings(
                DocumentType _eDocType
            );
    };


} // namespace svxform


#endif // INCLUDED_SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
