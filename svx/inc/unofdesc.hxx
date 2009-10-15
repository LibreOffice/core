/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unofdesc.hxx,v $
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

#ifndef SVX_UNOFDESC_HXX
#define SVX_UNOFDESC_HXX

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <svl/itemset.hxx>
#include <vcl/font.hxx>
#include "svx/svxdllapi.h"

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

