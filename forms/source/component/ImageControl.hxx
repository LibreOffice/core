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
#include <cppuhelper/implbase.hxx>

using namespace comphelper;


namespace frm
{



// OImageControlModel

typedef ::cppu::ImplHelper  <   css::form::XImageProducerSupplier
                            ,   css::awt::XImageProducer
                            >   OImageControlModel_Base;

class OImageControlModel
                :public OImageControlModel_Base
                ,public OBoundControlModel
{
    css::uno::Reference< css::awt::XImageProducer>    m_xImageProducer;
    ImageProducer*                                    m_pImageProducer;
    bool                                              m_bExternalGraphic;
    bool                                              m_bReadOnly;
    OUString                                          m_sImageURL;
    css::uno::Reference< css::graphic::XGraphicObject >
                                                      m_xGraphicObject;
    OUString                                          m_sDocumentURL;

protected:
    // UNO Anbindung
    virtual css::uno::Sequence< css::uno::Type> _getTypes() override;

    inline ImageProducer* GetImageProducer() { return m_pImageProducer; }

public:
    DECLARE_DEFAULT_LEAF_XTOR( OImageControlModel );

    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw ( css::uno::Exception, std::exception) override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue )
        throw(css::lang::IllegalArgumentException) override;

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlModel, OBoundControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OImageControlModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // XImageProducerSupplier
    virtual css::uno::Reference< css::awt::XImageProducer> SAL_CALL getImageProducer() throw ( css::uno::RuntimeException, std::exception) override;

    // XImageProducer
    virtual void SAL_CALL addConsumer( const css::uno::Reference< css::awt::XImageConsumer >& xConsumer ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeConsumer( const css::uno::Reference< css::awt::XImageConsumer >& xConsumer ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startProduction(  ) throw (css::uno::RuntimeException, std::exception) override;

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const override;
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

    // prevent method hiding
    using OBoundControlModel::disposing;
    using OBoundControlModel::getFastPropertyValue;

protected:
    // OBoundControlModel overridables
    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) override;
    virtual void            onDisconnectedDbColumn() override;
    virtual css::uno::Any   translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;

    virtual css::uno::Any   getControlValue( ) const override;
    virtual void            doSetControlValue( const css::uno::Any& _rValue ) override;

    virtual bool            approveDbColumnType(sal_Int32 _nColumnType) override;

    virtual void            resetNoBroadcast() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

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

typedef ::cppu::ImplHelper  <   css::awt::XMouseListener
                            ,   css::util::XModifyBroadcaster
                            >   OImageControlControl_Base;
class OImageControlControl  : public OBoundControl
                            , public OImageControlControl_Base
{
private:
    ::cppu::OInterfaceContainerHelper   m_aModifyListeners;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type> _getTypes() override;

public:
    explicit OImageControlControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OImageControlControl, OBoundControl )
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw(css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OImageControlControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) override;

    // XMouseListener
    virtual void SAL_CALL mousePressed(const css::awt::MouseEvent& e) throw ( css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseReleased(const css::awt::MouseEvent& e) throw ( css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseEntered(const css::awt::MouseEvent& e) throw ( css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL mouseExited(const css::awt::MouseEvent& e) throw ( css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

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
