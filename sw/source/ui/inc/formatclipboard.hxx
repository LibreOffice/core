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

#ifndef _SWFORMATCLIPBOARD_HXX
#define _SWFORMATCLIPBOARD_HXX

// header for class SwWrtShell
#include <wrtsh.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
// header for class SfxStyleSheetBasePool
#include <svl/style.hxx>

//-----------------------------------------------------------------------------
/** This class acts as data container and execution class for the format paintbrush feature in writer.
*/

class SwFormatClipboard
{
public:
    SwFormatClipboard();
    ~SwFormatClipboard();

    bool HasContent() const;
    bool HasContentForThisType( int nSelectionType ) const;
    bool CanCopyThisType( int nSelectionType ) const;

    void Copy( SwWrtShell& rWrtShell, SfxItemPool& rPool, bool bPersistentCopy=false );
    void Paste( SwWrtShell& rWrtShell, SfxStyleSheetBasePool* pPool
        , bool bNoCharacterFormats=false, bool bNoParagraphFormats=false );
    void Erase();

private:
    int         m_nSelectionType;
    SfxItemSet* m_pItemSet;
    SfxItemSet* m_pTableItemSet;

    String m_aCharStyle;
    String m_aParaStyle;
    //no frame style because it contains position information

    bool   m_bPersistentCopy;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
