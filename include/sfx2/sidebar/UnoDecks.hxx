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

#ifndef INCLUDED_SFX2_SIDEBAR_DECKS_HXX
#define INCLUDED_SFX2_SIDEBAR_DECKS_HXX


#include <com/sun/star/ui/XDecks.hpp>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>


#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>



using namespace css;

/** get the decks
*/
class SfxUnoDecks : public ::cppu::WeakImplHelper1< css::ui::XDecks >
{


public:

    SfxUnoDecks(const uno::Reference<frame::XFrame>&);
    virtual ~SfxUnoDecks() {};

// XNameAccess

    virtual uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(container::NoSuchElementException,
                                    lang::WrappedTargetException,
                                    uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XIndexAccess

    virtual sal_Int32 SAL_CALL getCount()
                                throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException,
                                  uno::RuntimeException, std::exception) SAL_OVERRIDE;

// XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:

    const uno::Reference<frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */