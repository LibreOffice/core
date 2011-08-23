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

#ifndef SVX_UNOFDESC_HXX
#define SVX_UNOFDESC_HXX

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
namespace binfilter {

class SvxUnoFontDescriptor
{
public:
    static void ConvertToFont( const ::com::sun::star::awt::FontDescriptor& rDesc, Font& rFont );
    static void ConvertFromFont( const Font& rFont, ::com::sun::star::awt::FontDescriptor& rDesc );

    static void FillItemSet( const ::com::sun::star::awt::FontDescriptor& rDesc, SfxItemSet& rSet );
    static void FillFromItemSet( const SfxItemSet& rSet, ::com::sun::star::awt::FontDescriptor& rDesc );

    static void setPropertyToDefault( SfxItemSet& rSet );
    static ::com::sun::star::uno::Any getPropertyDefault( SfxItemPool* pPool );

};

}//end of namespace binfilter
#endif

