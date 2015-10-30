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

#ifndef INCLUDED_SVX_SOURCE_INC_FORMCONTROLFACTORY_HXX
#define INCLUDED_SVX_SOURCE_INC_FORMCONTROLFACTORY_HXX

#include "fmdocumentclassification.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <memory>

class SdrUnoObj;
class Rectangle;

namespace comphelper {
    class ComponentContext;
}


namespace svxform
{

    struct FormControlFactory_Data;

    class FormControlFactory
    {
    public:
        FormControlFactory( const css::uno::Reference<css::uno::XComponentContext>& _rContext );
        FormControlFactory();
        ~FormControlFactory();

        /** initializes the given control model which is to be newly inserted into a document

            @param _eDocType
                the type of the document which the control is to be inserted into
            @param _rxControlModel
                the control model to be inserted
            @param _rControlBoundRect
                the bound rect of the control, if applicable
            @return
                the class ID of the control
        */
        sal_Int16 initializeControlModel(
                    const DocumentType _eDocType,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
                    const Rectangle& _rControlBoundRect
                );

        sal_Int16 initializeControlModel( const DocumentType _eDocType, const SdrUnoObj& _rObject );
        sal_Int16 initializeControlModel( const DocumentType _eDocType, const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel );

        void initializeTextFieldLineEnds(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxModel
                );

        static void initializeFieldDependentProperties(
                    const css::uno::Reference< css::beans::XPropertySet >& _rxDatabaseField,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
                    const css::uno::Reference< css::util::XNumberFormats >& _rxNumberFormats
                );

        static OUString getDefaultName(
                    const sal_Int16 nClassId,
                    const css::uno::Reference< css::lang::XServiceInfo >& _rxObject
                );

        static OUString getDefaultUniqueName_ByComponentType(
                    const css::uno::Reference< css::container::XNameAccess >& _rxContainer,
                    const css::uno::Reference< css::beans::XPropertySet >& _rxObject
                );

        static OUString getUniqueName(
                    const css::uno::Reference< css::container::XNameAccess >& _rxContainer,
                    const OUString& _rBaseName
                );

    private:
        ::std::unique_ptr< FormControlFactory_Data >  m_pData;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FORMCONTROLFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
