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
#ifndef INCLUDED_SW_INC_EDGLBLDC_HXX
#define INCLUDED_SW_INC_EDGLBLDC_HXX

#include <o3tl/sorted_vector.hxx>
#include <tools/solar.h>

class SwSection;
class SwTOXBase;
class SwTOXBaseSection;

enum GlobalDocContentType {
    GLBLDOC_UNKNOWN,
    GLBLDOC_TOXBASE,
    GLBLDOC_SECTION
};

class SwGlblDocContent
{
    GlobalDocContentType m_eType;
    sal_uLong m_nDocPos;
    union {
        const SwTOXBase* pTOX;
        const SwSection* pSect;
    } m_PTR;

public:
    SwGlblDocContent( sal_uLong nPos );
    SwGlblDocContent( const SwTOXBaseSection* pTOX );
    SwGlblDocContent( const SwSection* pSect );

    /// Query contents.
    GlobalDocContentType GetType() const { return m_eType; }
    const SwSection* GetSection() const
                            { return GLBLDOC_SECTION == m_eType ? m_PTR.pSect : nullptr; }
    const SwTOXBase* GetTOX() const
                            { return GLBLDOC_TOXBASE == m_eType ? m_PTR.pTOX : nullptr; }
    sal_uLong GetDocPos() const { return m_nDocPos; }

    /// For sorting.
    bool operator==( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() == rCmp.GetDocPos(); }
    bool operator<( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() < rCmp.GetDocPos(); }
};

class SwGlblDocContents : public o3tl::sorted_vector<std::unique_ptr<SwGlblDocContent>, o3tl::less_uniqueptr_to<SwGlblDocContent> > {};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
