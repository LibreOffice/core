/*************************************************************************
 *
 *  $RCSfile: formcontrolling.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-05 12:23:41 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef SVX_FORMCONTROLLING_HXX
#define SVX_FORMCONTROLLING_HXX

#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

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
        static  sal_Int32       getControllerFeatureIdForURL( const ::rtl::OUString& _rMainURL );

        /// retrieves the feature URL for a given feature id
        static ::rtl::OUString  getControllerFeatureURLForId( sal_Int32 _nId );

        // determines whether the given URL is a controller feature URL
        static sal_Bool         isFeatureURL( const ::rtl::OUString& _rMainURL );
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
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        m_xORB;
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
    //= ControllerFeatureState
    //====================================================================
    struct ControllerFeatureState
    {
        sal_Bool                    bEnabled;
        ::com::sun::star::uno::Any  aState;

        inline ControllerFeatureState() : bEnabled( sal_False ) { }
    };

    //====================================================================
    //= FormControllerHelper
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::beans::XPropertyChangeListener
                                    ,   ::com::sun::star::util::XModifyListener
                                    ,   ::com::sun::star::sdbc::XRowSetListener
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
        mutable
        ::osl::Mutex    m_aMutex;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >
                    m_xController;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                    m_xCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >
                    m_xUpdateCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xCursorProperties;
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >
                    m_xLoadableForm;

        mutable
        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSingleSelectQueryComposer >
                    m_xParser;

        sal_Bool    m_bInitializedParser     : 1;
        sal_Bool    m_bActiveControlModified : 1;

        ::svxform::OStaticDataAccessTools*
                    m_pDbTools;
        IControllerFeatureInvalidation*
                    m_pInvalidationCallback;

    public:
        /** constructs the helper from a <type scope="com::sun::star::form">XFormController<type> instance

            @param _rxORB
                a multi service factory for creating various needed components
            @param _rxController
                The form controller which the helper should be responsible for. Must not
                be <NULL/>, and must have a valid model (form).
            @param _pInvalidationCallback
                the callback for invalidating feature states
        */
        FormControllerHelper(
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
        FormControllerHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
            IControllerFeatureInvalidation* _pInvalidationCallback
        );

        // attribute access
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >&
                    getCursor() const { return m_xCursor; }
        inline const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >&
                    getUpdateCursor() const { return m_xUpdateCursor; }

        /** gets the state of a given feature
            @param _nFeatureId
                the id of the feature in question. Must be one of the SID_FM_* slots
            @param _rState
                is the output parameter for the state
        */
        void        getState(
                        sal_Int32 _nFeatureId,
                        ControllerFeatureState& /* [out] */ _rState
                    ) const;

        /** gets the state of a given feature, caring only whether or not
            it's enabled
            @param _nFeatureId
                the id of the feature in question. Must be one of the SID_FM_* slots
        */
        sal_Bool    getSimpleState( sal_Int32 _nFeatureId ) const;

        /** determines whether our cursor can be moved right
            @precond hasCursor()
        */
        sal_Bool    canMoveRight( ) const;

        /** determines whether our cursor can be moved left
            @precond hasCursor()
        */
        sal_Bool    canMoveLeft( ) const;

        /** executes the given feature
        */
        void        execute( sal_Int32 _nFeatureId ) const;

        /** executes the given feature, carign for a named parameter for the execution
        */
        void        execute( sal_Int32 _nFeatureId, const ::rtl::OUString& _rParamName,
                        const ::com::sun::star::uno::Any& _rParamValue ) const;

        /** moves our cursor one position to the left, caring for different possible
            cursor states.

            <p>Before the movement is done, the current row is saved, if necessary.</p>

            @precond
                canMoveLeft()
        */
        sal_Bool    moveLeft( ) const;

        /** moves our cursor one position to the right, caring for different possible
            cursor states.

            <p>Before the movement is done, the current row is saved, if necessary.</p>

            @precond
                canMoveRight()
        */
        sal_Bool    moveRight( ) const;

        /** commits the current record of our form if necessary

            @precond
                hasCursor()
            @param _pRecordInserted
                 íf not <NULL/>, the boolean pointed to by this arg will be <TRUE/> if and only
                 if a new record has been inserted.
        */
        sal_Bool    commitCurrentRecord( sal_Bool* _pRecordInserted = NULL ) const;

        /** commits the current control of our controller
            @precond
                we have a valid controller
        */
        sal_Bool    commitCurrentControl( ) const;

        // retrieving various information about the curren record
        sal_Bool    isNewRecord() const;
        sal_Bool    isModifiedRecord() const;
        sal_Int32   getRecordCount() const;
        sal_Bool    isRecordCountFinal() const;
        sal_Bool    isInsertOnlyForm() const;

        /// determines whether we can parse the query of our form
        sal_Bool    isParsable() const;

        /** resets all control models in the given form
        */
        static void resetAllControls( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm );

        /** disposes this instance.

            After this method has been called, the instance is not functional anymore
        */
        void        dispose();

    protected:
        /// dtor
        ~FormControllerHelper();

        // XRowSetListener
        virtual void SAL_CALL cursorMoved( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowSetChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& _rSource ) throw( ::com::sun::star::uno::RuntimeException );

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** resets all control models in our own form
        */
        void        resetAllControls( ) const;

        /** initializes the cursor-related members
            <p>Only to be called from within a constructor.</p>
        */
        void        initCursor(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm
        );

        /** initializes the controller-related members
            <p>Only to be called from within a constructor.</p>
        */
        void        initController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController
        );

        /** retrieces the column to which the current control of our controller is bound
            @precond
                m_xController.is()
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    getCurrentBoundField( ) const;

        /** determines if we have a valid cursor, including the property set and the XLoadable
            interface
        */
        inline  sal_Bool    hasCursor() const { return m_xCursorProperties.is(); }

        /** determines if we have a filter in place
        */
        sal_Bool    hasFilterOrOrder() const;

        /// ensures that our parse is initialized, or at least that we attempted to do so
        void        ensureInitializedParser();

        /** disposes our parser, if we have one
        */
        void        disposeParser();

        /** executes the "auto sort ascending" and "auto sort descending" features
        */
        void        executeAutoSort( sal_Bool _bUp ) const;

        /** executes the "auto filter" feature
        */
        void        executeAutoFilter( ) const;

        /** executes the "sort..." feature
        */
        void        executeFilterOrSort( bool _bFilter ) const;

        /// typedef for member method of this class
        typedef void (FormControllerHelper::*Action)( const void* ) const;

        /** calls a member function, with reporting an error - in case it happens - to the user

            @param _pAction
                the member function to call
            @param _pParam
                the parameters to pass to the member function
            @param _nErrorResourceId
                the id of the resources string to use as error message
            @return
                <TRUE/> if and only if the call was successfull (i.e. no error occured)
        */
        sal_Bool    doActionReportError( Action _pAction, const void* _pParam, sal_uInt16 _nErrorResourceId ) const;

        // parameter structure for appendOrderByColumn
        struct param_appendOrderByColumn
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        xField;
            sal_Bool    bUp;
        };
        void        appendOrderByColumn( const void* _pActionParam ) const;

        // parameter structure for appendFilterByColumn
        struct param_appendFilterByColumn
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        xField;
        };
        void        appendFilterByColumn( const void* _pActionParam ) const;

        /// invalidate the full palette of features which we know
        void        invalidateAllSupportedFeatures( ::osl::ClearableMutexGuard& _rClearForCallback ) const;
        /** invalidate the features which depend on the "modified" state of the current control
            of our controller
        */
        void        invalidateModifyDependentFeatures( ::osl::ClearableMutexGuard& _rClearForCallback ) const;

    private:
        FormControllerHelper();                                         // never implemented
        FormControllerHelper( const FormControllerHelper& );            // never implemented
        FormControllerHelper& operator=( const FormControllerHelper& ); // never implemented
    };

//........................................................................
}   // namespace svx
//........................................................................

#endif // SVX_FORMCONTROLLING_HXX
