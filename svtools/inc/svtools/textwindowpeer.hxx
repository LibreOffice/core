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

#ifndef INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
#define INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX

#include "svtools/svtdllapi.h"
#include <toolkit/awt/vclxwindow.hxx>

#include <memory>

class TextEngine;
class TextView;

namespace svt
{
    class AccessibleFactoryAccess ;

class TextWindowPeer: public ::VCLXWindow
{
public:
    SVT_DLLPUBLIC TextWindowPeer(::TextView & rView, bool bCompoundControlChild = false);

    virtual ~TextWindowPeer();

private:
    // VCLXWindow inherits funny copy constructor and assignment operator from
    // ::cppu::OWeakObject, so override them here:

    TextWindowPeer(TextWindowPeer &); // not implemented

    void operator =(TextWindowPeer); // not implemented

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext >
    CreateAccessibleContext();

    ::TextEngine & m_rEngine;
    ::TextView & m_rView;
    bool m_bCompoundControlChild;

    ::std::auto_ptr< ::svt::AccessibleFactoryAccess  >  m_pFactoryAccess;
};

}

#endif // INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
