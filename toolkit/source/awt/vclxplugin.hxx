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

#ifndef LAYOUT_AWT_VCLXPLUGIN_HXX
#define LAYOUT_AWT_VCLXPLUGIN_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <tools/wintypes.hxx>

class Control;
namespace layoutimpl
{

namespace css = ::com::sun::star;

class VCLXPlugin : public VCLXWindow
{
public:
    Window *mpWindow;
    ::Control *mpPlugin;
    WinBits mStyle;

    VCLXPlugin( Window *p, WinBits b );

    void SetPlugin( ::Control *p );

protected:
    ~VCLXPlugin();

    // XComponent
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize()
        throw(::com::sun::star::uno::RuntimeException);

private:
    VCLXPlugin( VCLXPlugin const & );
    VCLXPlugin& operator=( VCLXPlugin const & );
};

} // namespace layoutimpl

#endif /* LAYOUT_AWT_VCLXPLUGIN_HXX */
