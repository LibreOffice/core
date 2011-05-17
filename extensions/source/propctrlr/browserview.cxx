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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "browserview.hxx"
#include "propertyeditor.hxx"
#include "propctrlr.hrc"
#include <tools/debug.hxx>
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
                // #i63285#
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
