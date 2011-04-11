/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "precompiled_svtools.hxx"

#include "dummypanel.hxx"

//........................................................................
namespace svt
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;

    //====================================================================
    //= DummyPanel
    //====================================================================
    //--------------------------------------------------------------------
    DummyPanel::DummyPanel()
    {
    }

    //--------------------------------------------------------------------
    DummyPanel::~DummyPanel()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_IREFERENCE( DummyPanel )

    //--------------------------------------------------------------------
    void DummyPanel::Activate( Window& )
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::Deactivate()
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::SetSizePixel( const Size& )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString DummyPanel::GetDisplayName() const
    {
        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    Image DummyPanel::GetImage() const
    {
        return Image();
    }

    //--------------------------------------------------------------------
    rtl::OString DummyPanel::GetHelpID() const
    {
        return rtl::OString();
    }

    //--------------------------------------------------------------------
    void DummyPanel::GrabFocus()
    {
    }

    //--------------------------------------------------------------------
    void DummyPanel::Dispose()
    {
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > DummyPanel::CreatePanelAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        (void)i_rParentAccessible;
        return NULL;
    }

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
