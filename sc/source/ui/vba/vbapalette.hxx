/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbapalette.hxx,v $
 * $Revision: 1.4 $
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

#include "excelvbahelper.hxx"
#include <document.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

class ScVbaPalette
{
private:
    SfxObjectShell* m_pShell;
public:
    ScVbaPalette(  SfxObjectShell* pShell = NULL ) : m_pShell( pShell ){}
    // if no palette available e.g. because the document doesn't have a
    // palette defined then a default palette will be returned.
    css::uno::Reference< css::container::XIndexAccess > getPalette() const;
    static css::uno::Reference< css::container::XIndexAccess > getDefaultPalette();
};

#endif //SC_VBAPALETTE_HXX

