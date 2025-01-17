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
#ifndef INCLUDED_SW_INC_ISTYLEACCESS_HXX
#define INCLUDED_SW_INC_ISTYLEACCESS_HXX

#include <svl/itemset.hxx>
#include <memory>
#include <vector>

class SwAttrSet;
class ProgName;

// Management of (automatic) styles
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

    virtual std::shared_ptr<SfxItemSet> getAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily,
                                                               const OUString* pParentName = nullptr ) = 0;
    virtual std::shared_ptr<SwAttrSet> getAutomaticStyle( const SwAttrSet& rSet,
                                                               SwAutoStyleFamily eFamily,
                                                               const ProgName* pParentName = nullptr ) = 0;
    virtual void getAllStyles( std::vector<std::shared_ptr<SfxItemSet>> &rStyles,
                                                               SwAutoStyleFamily eFamily ) = 0;
    /** It's slow to iterate through a stylepool looking for a special name, but if
     the style has been inserted via "cacheAutomaticStyle" instead of "getAutomaticStyle",
     it's faster */
    virtual std::shared_ptr<SfxItemSet> getByName( const OUString& rName,
                                                               SwAutoStyleFamily eFamily ) = 0;
    /// insert the style to the pool and the cache (used during import)
    virtual std::shared_ptr<SfxItemSet> cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) = 0;
    /// To release the cached styles (shared_pointer!)
    virtual void clearCaches() = 0;
};

#endif // INCLUDED_SW_INC_ISTYLEACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
