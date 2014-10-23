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


//   Vorausdeklarationen


class SdrView;
class SdrPageWindow;
class SdrControlEventListenerImpl;

namespace sdr { namespace contact {
    class ViewContactOfUnoControl;
}}


//   SdrUnoObj


struct SdrUnoObjDataHolder;
class SVX_DLLPUBLIC SdrUnoObj : public SdrRectObj
{
    friend class                SdrPageView;
    friend class                SdrControlEventListenerImpl;

    SdrUnoObjDataHolder*        m_pImpl;

    OUString                    aUnoControlModelTypeName;
    OUString                    aUnoControlTypeName;
    bool                        bOwnUnoControlModel;

protected:
    ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > xUnoControlModel; // kann auch von aussen gesetzt werden

private:
    SVX_DLLPRIVATE void CreateUnoControlModel(const OUString& rModelName);
    SVX_DLLPRIVATE void CreateUnoControlModel(const OUString& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac );

public:
    TYPEINFO_OVERRIDE();

    explicit SdrUnoObj(const OUString& rModelName, bool bOwnsModel = true);
    SdrUnoObj(const OUString& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac,
        bool bOwnsModel = true);
    virtual ~SdrUnoObj();

    virtual void SetPage(SdrPage* pNewPage) SAL_OVERRIDE;
    virtual void SetModel(SdrModel* pModel) SAL_OVERRIDE;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier() const SAL_OVERRIDE;

    virtual SdrUnoObj* Clone() const SAL_OVERRIDE;
    SdrUnoObj& operator= (const SdrUnoObj& rObj);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
    virtual void NbcSetLayer(SdrLayerID nLayer) SAL_OVERRIDE;

    // SpecialDrag support
    virtual bool hasSpecialDrag() const SAL_OVERRIDE;

    // FullDrag support
    virtual bool supportsFullDrag() const SAL_OVERRIDE;
    virtual SdrObject* getFullDragClone() const SAL_OVERRIDE;

    virtual OUString TakeObjNameSingul() const SAL_OVERRIDE;
    virtual OUString TakeObjNamePlural() const SAL_OVERRIDE;

    virtual void SetContextWritingMode( const sal_Int16 _nContextWritingMode ) SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > GetUnoControlModel() const {return xUnoControlModel;}
    ::com::sun::star::uno::Reference< com::sun::star::awt::XControl > GetUnoControl(const SdrView& _rView, const OutputDevice& _rOut) const;

    /** retrieves a temporary XControl instance for a given output device

        The method GetUnoControl, used to retrieve the XControl whose parent is a given device, only works
        if the SdrUnoObj has already been painted at least once onto this device. However, there are valid
        scenarios where you need certain information on how a control is painted onto a window, without
        actually painting it. For example, you might be interested in the DeviceInfo of an UNO control.

        For those cases, you can ontain an XControl which behaves as the control which *would* be used to
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
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
        GetTemporaryControlForWindow(
            const vcl::Window& _rWindow,
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer
        ) const;

    const OUString& GetUnoControlModelTypeName() const { return aUnoControlTypeName; }
    const OUString& GetUnoControlTypeName() const { return aUnoControlTypeName; }

    virtual void SetUnoControlModel( const ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& xModel );

protected:
    // SdrObject overridables
    virtual ::sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

private:
    /** retrieves the typed ViewContact for the object
        @param _out_rpContact
            Upon successful return, ->_out_rpContact contains the ViewContact.
        @return
            <TRUE/> if and only if the contact was successfully retrieved. In this case,
            ->_out_rpContact contains a pointer to this contact.
            A failure to retrieve the contact object fires an assertion in non-product builds.
    */
    SVX_DLLPRIVATE  bool    impl_getViewContact( ::sdr::contact::ViewContactOfUnoControl*& _out_rpContact ) const;
};


#endif // INCLUDED_SVX_SVDOUNO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
