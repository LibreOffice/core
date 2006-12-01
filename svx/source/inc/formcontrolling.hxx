/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formcontrolling.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:27:57 $
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

#ifndef SVX_FORMCONTROLLING_HXX
#define SVX_FORMCONTROLLING_HXX

#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_RUNTIME_FEATURESTATE_HPP_
#include <com/sun/star/form/runtime/FeatureState.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_RUNTIME_XFORMOPERATIONS_HPP_
#include <com/sun/star/form/runtime/XFormOperations.hpp>
#endif

#include <cppuhelper/implbase1.hxx>
#include <comphelper/componentcontext.hxx>

#include <vector>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= FeatureSlotTranslation
    //====================================================================
    class FeatureSlotTranslation
    {
    public:
        /// retrieves the feature id for a given feature URL
        static  sal_Int32       getControllerFeatureSlotIdForURL( const ::rtl::OUString& _rMainURL );

        /// retrieves the feature URL for a given feature id
        static ::rtl::OUString  getControllerFeatureURLForSlotId( sal_Int32 _nSlotId );

        /// determines whether the given URL is a controller feature URL
        static sal_Bool         isFeatureURL( const ::rtl::OUString& _rMainURL );

        /// retrieves the css.form.runtime.FormFeature ID for a given slot ID
        static  sal_Int16       getFormFeatureForSlotId( sal_Int32 _nSlotId );

        /// retrieves the slot id for a given css.form.runtime.FormFeature ID
        static  sal_Int32       getSlotIdForFormFeature( sal_Int16 _nFormFeature );
    };

    //====================================================================
    //= IControllerFeatureInvalidation
    //====================================================================
    class IControllerFeatureInvalidation
    {
    public:
        /** invalidates the given features

            Invalidation means that any user interface representation (such as toolbox buttons), or
            any dispatches associated with the features in question are potentially out-of-date, and
            need to be updated

            @param _rFeatures
                Ids of the features to be invalidated.
        */
        virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures ) = 0;
    };

    //====================================================================
    //= ControllerFeatures
    //====================================================================
    class FormControllerHelper;
    /** easier access to an FormControllerHelper instance
    */
    class ControllerFeatures
    {
    protected:
        ::comphelper::ComponentContext  m_aContext;
        IControllerFeatureInvalidation* m_pInvalidationCallback;    // necessary as long as m_pImpl is not yet constructed
        FormControllerHelper*           m_pImpl;

    public:
        /** standard ctor

            The instance is not functional until <method>assign</method> is used.

            @param _rxORB
                a multi service factory for creating various needed components

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the instance from a <type scope="com::sun::star::form">XFormController<type> instance

            @param _rxORB
                a multi service factory for creating various needed components

            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the helper form a <type scope="com::sun::star::form">XForm<type> instance

            Any functionality which depends on a controller will not be available.

            @param _rxORB
                a multi service factory for creating various needed components

            @param _rxForm
                The form which the helper should be responsible for. Must not be <NULL/>.

            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        ControllerFeatures(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /// dtor
        ~ControllerFeatures();

        /// checks whether the instance is properly assigned to a form and/or controller
        inline bool isAssigned( ) const { return m_pImpl != NULL; }

        /** assign to a controller
        */
        void assign(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController
        );

        /** assign to a controller
        */
        void assign(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm
        );

        /// clears the instance so that it cannot be used afterwards
        void dispose();

        // access to the instance which implements the functionality. Not to be used when not assigned
        inline const FormControllerHelper* operator->() const { return m_pImpl; }
        inline       FormControllerHelper* operator->()       { return m_pImpl; }
        inline const FormControllerHelper& operator*() const  { return *m_pImpl; }
        inline       FormControllerHelper& operator*()        { return *m_pImpl; }
    };

    //====================================================================
    //= FormControllerHelper
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::form::runtime::XFeatureInvalidation
                                    >   FormControllerHelper_Base;
    /** is a helper class which manages form controller functionality (such as moveNext etc.).

        <p>The class helps implementing form controller functionality, by providing
        methods to determine the state of, and execute, various common form features.<br/>
        A <em>feature</em> is for instance moving the form associated with the controller
        to a certain position, or reloading the form, and so on.</p>
    */
    class FormControllerHelper : public FormControllerHelper_Base
    {
    protected:
        ::comphelper::ComponentContext  m_aContext;
        IControllerFeatureInvalidation* m_pInvalidationCallback;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormOperations >
                                        m_xFormOperations;

    public:
        /** constructs the helper from a <type scope="com::sun::star::form">XFormController<type> instance

            @param _rContext
                the context the component lives in
            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        /** constructs the helper form a <type scope="com::sun::star::form">XForm<type> instance

            Any functionality which depends on a controller will not be available.

            @param _rContext
                the context the component lives in
            @param _rxForm
                The form which the helper should be responsible for. Must not be <NULL/>.
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        // forwards to the XFormOperations implementation
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                    getCursor() const;
        void        getState(
                        sal_Int32 _nSlotId,
                        ::com::sun::star::form::runtime::FeatureState& _out_rState
                    ) const;
        sal_Bool    isEnabled( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId ) const;
        void        execute( sal_Int32 _nSlotId, const ::rtl::OUString& _rParamName, const ::com::sun::star::uno::Any& _rParamValue ) const;
        sal_Bool    commitCurrentRecord() const;
        sal_Bool    commitCurrentControl( ) const;
        sal_Bool    isInsertionRow() const;
        sal_Bool    isModifiedRow() const;

        bool        moveLeft( ) const;
        bool        moveRight( ) const;

        bool        canDoFormFilter() const;

        /** disposes this instance.

            After this method has been called, the instance is not functional anymore
        */
        void        dispose();

    protected:
        /// dtor
        ~FormControllerHelper();

        // XFeatureInvalidation
        virtual void SAL_CALL invalidateFeatures( const ::com::sun::star::uno::Sequence< ::sal_Int16 >& Features ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL invalidateAllFeatures() throw (::com::sun::star::uno::RuntimeException);

    private:
        FormControllerHelper();                                         // never implemented
        FormControllerHelper( const FormControllerHelper& );            // never implemented
        FormControllerHelper& operator=( const FormControllerHelper& ); // never implemented
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_FORMCONTROLLING_HXX
