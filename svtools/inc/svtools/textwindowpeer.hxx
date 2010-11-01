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

#if !defined INCLUDED_SVTOOLS_TEXTWINDOWPEER_HXX
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
