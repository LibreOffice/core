/*************************************************************************
 *
 *  $RCSfile: browserview.cxx,v $
 *
 *  $Revision: 1.3 $
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
#include "browserview.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#include "propertyeditor.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPHELPID_HRC_
#include "prophelpid.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#include "propcontroller.hxx"
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;


    //========================================================================
    //= class OPropertyBrowserView
    //========================================================================
    DBG_NAME(OPropertyBrowserView)
    //------------------------------------------------------------------------
    OPropertyBrowserView::OPropertyBrowserView( const Reference< XMultiServiceFactory >& _rxORB,
                                 Window* _pParent, WinBits nBits)
                  :Window(_pParent, nBits | WB_3DLOOK)
                  ,m_pActiveController(NULL)
                  ,m_xORB(_rxORB)
                  ,m_nActivePage(0)
    {
        DBG_CTOR(OPropertyBrowserView,NULL);

        m_pPropBox = new OPropertyEditor( this );
        m_pPropBox->SetHelpId(HID_FM_PROPDLG_TABCTR);
        m_pPropBox->setPageActivationHandler(LINK(this, OPropertyBrowserView, OnPageActivation));

        m_pPropBox->Show();
    }

    //------------------------------------------------------------------------
    OPropertyBrowserView::OPropertyBrowserView( const Reference< XMultiServiceFactory >& _rxORB,
                                 Window* _pParent, const ResId& rId)
                  :Window(_pParent, rId)
                  ,m_pActiveController(NULL)
                  ,m_xORB(_rxORB)
                  ,m_nActivePage(0)
    {
        DBG_CTOR(OPropertyBrowserView,NULL);

        m_pPropBox = new OPropertyEditor( this );
        m_pPropBox->SetHelpId(HID_FM_PROPDLG_TABCTR);
        m_pPropBox->setPageActivationHandler(LINK(this, OPropertyBrowserView, OnPageActivation));

        Size aSize = GetOutputSizePixel();
        m_pPropBox->SetPosSizePixel(Point(0,0), aSize);
        m_pPropBox->Show();
    }

    //------------------------------------------------------------------------
    IMPL_LINK(OPropertyBrowserView, OnPageActivation, void*, EMPTYARG)
    {
        m_nActivePage = m_pPropBox->GetCurPage();
        if (m_aPageActivationHandler.IsSet())
            m_aPageActivationHandler.Call(NULL);
        return 0L;
    }

    //------------------------------------------------------------------------
    OPropertyBrowserView::~OPropertyBrowserView()
    {
        if(m_pPropBox)
        {
            sal_uInt16 nTmpPage = m_pPropBox->GetCurPage();
            if (nTmpPage)
                m_nActivePage = nTmpPage;
            delete m_pPropBox;
        }
        if (m_pActiveController)
            m_pActiveController->release();
        m_pActiveController = NULL;

        m_xORB = NULL;

        DBG_DTOR(OPropertyBrowserView, NULL);
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::activatePage(sal_uInt16 _nPage)
    {
        m_nActivePage = _nPage;
        getPropertyBox()->SetPage(m_nActivePage);
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::setActiveController(OPropertyBrowserController* _pController)
    {
        if (_pController != m_pActiveController)
        {
            if (m_pActiveController) m_pActiveController->release();
            m_pActiveController = _pController;
            if (m_pActiveController) m_pActiveController->acquire();
        }
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::Resize()
    {
        Size aSize = GetOutputSizePixel();
        m_pPropBox->SetSizePixel(aSize);
    }

//............................................................................
} // namespace pcr
//............................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2001/01/18 14:45:10  rt
 *  #65293# semicolon removed
 *
 *  Revision 1.1  2001/01/12 11:26:10  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 08.01.01 11:05:54  fs
 ************************************************************************/

