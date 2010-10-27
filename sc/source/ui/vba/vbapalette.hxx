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

#ifndef SC_VBAPALETTE_HXX
#define SC_VBAPALETTE_HXX

#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace frame { class XModel; }
} } }

class SfxObjectShell;

class ScVbaPalette
{
private:
    SfxObjectShell* m_pShell;
public:
    ScVbaPalette( SfxObjectShell* pShell = 0 ) : m_pShell( pShell ) {}
    ScVbaPalette( const css::uno::Reference< css::frame::XModel >& rxModel );
    // if no palette available e.g. because the document doesn't have a
    // palette defined then a default palette will be returned.
    css::uno::Reference< css::container::XIndexAccess > getPalette() const;
    static css::uno::Reference< css::container::XIndexAccess > getDefaultPalette();
};

#endif //SC_VBAPALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
