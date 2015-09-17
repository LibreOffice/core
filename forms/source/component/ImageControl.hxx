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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_IMAGECONTROL_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_IMAGECONTROL_HXX

#include "FormComponent.hxx"
#include "imgprod.hxx"
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/implbase2.hxx>

using namespace comphelper;


namespace frm
{



// OImageControlModel

typedef ::cppu::ImplHelper2 <   ::com::sun::star::form::XImageProducerSupplier
                            ,   ::com::sun::star::awt::XImageProducer
                            >   OImageControlModel_Base;

class OImageControlModel
                :public OImageControlModel_Base
                ,public OBoundControlModel
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer>    m_xImageProducer;
    ImageProducer*                                  m_pImageProducer;
    bool                                            m_bExternalGraphic;
    bool                                        m_bReadOnly;
    OUString                                 m_sImageURL;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject >
                                                    m_xGraphicObject;
    OUString                                 m_sDocumentURL;

protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

    inline ImageProducer* GetImageProducer() { return m_pImageProducer; }

public:
    DECLARE_DEFAULT_LEAF_XTOR( OImageControlModel );

    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlModel, OBoundControlModel)
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OImageControlModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XImageProducerSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer> SAL_CALL getImageProducer() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XImageProducer
    virtual void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

    // prevent method hiding
    using OBoundControlModel::disposing;
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
    virtual void            onDisconnectedDbColumn() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            getControlValue( ) const SAL_OVERRIDE;
    virtual void            doSetControlValue( const ::com::sun::star::uno::Any& _rValue ) SAL_OVERRIDE;

    virtual bool            approveDbColumnType(sal_Int32 _nColumnType) SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    void implConstruct();

    /** displays the image described by the given URL
        @precond
            our own mutex is locked
    */
    bool    impl_handleNewImageURL_lck( ValueChangeInstigator _eInstigator );

    /** updates the binary stream, created from loading the file which the given URL points to, into our
        bound field, or the control itself if there is no bound field
    */
    bool    impl_updateStreamForURL_lck( const OUString& _rURL, ValueChangeInstigator _eInstigator );

    DECL_LINK_TYPED( OnImageImportDone, ::Graphic*, void );
};

typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XMouseListener
                            ,   ::com::sun::star::util::XModifyBroadcaster
                            >   OImageControlControl_Base;
class OImageControlControl  : public OBoundControl
                            , public OImageControlControl_Base
{
private:
    ::cppu::OInterfaceContainerHelper   m_aModifyListeners;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    OImageControlControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OImageControlControl, OBoundControl )
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OImageControlControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // XMouseListener
    virtual void SAL_CALL mousePressed(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL mouseReleased(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL mouseEntered(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL mouseExited(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

private:
    void    implClearGraphics( bool _bForce );
    bool    implInsertGraphics();

    /** determines whether the control does currently have an empty graphic set
    */
    bool    impl_isEmptyGraphics_nothrow() const;
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_IMAGECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
