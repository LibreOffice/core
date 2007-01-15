/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: browserview.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2007-01-15 14:40:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_
#include "browserview.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYEDITOR_HXX_
#include "propertyeditor.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_FORMHELPID_HRC_
#include "formhelpid.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <memory>

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
            ::std::auto_ptr<Window> aTemp(m_pPropBox);
            m_pPropBox = NULL;
        }
        m_xORB = NULL;

        DBG_DTOR(OPropertyBrowserView, NULL);
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::activatePage(sal_uInt16 _nPage)
    {
        m_nActivePage = _nPage;
        getPropertyBox().SetPage(m_nActivePage);
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::GetFocus()
    {
        if (m_pPropBox)
            m_pPropBox->GrabFocus();
        else
            Window::GetFocus();
    }

    //------------------------------------------------------------------------
    long OPropertyBrowserView::Notify( NotifyEvent& _rNEvt )
    {
        if ( EVENT_KEYINPUT == _rNEvt.GetType() )
        {
            sal_uInt16 nKey = _rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            if ( ( KEY_DELETE == nKey ) || ( KEY_BACKSPACE == nKey ) )
                // silence this, we don't want to propagate this outside the property
                // browser, as it will probably do harm there
                // #i63285# / 2006-12-06 / frank.schoenheit@sun.com
                return 1;
        }
        return Window::Notify( _rNEvt );
    }

    //------------------------------------------------------------------------
    void OPropertyBrowserView::Resize()
    {
        Size aSize = GetOutputSizePixel();
        m_pPropBox->SetSizePixel(aSize);
    }

    // #95343# ---------------------------------------------------------------
    ::com::sun::star::awt::Size OPropertyBrowserView::getMinimumSize()
    {
        Size aSize = GetOutputSizePixel();
        if( m_pPropBox )
        {
            aSize.setHeight( m_pPropBox->getMinimumHeight() );
            aSize.setWidth( m_pPropBox->getMinimumWidth() );
        }
        return ::com::sun::star::awt::Size( aSize.Width(), aSize.Height() );
    }

//............................................................................
} // namespace pcr
//............................................................................

