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

    /**
     * Test if the object contains text or paragraph attribute
     */
    bool HasContent() const;
    bool HasContentForThisType( int nSelectionType ) const;
    bool CanCopyThisType( int nSelectionType ) const;

    /**
     * Store/Backup the text and paragraph attribute of the current selection.
     *
     * @param bPersistentCopy
     * input parameter - specify if the Paste function will erase the current object.
     */
    void Copy( SwWrtShell& rWrtShell, SfxItemPool& rPool, bool bPersistentCopy=false );

    /**
     * Paste the stored text and paragraph attributes on the current selection and current paragraph.
     *
     * @param bNoCharacterFormats
     * Do not paste the character formats.
     *
     * @param bNoParagraphFormats
     * Do not paste the paragraph formats.
     */
    void Paste( SwWrtShell& rWrtShell, SfxStyleSheetBasePool* pPool
        , bool bNoCharacterFormats=false, bool bNoParagraphFormats=false );

    /**
     * Clear the currently stored text and paragraph attributes.
     */
    void Erase();

private:
    int         m_nSelectionType;

    /** automatic/named character attribute set */
    SfxItemSet* m_pItemSet_TxtAttr;
    /** automatic/named paragraph attribute set
     * (it can be caractere attribute applyied to the paragraph) */
    SfxItemSet* m_pItemSet_ParAttr;

    /** table attribute set */
    SfxItemSet* m_pTableItemSet;

    /** name of the character format (if it exist) */
    String m_aCharStyle;
    /** name of the paragraph format (if it exist) */
    String m_aParaStyle;
    //no frame style because it contains position information

    /** specify if the Paste function have to clear the current object */
    bool   m_bPersistentCopy;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
