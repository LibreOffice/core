/*************************************************************************
 *
 *  $RCSfile: eformshelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-07-01 11:49:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XMODEL_HPP_
#include <com/sun/star/xforms/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XFORMSSUPPLIER_HPP_
#include <com/sun/star/xforms/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XBINDABLEVALUE_HPP_
#include <com/sun/star/form/binding/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSOURCE_HPP_
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#endif
/** === end UNO includes === **/

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <vector>
#include <set>
#include <map>

//........................................................................
namespace pcr
{
//........................................................................

    typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >, ::std::less< ::rtl::OUString > >
            MapStringToPropertySet;

    //====================================================================
    //= EFormsHelper
    //====================================================================
    class EFormsHelper
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xControlModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XBindableValue >
                    m_xBindableControl;
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XFormsSupplier >
                    m_xDocument;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >
                    m_xBindingListener;
        MapStringToPropertySet
                    m_aSubmissionUINames;   // only filled upon request
        MapStringToPropertySet
                    m_aBindingUINames;      // only filled upon request

    public:
        EFormsHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
        );

        /** determines whether the given document is an eForm

            If this method returns <FALSE/>, you cannot instantiate a EFormsHelper with
            this document, since then no of it's functionality will be available.
        */
        static  bool
                isEForm(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
                ) SAL_THROW(());

        /** registers a listener to be notified when any aspect of the binding changes.

            The listener will be registered at the current binding of the control model. If the binding
            changes (see <method>setBinding</method>), the listener will be revoked from the old binding,
            registered at the new binding, and for all properties which differ between both bindings,
            the listener will be notified.
            @see revokeBindingListener
        */
        void    registerBindingListener(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxBindingListener
                );

        /** revokes the binding listener which has previously been registered
            @see registerBindingListener
        */
        void    revokeBindingListener();

        /** checks whether it's possible to bind the control model to a given XSD data type

            @param _nDataType
                the data type which should be bound. If this is -1, <TRUE/> is returned if the control model
                can be bound to <em>any</em> data type.
        */
        bool    canBindToDataType( sal_Int32 _nDataType = -1 ) const SAL_THROW(());

        /** checks whether the control model cna be bound to any XSD data type
        */
        bool    canBindToAnyDataType() const SAL_THROW(()) { return canBindToDataType( -1 ); }

        /** checks whether the control model is a source for list entries, as supplied by XML data bindings
        */
        bool    isListEntrySink() const SAL_THROW(());

        /** retrieves the names of all XForms models in the document the control lives in
        */
        void    getFormModelNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rModelNames ) const SAL_THROW(());

        /** retrieves the names of all bindings for a given model
            @see getFormModelNames
        */
        void    getBindingNames( const ::rtl::OUString& _rModelName, ::std::vector< ::rtl::OUString >& /* [out] */ _rBindingNames ) const SAL_THROW(());

        /// retrieves the XForms model (within the control model's document) with the given name
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XModel >
                getFormModelByName( const ::rtl::OUString& _rModelName ) const SAL_THROW(());

        /** retrieves the model which the active binding of the control model belongs to
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XModel >
                getCurrentFormModel() const SAL_THROW(());

        /** retrieves the name of the model which the active binding of the control model belongs to
        */
        ::rtl::OUString
                getCurrentFormModelName() const SAL_THROW(());

        /** retrieves the binding instance which is currently attached to the control model
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                getCurrentBinding() const SAL_THROW(());

        /** retrieves the name of the binding instance which is currently attached to the control model
        */
        ::rtl::OUString
                getCurrentBindingName() const SAL_THROW(());

        /** sets a new binding at the control model
        */
        void    setBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxBinding ) SAL_THROW(());

        /** retrieves the binding instance which is currently used as list source for the control model
            @see isListEntrySink
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >
                getCurrentListSourceBinding() const SAL_THROW(());

        /** sets a new list source at the control model
            @see isListEntrySink
        */
        void    setListSourceBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntrySource >& _rxListSource ) SAL_THROW(());

        /** creates a new binding for the given target model

            @param _rTargetModel
                the name of the model to create a binding for. If empty, a default model is chosen from the models
                available at the document. If this fails, no binding is created.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                createBindingForFormModel( const ::rtl::OUString& _rTargetModel ) const SAL_THROW(());

        /** retrieves a given binding for a given model, or creates a new one

            @param _rTargetModel
                the name of the model to create a binding for. Must not be empty
            @param _rBindingName
                the name of the binding to retrieve. If the model denoted by <arg>_rTargetModel</arg> does not
                have a binding with this name, a new binding is created and returned.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            getOrCreateBindingForModel( const ::rtl::OUString& _rTargetModel, const ::rtl::OUString& _rBindingName ) const SAL_THROW(());

        /** types of sub-elements of a model
        */
        enum ModelElementType
        {
            Submission,
            Binding
        };

        /** retrieves the name of a model's sub-element, as to be shown in the UI
            @see getModelElementFromUIName
            @see getAllElementUINames
        */
        ::rtl::OUString
                getModelElementUIName(
                    const ModelElementType _eType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement
                ) const SAL_THROW(());

        /** retrieves the submission object for an UI name

            Note that <member>getAllElementUINames</member> must have been called before, for the given element type

            @see getModelElementUIName
            @see getAllElementUINames
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                getModelElementFromUIName(
                    const ModelElementType _eType,
                    const ::rtl::OUString& _rUIName
                ) const SAL_THROW(());

        /** retrieves the UI names of all elements of all models in our document
            @param _eType
                the type of elements for which the names should be retrieved
            @param _rElementNames
                the array of element names
            @see getModelElementUIName
            @see getModelElementFromUIName
        */
        void    getAllElementUINames(
                    const ModelElementType _eType,
                    ::std::vector< ::rtl::OUString >& /* [out] */ _rElementNames,
                    bool _bPrepentEmptyEntry
                );

    protected:
        void    firePropertyChanges(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxOldProps,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxNewProps,
                    ::std::set< ::rtl::OUString >& _rFilter
                ) const;

    private:
        void switchBindingListening( bool _bDoListening );

        /// implementation for both <member>createBindingForFormModel</member> and <member>getOrCreateBindingForModel</member>
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            implGetOrCreateBinding( const ::rtl::OUString& _rTargetModel, const ::rtl::OUString& _rBindingName ) const SAL_THROW(());
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

