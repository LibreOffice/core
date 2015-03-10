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

#include <swtypes.hxx>
#include <dbtablepreviewdialog.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/Frame.hpp>
#include <toolkit/helper/vclunohelper.hxx>

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

SwDBTablePreviewDialog::SwDBTablePreviewDialog(vcl::Window* pParent, uno::Sequence< beans::PropertyValue>& rValues ) :
    SfxModalDialog(pParent, "TablePreviewDialog", "modules/swriter/ui/tablepreviewdialog.ui")
{
    get(m_pDescriptionFI, "description");
    get(m_pBeamerWIN, "beamer");
    Size aSize(LogicToPixel(Size(338, 150), MAP_APPFONT));
    m_pBeamerWIN->set_width_request(aSize.Width());
    m_pBeamerWIN->set_height_request(aSize.Height());

    const beans::PropertyValue* pValues = rValues.getConstArray();
    for(sal_Int32 nValue = 0; nValue < rValues.getLength(); ++nValue        )
    {
        if ( pValues[nValue].Name == "Command" )
        {
            OUString sDescription = m_pDescriptionFI->GetText();
            OUString sTemp;
            pValues[nValue].Value >>= sTemp;
            m_pDescriptionFI->SetText(sDescription.replaceFirst("%1", sTemp));
            break;
        }
    }

    try
    {
        // create a frame wrapper for myself
        m_xFrame = frame::Frame::create( comphelper::getProcessComponentContext() );
        // m_xFrame takes ownership of m_pBeamerWIN
        m_pUIBuilder->drop_ownership(m_pBeamerWIN);
        m_xFrame->initialize( VCLUnoHelper::GetInterface( m_pBeamerWIN ) );
    }
    catch (uno::Exception const &)
    {
        m_xFrame.clear();
    }
    if(m_xFrame.is())
    {
        util::URL aURL;
        aURL.Complete = ".component:DB/DataSourceBrowser";
        uno::Reference<frame::XDispatch> xD = m_xFrame->queryDispatch(aURL, "", 0x0C);
        if(xD.is())
        {
            xD->dispatch(aURL, rValues);
            m_pBeamerWIN->Show();
        }
    }
}

SwDBTablePreviewDialog::~SwDBTablePreviewDialog()
{
    disposeOnce();
}

void SwDBTablePreviewDialog::dispose()
{
    if(m_xFrame.is())
    {
        m_xFrame->setComponent(NULL, NULL);
        m_xFrame->dispose();
    }
    m_pDescriptionFI.clear();
    m_pBeamerWIN.clear();
    SfxModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
