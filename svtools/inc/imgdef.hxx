/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgdef.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-04 14:47:50 $
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

#ifndef _SVTOOLS_IMGDEF_HXX
#define _SVTOOLS_IMGDEF_HXX

enum SfxSymbolsSize
{
    SFX_SYMBOLS_SIZE_SMALL,
    SFX_SYMBOLS_SIZE_LARGE,
    SFX_SYMBOLS_SIZE_AUTO
};

enum SfxSymbolsStyle
{
    SFX_SYMBOLS_STYLE_AUTO,
    SFX_SYMBOLS_STYLE_DEFAULT,
    SFX_SYMBOLS_STYLE_HICONTRAST,
    SFX_SYMBOLS_STYLE_INDUSTRIAL,
    SFX_SYMBOLS_STYLE_CRYSTAL,
    SFX_SYMBOLS_STYLE_TANGO
};

#define SFX_TOOLBOX_CHANGESYMBOLSET     0x0001
#define SFX_TOOLBOX_CHANGEOUTSTYLE      0x0002
#define SFX_TOOLBOX_CHANGEBUTTONTYPE    0x0004

#endif // _SVTOOLS_IMGDEF_HXX

