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

#include <sal/config.h>

#include <svtaccessiblefactory.hxx>

#include <boost/noncopyable.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <sal/types.h>
#include <svtools/textwindowpeer.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>

namespace {

class TextWindowPeer: public VCLXWindow, private boost::noncopyable {
public:
    explicit TextWindowPeer(TextView & view);

    virtual ~TextWindowPeer() {}

private:
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
    CreateAccessibleContext() override;

    TextEngine & m_rEngine;
    TextView & m_rView;
    svt::AccessibleFactoryAccess m_aFactoryAccess;
};

TextWindowPeer::TextWindowPeer(TextView & view):
    m_rEngine(*view.GetTextEngine()), m_rView(view)
{
    SetWindow(view.GetWindow());
}

css::uno::Reference<css::accessibility::XAccessibleContext>
TextWindowPeer::CreateAccessibleContext() {
    return m_aFactoryAccess.getFactory().createAccessibleTextWindowContext(
        this, m_rEngine, m_rView);
}

}

css::uno::Reference<css::awt::XWindowPeer> svt::createTextWindowPeer(
    TextView & view)
{
    return new TextWindowPeer(view);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
