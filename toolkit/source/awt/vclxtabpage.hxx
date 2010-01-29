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
