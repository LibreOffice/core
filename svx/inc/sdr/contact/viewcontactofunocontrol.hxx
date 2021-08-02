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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/svdouno.hxx>

class OutputDevice;
namespace vcl { class Window; }
namespace com::sun::star {
    namespace awt {
        class XControl;
        class XControlContainer;
    }
}


namespace sdr::contact {


    //= ViewContactOfUnoControl

    class ViewContactOfUnoControl final : public ViewContactOfSdrObj
    {
    public:
        // access to SdrObject
        const SdrUnoObj& GetSdrUnoObj() const
        {
            return static_cast<const SdrUnoObj&>(GetSdrObject());
        }

        explicit ViewContactOfUnoControl( SdrUnoObj& _rUnoObject );
        virtual ~ViewContactOfUnoControl() override;

        /** retrieves a temporary XControl instance, whose parent is the given window
            @seealso SdrUnoObj::GetTemporaryControlForWindow
        */
        css::uno::Reference< css::awt::XControl >
            getTemporaryControlForWindow( const vcl::Window& _rWindow, css::uno::Reference< css::awt::XControlContainer >& _inout_ControlContainer ) const;

    private:
        virtual ViewObjectContact& CreateObjectSpecificViewObjectContact( ObjectContact& _rObjectContact ) override;

        ViewContactOfUnoControl( const ViewContactOfUnoControl& ) = delete;
        ViewContactOfUnoControl& operator=( const ViewContactOfUnoControl& ) = delete;

        // This method is responsible for creating the graphical visualisation data
        // ONLY based on model data
        virtual drawinglayer::primitive2d::Primitive2DContainer createViewIndependentPrimitive2DSequence() const override;
    };


} // namespace sdr::contact

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
