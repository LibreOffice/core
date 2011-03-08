/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef LAYOUT_AWT_VCLXTABPAGE_HXX
#define LAYOUT_AWT_VCLXTABPAGE_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <layout/core/bin.hxx>
#include <comphelper/uno3.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;

class VCLXTabPage : public VCLXWindow
                  , public Bin
{
    bool bRealized;

public:
    VCLXTabPage( Window *p );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

protected:
    ~VCLXTabPage();

    // XComponent
    void SAL_CALL dispose() throw(css::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutContainer
    virtual void SAL_CALL allocateArea( css::awt::Rectangle const& rArea )
        throw (css::uno::RuntimeException);
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException);

private:
    VCLXTabPage( VCLXTabPage const & );
    VCLXTabPage& operator=( VCLXTabPage const & );
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXTABPAGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
