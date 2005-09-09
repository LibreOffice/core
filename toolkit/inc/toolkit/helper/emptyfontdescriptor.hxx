/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: emptyfontdescriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:54:21 $
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

#ifndef _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_
#define _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif

//  ----------------------------------------------------
//  class EmptyFontDescriptor
//  ----------------------------------------------------
class EmptyFontDescriptor : public ::com::sun::star::awt::FontDescriptor
{
public:
    EmptyFontDescriptor()
    {
        // Not all enums are initialized correctly in FontDescriptor-CTOR because
        // they are set to the first enum value, this is not allways the default value.
        Slant = ::com::sun::star::awt::FontSlant_DONTKNOW;
        Underline = ::com::sun::star::awt::FontUnderline::DONTKNOW;
        Strikeout = ::com::sun::star::awt::FontStrikeout::DONTKNOW;
    }
};




#endif // _TOOLKIT_HELPER_EMPTYFONTDESCRIPTOR_HXX_

