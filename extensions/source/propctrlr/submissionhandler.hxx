/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: submissionhandler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 08:01:27 $
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

namespace comphelper
{
    class OPropertyChangeMultiplexer;
}

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
            osl::Mutex& _rMutex,
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
    class SubmissionPropertyHandler;
    typedef HandlerComponentBase< SubmissionPropertyHandler > EditPropertyHandler_Base;
    /** a property handler for any virtual string properties
    */
    class SubmissionPropertyHandler : public EditPropertyHandler_Base, public ::comphelper::OPropertyChangeListener
    {
    private:
        ::osl::Mutex                                m_aMutex;
        ::std::auto_ptr< SubmissionHelper >         m_pHelper;
        ::comphelper::OPropertyChangeMultiplexer*   m_pPropChangeMultiplexer;

    public:
        SubmissionPropertyHandler(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext
        );

        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (::com::sun::star::uno::RuntimeException);

        ~SubmissionPropertyHandler();

    protected:
        // XPropertyHandler overriables
        virtual ::com::sun::star::uno::Any  SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                            SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                            SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::LineDescriptor
                                            SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any  SAL_CALL convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any  SAL_CALL convertToControlValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

        // PropertyHandler overridables
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                            SAL_CALL doDescribeSupportedProperties() const;
        virtual void onNewComponent();

    private:
        // OPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX

