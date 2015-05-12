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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FORMATCLIPBOARD_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FORMATCLIPBOARD_HXX

#include <wrtsh.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>

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
    static bool CanCopyThisType( int nSelectionType );

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
    SfxItemSet* m_pItemSet_TextAttr;
    /** automatic/named paragraph attribute set
     * (it can be caractere attribute applyied to the paragraph) */
    SfxItemSet* m_pItemSet_ParAttr;

    /** table attribute set */
    SfxItemSet* m_pTableItemSet;

    /** name of the character format (if it exist) */
    OUString m_aCharStyle;
    /** name of the paragraph format (if it exist) */
    OUString m_aParaStyle;
    //no frame style because it contains position information

    /** specify if the Paste function have to clear the current object */
    bool   m_bPersistentCopy;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
