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

#ifndef INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX
#define INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/svxdllapi.h>

#include <memory>

class OutputDevice;
namespace vcl { class Window; }
namespace com { namespace sun { namespace star {
    namespace awt {
        class XControl;
        class XControlContainer;
    }
} } }


namespace sdr { namespace contact {



    //= ViewContactOfUnoControl

    class ViewContactOfUnoControl_Impl;
    class SVX_DLLPRIVATE ViewContactOfUnoControl : public ViewContactOfSdrObj
    {
    private:
        ::std::unique_ptr< ViewContactOfUnoControl_Impl >   m_pImpl;

    public:
        // access to SdrObject
        const SdrUnoObj& GetSdrUnoObj() const
        {
            return static_cast<const SdrUnoObj&>(GetSdrObject());
        }

        explicit ViewContactOfUnoControl( SdrUnoObj& _rUnoObject );
        virtual ~ViewContactOfUnoControl();

        /** access control to selected members
        */
        struct SdrUnoObjAccessControl { friend class ::SdrUnoObj; private: SdrUnoObjAccessControl() { } };

        /** retrieves a temporary XControl instance, whose parent is the given window
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
            getTemporaryControlForWindow( const vcl::Window& _rWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& _inout_ControlContainer ) const;

    protected:
        virtual ViewObjectContact& CreateObjectSpecificViewObjectContact( ObjectContact& _rObjectContact ) override;

    private:
        ViewContactOfUnoControl( const ViewContactOfUnoControl& ) = delete;
        ViewContactOfUnoControl& operator=( const ViewContactOfUnoControl& ) = delete;

    protected:
        // This method is responsible for creating the graphical visualisation data
        // ONLY based on model data
        virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const override;
    };


} } // namespace sdr::contact


#endif // INCLUDED_SVX_INC_SDR_CONTACT_VIEWCONTACTOFUNOCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
