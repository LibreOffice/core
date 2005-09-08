/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: submissionhandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:30:15 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#include "propertyhandler.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#include "eformshelper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_XFORMS_XSUBMISSION_HPP_
#include <com/sun/star/xforms/XSubmission.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= SubmissionHelper
    //====================================================================
    class SubmissionHelper : public EFormsHelper
    {
    public:
        SubmissionHelper(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
        );

        /** determines whether the given control model is able to trigger submissions

            Instances of the <type>SubmissionHelper</type> class should not be instantiated
            for components where this method returned <FALSE/>
        */
        static bool canTriggerSubmissions(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
                ) SAL_THROW(());
    };

    //====================================================================
    //= SubmissionPropertyHandler
    //====================================================================
    class SubmissionPropertyHandler : public PropertyHandler, public ::comphelper::OPropertyChangeListener
    {
    private:
        ::osl::Mutex                        m_aMutex;
        ::std::auto_ptr< SubmissionHelper > m_pHelper;

    public:
        // HandlerFactory
        inline static IPropertyHandler* Create(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        )
        {
            return new SubmissionPropertyHandler( _rxIntrospectee, _rxContextDocument, _rxTypeConverter );
        }

    protected:
        SubmissionPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
            const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        );
        ~SubmissionPropertyHandler();

    protected:
        // IPropertyHandler overriables
        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValue( PropertyId _nPropId, bool _bLazy = true ) const;
        virtual void                        SAL_CALL setPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue );

        virtual ::std::vector< ::rtl::OUString >
                                            SAL_CALL getActuatingProperties( ) const;
        virtual ::std::vector< ::rtl::OUString >
                                            SAL_CALL getSupersededProperties( ) const;

        virtual void                        SAL_CALL describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const;
        virtual void                        SAL_CALL initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater );
        virtual bool                        SAL_CALL supportsUIDescriptor( PropertyId _nPropId ) const;
        virtual void                        SAL_CALL actuatingPropertyChanged( PropertyId _nActuatingPropId, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, IPropertyBrowserUI* _pUpdater, bool );

        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const;
        virtual ::rtl::OUString             SAL_CALL getStringRepFromPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) const;

        // PropertyHandler overridables
        virtual ::std::vector< ::com::sun::star::beans::Property >
                                            SAL_CALL implDescribeSupportedProperties() const;
    private:
        // OPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX

