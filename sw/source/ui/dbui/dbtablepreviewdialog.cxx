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

#include <swtypes.hxx>
#include <dbtablepreviewdialog.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <toolkit/unohlp.hxx>

#include <dbui.hrc>
#include <dbtablepreviewdialog.hrc>
#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::rtl;

SwDBTablePreviewDialog::SwDBTablePreviewDialog(Window* pParent, uno::Sequence< beans::PropertyValue>& rValues ) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_DBTABLEPREVIEWDIALOG)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aDescriptionFI( this, SW_RES(        FI_DESCRIPTION)),
    m_pBeamerWIN( new Window(this, SW_RES( WIN_BEAMER ))),
    m_aOK( this, SW_RES(                   PB_OK  ))
#ifdef MSC
#pragma warning (default : 4355)
#endif
{
    FreeResource();
    const beans::PropertyValue* pValues = rValues.getConstArray();
    for(sal_Int32 nValue = 0; nValue < rValues.getLength(); ++nValue        )
    {
        if ( pValues[nValue].Name == "Command" )
        {
            String sDescription = m_aDescriptionFI.GetText();
            OUString sTemp;
            pValues[nValue].Value >>= sTemp;
            sDescription.SearchAndReplaceAscii("%1", sTemp);
            m_aDescriptionFI.SetText(sDescription);
            break;
        }
    }

    try
    {
        // create a frame wrapper for myself
        uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
        m_xFrame = uno::Reference< frame::XFrame >(xMgr->createInstance(C2U("com.sun.star.frame.Frame")), uno::UNO_QUERY);
        if(m_xFrame.is())
        {
            m_xFrame->initialize( VCLUnoHelper::GetInterface ( m_pBeamerWIN ) );
        }
    }
    catch (uno::Exception const &)
    {
        m_xFrame.clear();
    }
    if(m_xFrame.is())
    {
        uno::Reference<frame::XDispatchProvider> xDP(m_xFrame, uno::UNO_QUERY);
        util::URL aURL;
        aURL.Complete = C2U(".component:DB/DataSourceBrowser");
        uno::Reference<frame::XDispatch> xD = xDP->queryDispatch(aURL,
                    C2U(""),
                    0x0C);
        if(xD.is())
        {
            xD->dispatch(aURL, rValues);
            m_pBeamerWIN->Show();
        }
    }
}

SwDBTablePreviewDialog::~SwDBTablePreviewDialog()
{
    if(m_xFrame.is())
    {
        m_xFrame->setComponent(NULL, NULL);
        m_xFrame->dispose();
    }
    else
        delete m_pBeamerWIN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
