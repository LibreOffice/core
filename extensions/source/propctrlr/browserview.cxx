/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "browserview.hxx"
#include "propertyeditor.hxx"
#include "propctrlr.hrc"
#include <tools/debug.hxx>
#include <boost/scoped_ptr.hpp>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;



    //= class OPropertyBrowserView


    OPropertyBrowserView::OPropertyBrowserView(Window* _pParent, WinBits nBits)
                  :Window(_pParent, nBits | WB_3DLOOK)
                  ,m_nActivePage(0)
    {
        m_pPropBox = new OPropertyEditor( this );
        m_pPropBox->SetHelpId(HID_FM_PROPDLG_TABCTR);
        m_pPropBox->setPageActivationHandler(LINK(this, OPropertyBrowserView, OnPageActivation));

        m_pPropBox->Show();
    }


    IMPL_LINK_NOARG(OPropertyBrowserView, OnPageActivation)
    {
        m_nActivePage = m_pPropBox->GetCurPage();
        if (m_aPageActivationHandler.IsSet())
            m_aPageActivationHandler.Call(NULL);
        return 0L;
    }


    OPropertyBrowserView::~OPropertyBrowserView()
    {
        if(m_pPropBox)
        {
            sal_uInt16 nTmpPage = m_pPropBox->GetCurPage();
            if (nTmpPage)
                m_nActivePage = nTmpPage;
            boost::scoped_ptr<Window> aTemp(m_pPropBox);
            m_pPropBox = NULL;
        }

    }


    void OPropertyBrowserView::activatePage(sal_uInt16 _nPage)
    {
        m_nActivePage = _nPage;
        getPropertyBox().SetPage(m_nActivePage);
    }


    void OPropertyBrowserView::GetFocus()
    {
        if (m_pPropBox)
            m_pPropBox->GrabFocus();
        else
            Window::GetFocus();
    }


    bool OPropertyBrowserView::Notify( NotifyEvent& _rNEvt )
    {
        if ( EVENT_KEYINPUT == _rNEvt.GetType() )
        {
            sal_uInt16 nKey = _rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

            if ( ( KEY_DELETE == nKey ) || ( KEY_BACKSPACE == nKey ) )
                // silence this, we don't want to propagate this outside the property
                // browser, as it will probably do harm there
                // #i63285#
                return true;
        }
        return Window::Notify( _rNEvt );
    }


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


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
