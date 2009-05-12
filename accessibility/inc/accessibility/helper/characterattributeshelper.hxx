/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: characterattributeshelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
#define ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX
#include <com/sun/star/uno/Sequence.hxx>
#endif
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/font.hxx>

#include <map>

// -----------------------------------------------------------------------------
// class CharacterAttributesHelper
// -----------------------------------------------------------------------------

class CharacterAttributesHelper
{
private:

    typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any, ::std::less< ::rtl::OUString > > AttributeMap;

    AttributeMap    m_aAttributeMap;

public:

    CharacterAttributesHelper( const Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor );
    ~CharacterAttributesHelper();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes );
};

#endif  // ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
