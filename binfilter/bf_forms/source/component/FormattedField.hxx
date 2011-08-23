/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FORMS_FORMATTEDFIELD_HXX_
#define _FORMS_FORMATTEDFIELD_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef FORMS_ERRORBROADCASTER_HXX
#include "errorbroadcaster.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace frm
{

    //==================================================================
    //= OFormattedModel
    //==================================================================

    class OFormattedModel
                    :public OEditBaseModel
                    ,public OErrorBroadcaster
                    ,public OPropertyChangeListener
                    ,public ::comphelper::OAggregationArrayUsageHelper< OFormattedModel >
    {
        OPropertyChangeMultiplexer* 		m_pPropertyMultiplexer;

        // das Original, falls ich die Format-Properties meines aggregierten Models gefaket, d.h. von dem Feld, an das
        // ich gebunden bin, weitergereicht habe (nur gueltig wenn loaded)
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>	m_xOriginalFormatter;
        ::com::sun::star::util::Date		m_aNullDate;
        ::com::sun::star::uno::Any			m_aSaveValue;

        sal_Int32							m_nFieldType;
        sal_Int16							m_nKeyType;
        sal_Bool							m_bOriginalNumeric		: 1,
                                            m_bNumeric				: 1,	// analog fuer TreatAsNumeric-Property
                                            m_bAggregateListening	: 1;

        static ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >	s_xDefaultFormatter;
        static sal_Int32						nValueHandle;
            // falls ich wirklich mal einen selber benutzen muss, wird der zwischen allen Instanzen geteilt

    protected:
        virtual void _onValueChanged();

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcDefaultFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormFormatsSupplier() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier>  calcFormatsSupplier() const;
        DECLARE_DEFAULT_LEAF_XTOR( OFormattedModel );

        friend class OFormattedFieldWrapper;

        /// starts multiplexing the aggregate's property changes
        void startAggregateListening();
        /// stops multiplexing the aggregate's property changes
        void stopAggregateListening();
        /// release the aggregate listener
        void releaseAggregateListener();

    protected:
    // XInterface
        DECLARE_UNO3_AGG_DEFAULTS( OFormattedModel, OEditBaseModel );

    // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    // XAggregation
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
        virtual void SAL_CALL disposing();

    // XServiceInfo
        IMPLEMENTATION_NAME(OFormattedModel);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // XBoundComponent
        virtual sal_Bool _commit();

    // XPersistObject
        virtual void SAL_CALL write(const ::com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL read(const ::com::sun::star::uno::Reference<stario::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);

    // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                              sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                            throw(::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    // XLoadListener
        virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent) throw ( ::com::sun::star::uno::RuntimeException);
        virtual void _loaded(const ::com::sun::star::lang::EventObject& rEvent);
        virtual void _unloaded();

    // XReset
        virtual void _reset( void );

    // XPropertyState
        void setPropertyToDefaultByHandle(sal_Int32 nHandle);
        ::com::sun::star::uno::Any getPropertyDefaultByHandle(sal_Int32 nHandle) const;

        void SAL_CALL setPropertyToDefault(const ::rtl::OUString& aPropertyName) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

        // OAggregationArrayUsageHelper
        virtual void fillProperties(
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
            ) const;
        IMPLEMENT_INFO_SERVICE()

    // XPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    protected:
        virtual sal_Int16 getPersistenceFlags() const;
            // as we have an own version handling for persistence

    protected:
        DECLARE_XCLONEABLE();

        void implConstruct();
    };

    //==================================================================
    //= OFormattedControl
    //==================================================================
    typedef ::cppu::ImplHelper1< ::com::sun::star::awt::XKeyListener> OFormattedControl_BASE;
    class OFormattedControl :	 public OBoundControl
                                ,public OFormattedControl_BASE
    {
        sal_uInt32				m_nKeyEvent;

    public:
        OFormattedControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
        virtual ~OFormattedControl();

        DECLARE_UNO3_AGG_DEFAULTS(OFormattedControl, OBoundControl);
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    // ::com::sun::star::lang::XServiceInfo
        IMPLEMENTATION_NAME(OFormattedControl);
        virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XKeyListener
        virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
        virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw ( ::com::sun::star::uno::RuntimeException);

    private:
        DECL_LINK( OnKeyPressed, void* );
    };

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_FORMATTEDFIELD_HXX_

