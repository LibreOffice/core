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

#ifndef SVX_FORMCONTROLMANAGER_HXX
#define SVX_FORMCONTROLMANAGER_HXX

#include "fmdocumentclassification.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <memory>

class SdrUnoObj;
class Rectangle;

namespace comphelper {
    class ComponentContext;
}

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= FormControlFactory
    //====================================================================
    struct FormControlFactory_Data;

    class FormControlFactory
    {
    public:
        FormControlFactory( const ::comphelper::ComponentContext& _rContext );
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
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                    const Rectangle& _rControlBoundRect
                );

        sal_Int16 initializeControlModel( const DocumentType _eDocType, const SdrUnoObj& _rObject );
        sal_Int16 initializeControlModel( const DocumentType _eDocType, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel );

        void initializeTextFieldLineEnds(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                );

        void    initializeFieldDependentProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatabaseField,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& _rxNumberFormats
                );

        static OUString getDefaultName(
                    const sal_Int16 nClassId,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >& _rxObject
                );

        static OUString getDefaultUniqueName_ByComponentType(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject
                );

        static OUString getUniqueName(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
                    const OUString& _rBaseName
                );

    private:
        ::std::auto_ptr< FormControlFactory_Data >  m_pData;
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FORMCONTROLMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
