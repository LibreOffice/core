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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

#include <svx/sdr/contact/viewobjectcontactofsdrobj.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>

class OutputDevice;
namespace vcl { class Window; }
class SdrUnoObj;
namespace com { namespace sun { namespace star {
    namespace awt {
        class XControl;
        class XControlContainer;
    }
} } }


namespace sdr { namespace contact {


    class ViewContactOfUnoControl;
    class ObjectContactOfPageView;

    //= ViewObjectContactOfUnoControl

    class ViewObjectContactOfUnoControl_Impl;
    class SVX_DLLPRIVATE ViewObjectContactOfUnoControl : public ViewObjectContactOfSdrObj
    {
    protected:
        ::rtl::Reference< ViewObjectContactOfUnoControl_Impl >    m_pImpl;

    public:
        ViewObjectContactOfUnoControl( ObjectContact& _rObjectContact, ViewContactOfUnoControl& _rViewContact );

        /// returns the ->XControl instance belonging to the instance, creates it if necessary
        css::uno::Reference< css::awt::XControl >
                getControl();

        /** retrieves a temporary XControl instance, whose parent is the given device
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        static css::uno::Reference< css::awt::XControl >
            getTemporaryControlForWindow(
                const vcl::Window& _rWindow,
                css::uno::Reference< css::awt::XControlContainer >& _inout_ControlContainer,
                const SdrUnoObj& _rUnoObject
            );

        /// ensures that the control belonging to this instances has a given visibility
        void    ensureControlVisibility( bool _bVisible ) const;

        /** sets the design/alive mode of the control
        */
        void    setControlDesignMode( bool _bDesignMode ) const;

        /** callback from impl class to react on changes of properties form the XControlModel
        */
        void propertyChange();

        /** React on changes of the object of this ViewContact
        */
        virtual void ActionChanged() override;

        /** to be called when any aspect of the control which requires view updates changed
        */
        struct ImplAccess { friend class ViewObjectContactOfUnoControl_Impl; friend class ViewObjectContactOfUnoControl; private: ImplAccess() { } };
        void onControlChangedOrModified( ImplAccess ) { impl_onControlChangedOrModified(); }

    protected:
        virtual ~ViewObjectContactOfUnoControl();

        // support for Primitive2D
        virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const override;

        // visibility check
        virtual bool isPrimitiveVisible( const DisplayInfo& _rDisplayInfo ) const override;
        /// to be called when any aspect of the control which requires view updates changed
        void impl_onControlChangedOrModified();

    private:
        ViewObjectContactOfUnoControl( const ViewObjectContactOfUnoControl& ) = delete;
        ViewObjectContactOfUnoControl& operator=( const ViewObjectContactOfUnoControl& ) = delete;
    };

    class SVX_DLLPRIVATE UnoControlPrintOrPreviewContact : public ViewObjectContactOfUnoControl
    {
    public:
        UnoControlPrintOrPreviewContact( ObjectContactOfPageView& _rObjectContact, ViewContactOfUnoControl& _rViewContact );
        virtual ~UnoControlPrintOrPreviewContact();

    private:
        UnoControlPrintOrPreviewContact( const UnoControlPrintOrPreviewContact& ) = delete;
        UnoControlPrintOrPreviewContact& operator=( const UnoControlPrintOrPreviewContact& ) = delete;

        virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo ) const override;
    };


} } // namespace sdr::contact


#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWOBJECTCONTACTOFUNOCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
