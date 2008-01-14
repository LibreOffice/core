/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertycomposer.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:00:06 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX
#include "composeduiupdate.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYHANDLER_HPP_
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

#include <vector>
#include <set>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyComposer
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::inspection::XPropertyHandler
                                                ,   ::com::sun::star::beans::XPropertyChangeListener
                                                >   PropertyComposer_Base;
    /** implements an <type>XPropertyHandler</type> which composes it's information
        from a set of other property handlers
    */
    class PropertyComposer  :public PropertyComposer_Base
                            ,public ::comphelper::OBaseMutex
                            ,public IPropertyExistenceCheck
    {
    public:
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler > >
                                                            HandlerArray;

    private:
        HandlerArray                                    m_aSlaveHandlers;
        ::std::auto_ptr< ComposedPropertyUIUpdate >     m_pUIRequestComposer;
        PropertyChangeListeners                         m_aPropertyListeners;
        bool                                            m_bSupportedPropertiesAreKnown;
        PropertyBag                                     m_aSupportedProperties;

    public:
        /** constructs an <type>XPropertyHandler</type> which composes it's information from a set
            of other property handlers

            @param _rSlaveHandlers
                the set of slave handlers to invoke. Must not be <NULL/>
        */
        PropertyComposer( const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler > >& _rSlaveHandlers );

    public:
        // XPropertyHandler overridables
        virtual void                                SAL_CALL inspect( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIntrospectee ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any          SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rControlValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL convertToControlValue( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rPropertyValue, const ::com::sun::star::uno::Type& _rControlValueType ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::beans::PropertyState
                                                    SAL_CALL getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void                                SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                                SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >
                                                    SAL_CALL getSupportedProperties() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                                    SAL_CALL getSupersededProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                                                    SAL_CALL getActuatingProperties( ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::LineDescriptor
                                                    SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool                          SAL_CALL isComposable( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::inspection::InteractiveSelectionResult
                                                    SAL_CALL onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, ::com::sun::star::uno::Any& _rData, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                                SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool                            SAL_CALL suspend( sal_Bool _bSuspend ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        DECLARE_XCOMPONENT()
        virtual void                                SAL_CALL disposing();

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        // IPropertyExistenceCheck
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& _rName ) throw (::com::sun::star::uno::RuntimeException);

    private:
        /** ensures that m_pUIRequestComposer exists
        */
        void    impl_ensureUIRequestComposer( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorUI >& _rxInspectorUI );

        /** checks whether a given property exists in <member>m_aSupportedProperties</member>
        */
        bool    impl_isSupportedProperty_nothrow( const ::rtl::OUString& _rPropertyName )
        {
            ::com::sun::star::beans::Property aDummy; aDummy.Name = _rPropertyName;
            return m_aSupportedProperties.find( aDummy ) != m_aSupportedProperties.end();
        }

    private:
        class MethodGuard;
        friend class MethodGuard;
        class MethodGuard : public ::osl::MutexGuard
        {
        public:
            MethodGuard( PropertyComposer& _rInstance )
                : ::osl::MutexGuard( _rInstance.m_aMutex )
            {
                if ( _rInstance.m_aSlaveHandlers.empty() )
                    throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), *(&_rInstance) );
            }
        };
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX

