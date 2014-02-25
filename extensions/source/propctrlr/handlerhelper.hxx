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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/inspection/XPropertyControlFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/Optional.hpp>

#include <vector>

class Window;
namespace com { namespace sun { namespace star {
    namespace inspection {
        struct LineDescriptor;
    }
} } }

namespace pcr
{



    //= PropertyHandlerHelper

    class PropertyHandlerHelper
    {
    public:
        /** helper for implementing XPropertyHandler::describePropertyLine in a generic way
        */
        static  void describePropertyLine(
                const ::com::sun::star::beans::Property& _rProperty,
                ::com::sun::star::inspection::LineDescriptor& /* [out] */ _out_rDescriptor,
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory
            );

        /** helper for implementing XPropertyHandler::convertToPropertyValue
        */
        static ::com::sun::star::uno::Any convertToPropertyValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter,
                const ::com::sun::star::beans::Property& _rProperty,
                const ::com::sun::star::uno::Any& _rControlValue
            );

        /// helper for implementing XPropertyHandler::convertToControlValue
        static ::com::sun::star::uno::Any convertToControlValue(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter,
                const ::com::sun::star::uno::Any& _rPropertyValue,
                const ::com::sun::star::uno::Type& _rControlValueType
            );

        /** creates an <member scope="com::sun::star::inspection">PropertyControlType::ListBox</member>-type control
            and fills it with initial values

            @param _rxControlFactory
                A control factory. Must not be <NULL/>.

            @param  _rInitialListEntries
                the initial values of the control

            @param _bReadOnlyControl
                determines whether the control should be read-only

            @param _bSorted
                determines whether the list entries should be sorted

            @return
                the newly created control
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createListBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl,
                sal_Bool _bSorted
            );

        /** creates an <member scope="com::sun::star::inspection">PropertyControlType::ComboBox</member>-type control
            and fills it with initial values

            @param _rxControlFactory
                A control factory. Must not be <NULL/>.

            @param  _rInitialListEntries
                the initial values of the control

            @param _bReadOnlyControl
                determines whether the control should be read-only

            @param _bSorted
                determines whether the list entries should be sorted

            @return
                the newly created control
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createComboBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl,
                sal_Bool _bSorted
            );

        /** creates an <member scope="com::sun::star::inspection">PropertyControlType::NumericField</member>-type control
            and initializes it

            @param _rxControlFactory
                A control factory. Must not be <NULL/>.
            @param _nDigits
                number of decimal digits for the control
                (<member scope="com::sun::star::inspection">XNumericControl::DecimalDigits</member>)
            @param _rMinValue
                minimum value which can be entered in the control
                (<member scope="com::sun::star::inspection">XNumericControl::MinValue</member>)
            @param _rMaxValue
                maximum value which can be entered in the control
                (<member scope="com::sun::star::inspection">XNumericControl::MaxValue</member>)
            @param _bReadOnlyControl
                determines whether the control should be read-only

            @return
                the newly created control
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createNumericControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                sal_Int16 _nDigits,
                const ::com::sun::star::beans::Optional< double >& _rMinValue,
                const ::com::sun::star::beans::Optional< double >& _rMaxValue,
                sal_Bool _bReadOnlyControl
            );

        /** marks the document passed in our UNO context as modified

            The method looks up a value called "ContextDocument" in the given UNO component context,
            queries it for the ->com::sun::star::util::XModifiable interface, and calls its
            setModified method. If either of those steps fails, this is asserted in a non-product
            version, and silently ignore otherwise.

            @param _rContext
                the component context which was used to create the component calling this method
        */
        static void setContextDocumentModified(
                const css::uno::Reference< css::uno::XComponentContext > & _rContext
            );

        static css::uno::Reference< css::uno::XInterface > getContextDocument( const css::uno::Reference<css::uno::XComponentContext> & _rContext );

        static css::uno::Reference< css::uno::XInterface > getContextDocument_throw( const css::uno::Reference<css::uno::XComponentContext> & _rContext ) throw (css::uno::RuntimeException);

        /** gets the window of the ObjectInspector in which an property handler lives

            The method looks up a value called "DialogParentWindow" in the given UNO copmonent context,
            queries it for XWindow, and returns the respective Window*. If either of those steps fails,
            this is asserted in a non-product version, and silently ignore otherwise.

            @param  _rContext
                the component context which was used to create the component calling this method
        */
        static Window* getDialogParentWindow( const css::uno::Reference< css::uno::XComponentContext > & _rContext );


        /** determines whether given PropertyAttributes require a to-be-created
            <type scope="com::sun::star::inspection">XPropertyControl</type> to be read-only

            @param  _nPropertyAttributes
                the attributes of the property which should be reflected by a to-be-created
                <type scope="com::sun::star::inspection">XPropertyControl</type>
        */
        inline static sal_Bool requiresReadOnlyControl( sal_Int16 _nPropertyAttributes )
        {
            return ( _nPropertyAttributes & ::com::sun::star::beans::PropertyAttribute::READONLY ) != 0;
        }

    private:
        PropertyHandlerHelper();                                            // never implemented
        PropertyHandlerHelper( const PropertyHandlerHelper& );              // never implemented
        PropertyHandlerHelper& operator=( const PropertyHandlerHelper& );   // never implemented
    };


} // namespace pcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
