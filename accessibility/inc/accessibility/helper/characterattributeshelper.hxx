/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    typedef ::std::map< OUString, ::com::sun::star::uno::Any, ::std::less< OUString > > AttributeMap;

    AttributeMap    m_aAttributeMap;

public:

    CharacterAttributesHelper( const Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor );
    ~CharacterAttributesHelper();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes( const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes );
};

#endif  // ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
