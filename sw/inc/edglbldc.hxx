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
#ifndef _EDGLBLDC_HXX
#define _EDGLBLDC_HXX

#include <o3tl/sorted_vector.hxx>

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
    GlobalDocContentType eType;
    sal_uLong nDocPos;
    union {
        const SwTOXBase* pTOX;
        const SwSection* pSect;
    } PTR;

public:
    SwGlblDocContent( sal_uLong nPos );
    SwGlblDocContent( const SwTOXBaseSection* pTOX );
    SwGlblDocContent( const SwSection* pSect );

    /// Query contents.
    GlobalDocContentType GetType() const { return eType; }
    const SwSection* GetSection() const
                            { return GLBLDOC_SECTION == eType ? PTR.pSect : 0; }
    const SwTOXBase* GetTOX() const
                            { return GLBLDOC_TOXBASE == eType ? PTR.pTOX : 0; }
    sal_uLong GetDocPos() const { return nDocPos; }

    /// For sorting.
    inline int operator==( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() == rCmp.GetDocPos(); }
    inline int operator<( const SwGlblDocContent& rCmp ) const
        {   return GetDocPos() < rCmp.GetDocPos(); }
};


class SwGlblDocContents : public o3tl::sorted_vector<SwGlblDocContent*, o3tl::less_ptr_to<SwGlblDocContent> > {
public:
    ~SwGlblDocContents() { DeleteAndDestroyAll(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
