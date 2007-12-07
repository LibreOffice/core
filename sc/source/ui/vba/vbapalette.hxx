/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapalette.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:57:55 $
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
#ifndef SC_VBAPALETTE_HXX
#define SC_VBAPALETTE_HXX

#include "vbahelper.hxx"
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
    css::uno::Reference< css::container::XIndexAccess > getPalette();
    static css::uno::Reference< css::container::XIndexAccess > getDefaultPalette();
};

#endif //SC_VBAPALETTE_HXX

