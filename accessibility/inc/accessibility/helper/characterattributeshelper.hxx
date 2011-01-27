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

#ifndef ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
#define ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
