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

#ifndef INCLUDED_SVX_SVDOUNO_HXX
#define INCLUDED_SVX_SVDOUNO_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <svx/svxdllapi.h>
#include <svx/svdorect.hxx>
#include <memory>


// Forward declaration
class SdrView;
class SdrPageWindow;
class SdrControlEventListenerImpl;

namespace sdr::contact {
    class ViewContactOfUnoControl;
}


// SdrUnoObj
struct SdrUnoObjDataHolder;
class SVXCORE_DLLPUBLIC SdrUnoObj : public SdrRectObj
{
    friend class                SdrPageView;
    friend class                SdrControlEventListenerImpl;

    std::unique_ptr<SdrUnoObjDataHolder>        m_pImpl;

    OUString                    aUnoControlModelTypeName;
    OUString                    aUnoControlTypeName;

protected:
    css::uno::Reference< css::awt::XControlModel > xUnoControlModel; // Can also be set from outside

private:
    SVX_DLLPRIVATE void CreateUnoControlModel(const OUString& rModelName);
    SVX_DLLPRIVATE void CreateUnoControlModel(const OUString& rModelName,
        const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSFac );

protected:
    // protected destructor
    virtual ~SdrUnoObj() override;

public:
    explicit SdrUnoObj(
        SdrModel& rSdrModel,
        const OUString& rModelName);
    // Copy constructor
    SdrUnoObj(SdrModel& rSdrModel, SdrUnoObj const & rSource);
    SdrUnoObj(
        SdrModel& rSdrModel,
        const OUString& rModelName,
        const css::uno::Reference< css::lang::XMultiServiceFactory >& rxSFac);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrObjKind GetObjIdentifier() const override;

    virtual SdrUnoObj* CloneSdrObject(SdrModel& rTargetModel) const override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcSetLayer(SdrLayerID nLayer) override;

    // SpecialDrag support
    virtual bool hasSpecialDrag() const override;

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual void SetContextWritingMode( const sal_Int16 _nContextWritingMode ) override;

    const css::uno::Reference< css::awt::XControlModel >& GetUnoControlModel() const {return xUnoControlModel;}
    css::uno::Reference< css::awt::XControl > GetUnoControl(const SdrView& _rView, const OutputDevice& _rOut) const;

    /** Retrieves a temporary XControl instance for a given output device

        The method GetUnoControl, used to retrieve the XControl whose parent is a given device, only works
        if the SdrUnoObj has already been painted at least once onto this device. However, there are valid
        scenarios where you need certain information on how a control is painted onto a window, without
        actually painting it. For example, you might be interested in the DeviceInfo of a UNO control.

        For those cases, you can contain an XControl which behaves as the control which *would* be used to
        paint onto a window.

        @param  _rWindow
            the window for which should act as parent for the temporary control
        @param  _inout_ControlContainer
            the control container belonging to the window, necessary as context
            for the newly created control. A control container is usually created by calling
            VCLUnoHelper::CreateControlContainer.
            If _inout_ControlContainer is <NULL/>, it will be created internally, and passed to the caller.
            In this case, the caller also takes ownership of the control container, and is responsible for
            disposing it when not needed anymore.
        @return
            The requested control. This control is temporary only, and the caller is responsible for it.
            In particular, the caller is required to dispose it when it's not needed anymore.

    */
    css::uno::Reference< css::awt::XControl >
        GetTemporaryControlForWindow(
            const vcl::Window& _rWindow,
            css::uno::Reference< css::awt::XControlContainer >& _inout_ControlContainer
        ) const;

    const OUString& GetUnoControlTypeName() const { return aUnoControlTypeName; }
    const OUString& getUnoControlModelTypeName() const { return aUnoControlModelTypeName; }

    virtual void SetUnoControlModel( const css::uno::Reference< css::awt::XControlModel >& xModel );

protected:
    // SdrObject overridables
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

private:
    /** Retrieves the typed ViewContact for the object
        @param _out_rpContact
            Upon successful return, ->_out_rpContact contains the ViewContact.
        @return
            <TRUE/> if and only if the contact was successfully retrieved. In this case,
            ->_out_rpContact contains a pointer to this contact.
            A failure to retrieve the contact object fires an assertion in non-product builds.
    */
    SVX_DLLPRIVATE  bool    impl_getViewContact( sdr::contact::ViewContactOfUnoControl*& _out_rpContact ) const;
};

#endif // INCLUDED_SVX_SVDOUNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
