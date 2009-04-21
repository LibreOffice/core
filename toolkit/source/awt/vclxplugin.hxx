/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
 *    Foundation, Inc., 51 Franklin Street, 5th Floor, Boston,
 *    MA  02110-1301  USA
 *
 ************************************************************************/

#ifndef LAYOUT_AWT_VCLXPLUGIN_HXX
#define LAYOUT_AWT_VCLXPLUGIN_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/wintypes.hxx>

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
