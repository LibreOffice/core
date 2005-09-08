/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unofdesc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:24:38 $
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

#ifndef SVX_UNOFDESC_HXX
#define SVX_UNOFDESC_HXX

#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SVX_DLLPUBLIC SvxUnoFontDescriptor
{
public:
    static void ConvertToFont( const ::com::sun::star::awt::FontDescriptor& rDesc, Font& rFont );
    static void ConvertFromFont( const Font& rFont, ::com::sun::star::awt::FontDescriptor& rDesc );

    static void FillItemSet( const ::com::sun::star::awt::FontDescriptor& rDesc, SfxItemSet& rSet );
    static void FillFromItemSet( const SfxItemSet& rSet, ::com::sun::star::awt::FontDescriptor& rDesc );

    static com::sun::star::beans::PropertyState getPropertyState( const SfxItemSet& rSet );
    static void setPropertyToDefault( SfxItemSet& rSet );
    static ::com::sun::star::uno::Any getPropertyDefault( SfxItemPool* pPool );

};


#endif

