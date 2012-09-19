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
#ifndef _SVX_LANGBOX_HXX
#define _SVX_LANGBOX_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/lstbox.hxx>
#include "svx/svxdllapi.h"
#include <vcl/image.hxx>
#include <tools/solar.h>

class SvtLanguageTable;

#define LANG_LIST_EMPTY             0x0000
#define LANG_LIST_ALL               0x0001
#define LANG_LIST_WESTERN           0x0002
#define LANG_LIST_CTL               0x0004
#define LANG_LIST_CJK               0x0008
#define LANG_LIST_FBD_CHARS         0x0010
#define LANG_LIST_SPELL_AVAIL       0x0020
#define LANG_LIST_HYPH_AVAIL        0x0040
#define LANG_LIST_THES_AVAIL        0x0080
#define LANG_LIST_ONLY_KNOWN        0x0100  // list only locales provided by I18N
#define LANG_LIST_SPELL_USED        0x0200
#define LANG_LIST_HYPH_USED         0x0400
#define LANG_LIST_THES_USED         0x0800
#define LANG_LIST_ALSO_PRIMARY_ONLY 0x1000  // Do not exclude primary-only
                                            // languages that do not form a
                                            // locale, such as Arabic as
                                            // opposed to Arabic-Egypt.


// load language strings from resource
SVX_DLLPUBLIC String    GetDicInfoStr( const String& rName, const sal_uInt16 nLang, sal_Bool bNeg );

class SVX_DLLPUBLIC SvxLanguageBox : public ListBox
{
public:

private:
    Image                   m_aNotCheckedImage;
    Image                   m_aCheckedImage;
    String                  m_aAllString;
    com::sun::star::uno::Sequence< sal_Int16 >  *m_pSpellUsedLang;
    SvtLanguageTable*       m_pLangTable;
    sal_Int16                   m_nLangList;
    sal_Bool                    m_bHasLangNone;
    sal_Bool                    m_bLangNoneIsLangAll;
    sal_Bool                    m_bWithCheckmark;

    SVX_DLLPRIVATE void                    Init();
    SVX_DLLPRIVATE sal_uInt16                  ImplInsertImgEntry( const String& rEntry, sal_uInt16 nPos, bool bChecked );
    SVX_DLLPRIVATE sal_uInt16                  ImplInsertLanguage(LanguageType, sal_uInt16, sal_Int16 );

public:
    SvxLanguageBox(Window* pParent, const ResId& rResId, sal_Bool bCheck = sal_False);
    SvxLanguageBox(Window* pParent, WinBits nBits, sal_Bool bCheck = sal_False);
    ~SvxLanguageBox();

    void            SetLanguageList( sal_Int16 nLangList,
                            sal_Bool bHasLangNone, sal_Bool bLangNoneIsLangAll = sal_False,
                            sal_Bool bCheckSpellAvail = sal_False );

    sal_uInt16          InsertLanguage( const LanguageType eLangType, sal_uInt16 nPos = LISTBOX_APPEND );
    sal_uInt16          InsertDefaultLanguage( sal_Int16 nType, sal_uInt16 nPos = LISTBOX_APPEND );
    sal_uInt16          InsertSystemLanguage( sal_uInt16 nPos = LISTBOX_APPEND );
    sal_uInt16          InsertLanguage( const LanguageType eLangType,
                            sal_Bool bCheckEntry, sal_uInt16 nPos = LISTBOX_APPEND );
    void            RemoveLanguage( const LanguageType eLangType );
    void            SelectLanguage( const LanguageType eLangType, sal_Bool bSelect = sal_True );
    LanguageType    GetSelectLanguage() const;
    sal_Bool            IsLanguageSelected( const LanguageType eLangType ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
