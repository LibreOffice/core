/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserview.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 14:40:35 $
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
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_
#define _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
// #95343# --------------------
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    class OPropertyEditor;

    //========================================================================
    //=
    //========================================================================
    class OPropertyBrowserView : public Window
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

        OPropertyEditor*        m_pPropBox;
        sal_uInt16              m_nActivePage;
        Link                    m_aPageActivationHandler;

    protected:
        virtual void Resize();
        virtual void GetFocus();
        virtual long Notify( NotifyEvent& _rNEvt );

    public:
        OPropertyBrowserView(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xORB,
                        Window* pParent, WinBits nBits = 0);

        virtual ~OPropertyBrowserView();

        OPropertyEditor&    getPropertyBox() { return *m_pPropBox; }

        // page handling
        sal_uInt16  getActivaPage() const { return m_nActivePage; }
        void        activatePage(sal_uInt16 _nPage);

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

        // #95343# ------------------
        ::com::sun::star::awt::Size getMinimumSize();

    protected:
        DECL_LINK(OnPageActivation, void*);
    };


//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_

