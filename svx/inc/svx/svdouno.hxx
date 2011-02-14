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

#ifndef _SVDOUNO_HXX
#define _SVDOUNO_HXX

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#ifndef _COM_SUN_STAR_AWT_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/awt/XControlContainer.hpp>
#include "svx/svxdllapi.h"
#include <svx/svdorect.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrView;
class SdrPageWindow;
class SdrControlEventListenerImpl;

namespace sdr { namespace contact {
    class ViewContactOfUnoControl;
}}

//************************************************************
//   SdrUnoObj
//************************************************************

struct SdrUnoObjDataHolder;
class SVX_DLLPUBLIC SdrUnoObj : public SdrRectObj
{
    friend class                SdrPageView;
    friend class                SdrControlEventListenerImpl;

    SdrUnoObjDataHolder*        m_pImpl;

    String                      aUnoControlModelTypeName;
    String                      aUnoControlTypeName;
    sal_Bool                        bOwnUnoControlModel;

protected:
    ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > xUnoControlModel; // kann auch von aussen gesetzt werden

private:
    SVX_DLLPRIVATE void CreateUnoControlModel(const String& rModelName);
    SVX_DLLPRIVATE void CreateUnoControlModel(const String& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac );

public:
    TYPEINFO();

    SdrUnoObj(const String& rModelName, sal_Bool bOwnsModel = sal_True);
    SdrUnoObj(const String& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac,
        sal_Bool bOwnsModel = sal_True);
    virtual ~SdrUnoObj();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pModel);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual void operator = (const SdrObject& rObj);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetLayer(SdrLayerID nLayer);

    // SpecialDrag support
    virtual bool hasSpecialDrag() const;

    // FullDrag support
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;

    virtual void TakeObjNameSingul(XubString& rName) const;
    virtual void TakeObjNamePlural(XubString& rName) const;

    virtual void SetContextWritingMode( const sal_Int16 _nContextWritingMode );

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
            const Window& _rWindow,
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer
        ) const;

    const String& GetUnoControlModelTypeName() const { return aUnoControlTypeName; }
    const String& GetUnoControlTypeName() const { return aUnoControlTypeName; }

    virtual void SetUnoControlModel( const ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel >& xModel );

protected:
    // SdrObject overridables
    virtual ::sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
    /** retrieves the typed ViewContact for the object
        @param _out_rpContact
            Upon successfull return, ->_out_rpContact contains the ViewContact.
        @return
            <TRUE/> if and only if the contact was successfully retrieved. In this case,
            ->_out_rpContact contains a pointer to this contact.
            A failure to retrieve the contact object fires an assertion in non-product builds.
    */
    SVX_DLLPRIVATE  bool    impl_getViewContact( ::sdr::contact::ViewContactOfUnoControl*& _out_rpContact ) const;
};


#endif          // _SVDOUNO_HXX

