/*************************************************************************
 *
 *  $RCSfile: browserview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-19 14:08:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

//............................................................................
namespace pcr
{
//............................................................................

    class OPropertyBrowserController;
    class OPropertyEditor;

    //========================================================================
    //=
    //========================================================================
    class OPropertyBrowserView : public Window
    {
//      friend class OPropertyBrowserController;

        OPropertyBrowserController*     m_pActiveController;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xObject;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;

        OPropertyEditor*        m_pPropBox;
        sal_uInt16              m_nActivePage;
        Link                    m_aPageActivationHandler;

    protected:
        virtual void Resize();

    public:
        OPropertyBrowserView(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xORB,
                        Window* pParent, WinBits nBits = 0);
        OPropertyBrowserView(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xORB,
                        Window* pParent, const ResId& rId);

        virtual ~OPropertyBrowserView();

        OPropertyBrowserController* getActiveController() const
                { return m_pActiveController; }
        void                        setActiveController( OPropertyBrowserController* pController );

        OPropertyEditor*    getPropertyBox() { return m_pPropBox; }

        void    setObject(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject)
                { m_xObject = _rxObject; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& getObject() const
                { return m_xObject; }

        // page handling
        sal_uInt16  getActivaPage() const { return m_nActivePage; }
        void        activatePage(sal_uInt16 _nPage);

        void    setPageActivationHandler(const Link& _rHdl) { m_aPageActivationHandler = _rHdl; }
        Link    getPageActivationHandler() const { return m_aPageActivationHandler; }

    protected:
        DECL_LINK(OnPageActivation, void*);
    };


//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/12 11:26:24  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 11:01:16  fs
 ************************************************************************/

