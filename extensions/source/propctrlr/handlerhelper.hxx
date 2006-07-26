/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: handlerhelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:58:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROLFACTORY_HPP_
#include <com/sun/star/inspection/XPropertyControlFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_OPTIONAL_HPP_
#include <com/sun/star/beans/Optional.hpp>
#endif
/** === end UNO includes === **/

#include <vector>

class Window;
namespace com { namespace sun { namespace star {
    namespace inspection {
        struct LineDescriptor;
    }
} } }
//........................................................................
namespace pcr
{
//........................................................................

    class ComponentContext;

    //====================================================================
    //= PropertyHandlerHelper
    //====================================================================
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
                const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter,
                const ::com::sun::star::beans::Property& _rProperty,
                const ::com::sun::star::uno::Any& _rControlValue
            );

        /// helper for implementing XPropertyHandler::convertToControlValue
        static ::com::sun::star::uno::Any convertToControlValue(
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

            @return
                the newly created control
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createListBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl
            );

        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createListBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl
            );

        /** creates an <member scope="com::sun::star::inspection">PropertyControlType::ComboBox</member>-type control
            and fills it with initial values

            @param _rxControlFactory
                A control factory. Must not be <NULL/>.

            @param  _rInitialListEntries
                the initial values of the control

            @param _bReadOnlyControl
                determines whether the control should be read-only

            @return
                the newly created control
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createComboBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl
            );

        static ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >
            createComboBoxControl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl
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
                const ComponentContext& _rContext
            );

        /** gets the window of the ObjectInspector in which an property handler lives

            The method looks up a value called "DialogParentWindow" in the given UNO copmonent context,
            queries it for XWindow, and returns the respective Window*. If either of those steps fails,
            this is asserted in a non-product version, and silently ignore otherwise.

            @param  _rContext
                the component context which was used to create the component calling this method
        */
        static Window* getDialogParentWindow( const ComponentContext& _rContext );


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

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX

