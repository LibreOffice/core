/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eformshelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:21:25 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
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
        PropertyChangeListeners
                        m_aPropertyListeners;
        MapStringToPropertySet
                        m_aSubmissionUINames;   // only filled upon request
        MapStringToPropertySet
                        m_aBindingUINames;      // only filled upon request

    public:
        EFormsHelper(
            ::osl::Mutex& _rMutex,
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
        void    revokeBindingListener(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxBindingListener
                );

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

        /** fires a change in a single property, if the property value changed, and if we have a listener
            interested in property changes
        */
        void    firePropertyChange(
                    const ::rtl::OUString& _rName,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                ) const;

    private:
        void impl_switchBindingListening_throw( bool _bDoListening, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );

        /// implementation for both <member>createBindingForFormModel</member> and <member>getOrCreateBindingForModel</member>
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
            implGetOrCreateBinding( const ::rtl::OUString& _rTargetModel, const ::rtl::OUString& _rBindingName ) const SAL_THROW(());

        void
            impl_toggleBindingPropertyListening_throw( bool _bDoListen, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxConcreteListenerOrNull );

    private:
        EFormsHelper();                                 // never implemented
        EFormsHelper( const EFormsHelper& );            // never implemented
        EFormsHelper& operator=( const EFormsHelper& ); // never implemented
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX

