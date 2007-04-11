/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdouno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:26:01 $
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

#ifndef _SVDOUNO_HXX
#define _SVDOUNO_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SdrView;
class SdrPageWindow;
class SdrControlEventListenerImpl;

namespace sdr { namespace contact {
    class ViewContactOfUnoControl;
} }

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
    BOOL                        bOwnUnoControlModel;

protected:
    ::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > xUnoControlModel; // kann auch von aussen gesetzt werden

private:
    SVX_DLLPRIVATE void CreateUnoControlModel(const String& rModelName);
    SVX_DLLPRIVATE void CreateUnoControlModel(const String& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac );

public:
    TYPEINFO();

    SdrUnoObj(const String& rModelName, BOOL bOwnsModel = TRUE);
    SdrUnoObj(const String& rModelName,
        const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSFac,
        BOOL bOwnsModel = TRUE);
    virtual ~SdrUnoObj();

    virtual void SetPage(SdrPage* pNewPage);
    virtual void SetModel(SdrModel* pModel);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual UINT16 GetObjIdentifier() const;
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;

    virtual void operator = (const SdrObject& rObj);
    virtual FASTBOOL HasSpecialDrag() const;

    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcSetLayer(SdrLayerID nLayer);

    virtual void TakeObjNameSingul(XubString& rName) const;
    virtual void TakeObjNamePlural(XubString& rName) const;

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

    void SetUnoControlModel(::com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > xModel);

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

