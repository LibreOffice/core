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
#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_GENERICSTATUSBARCONTROLLER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_GENERICSTATUSBARCONTROLLER_HXX

#include <svtools/statusbarcontroller.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>

namespace framework
{

struct AddonStatusbarItemData;

class GenericStatusbarController : public svt::StatusbarController
{
    public:
        GenericStatusbarController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                                    const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                                    const com::sun::star::uno::Reference< com::sun::star::ui::XStatusbarItem >& rxItem,
                                    AddonStatusbarItemData *pItemData );
        virtual ~GenericStatusbarController();

        // XComponent
        virtual void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;
        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
        bool m_bEnabled;
        bool m_bOwnerDraw;
        AddonStatusbarItemData *m_pItemData;
        ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xGraphic;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
