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
#ifndef _ISTYLEACCESS_HXX
#define _ISTYLEACCESS_HXX

#include <vector>
#include <svl/stylepool.hxx>

/*--------------------------------------------------------------------
    Management of (automatic) styles
 --------------------------------------------------------------------*/

class SwStyleHandle;

class IStyleAccess
{
public:

    enum SwAutoStyleFamily
    {
        AUTO_STYLE_CHAR,
        AUTO_STYLE_RUBY,
        AUTO_STYLE_PARA,
        AUTO_STYLE_NOTXT
    };

    virtual ~IStyleAccess() {}

    virtual StylePool::SfxItemSet_Pointer_t getAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) = 0;
    virtual void getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // It's slow to iterate through a stylepool looking for a special name, but if
    // the style has been inserted via "cacheAutomaticStyle" instead of "getAutomaticStyle",
    // it's faster
    virtual StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // insert the style to the pool and the cache (used during import)
    virtual StylePool::SfxItemSet_Pointer_t cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) = 0;
    // To release the cached styles (shared_pointer!)
    virtual void clearCaches() = 0;
};

#endif // _ISTYLEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
