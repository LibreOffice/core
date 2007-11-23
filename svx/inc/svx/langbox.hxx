/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:35:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_LANGBOX_HXX
#define _SVX_LANGBOX_HXX

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SvtLanguageTable;

#define LANG_LIST_EMPTY         0x0000
#define LANG_LIST_ALL           0x0001
#define LANG_LIST_WESTERN       0x0002
#define LANG_LIST_CTL           0x0004
#define LANG_LIST_CJK           0x0008
#define LANG_LIST_FBD_CHARS     0x0010
#define LANG_LIST_SPELL_AVAIL   0x0020
#define LANG_LIST_HYPH_AVAIL    0x0040
#define LANG_LIST_THES_AVAIL    0x0080
#define LANG_LIST_ONLY_KNOWN    0x0100  // list only locales provided by I18N
#define LANG_LIST_SPELL_USED    0x0200
#define LANG_LIST_HYPH_USED     0x0400
#define LANG_LIST_THES_USED     0x0800


class SVX_DLLPUBLIC SvxLanguageBox : public ListBox
{
public:

private:
    Image                   m_aNotCheckedImage;
    Image                   m_aCheckedImage;
    Image                   m_aCheckedImageHC;
    String                  m_aAllString;
    com::sun::star::uno::Sequence< INT16 >  *m_pSpellUsedLang;
    SvtLanguageTable*       m_pLangTable;
    INT16                   m_nLangList;
    BOOL                    m_bHasLangNone;
    BOOL                    m_bLangNoneIsLangAll;
    BOOL                    m_bWithCheckmark;

    SVX_DLLPRIVATE void                    Init();
    SVX_DLLPRIVATE USHORT                  ImplInsertImgEntry( const String& rEntry, USHORT nPos, bool bChecked );

public:
    SvxLanguageBox( Window* pParent, WinBits nWinStyle, BOOL bCheck = FALSE);
    SvxLanguageBox( Window* pParent, const ResId& rResId, BOOL bCheck = FALSE);
    ~SvxLanguageBox();

    void            SetLanguageList( INT16 nLangList,
                            BOOL bHasLangNone, BOOL bLangNoneIsLangAll = FALSE,
                            BOOL bCheckSpellAvail = FALSE );

    USHORT          InsertLanguage( const LanguageType eLangType, USHORT nPos = LISTBOX_APPEND );
    USHORT          InsertLanguage( const LanguageType eLangType,
                            BOOL bCheckEntry, USHORT nPos = LISTBOX_APPEND );
    void            RemoveLanguage( const LanguageType eLangType );
    void            SelectLanguage( const LanguageType eLangType, BOOL bSelect = TRUE );
    LanguageType    GetSelectLanguage() const;
    BOOL            IsLanguageSelected( const LanguageType eLangType ) const;
};

#endif

