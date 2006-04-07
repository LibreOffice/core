/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zforauto.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 16:23:14 $
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

#ifndef _ZFORAUTO_HXX_
#define _ZFORAUTO_HXX_

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif


class SvStream;
class SvNumberFormatter;


class ScNumFormatAbbrev
{
    String sFormatstring;
    LanguageType eLnge;
    LanguageType eSysLnge;
public:
    ScNumFormatAbbrev();
    ScNumFormatAbbrev(const ScNumFormatAbbrev& aFormat);
    ScNumFormatAbbrev(ULONG nFormat, SvNumberFormatter& rFormatter);
    void Load( SvStream& rStream, CharSet eByteStrSet );            // Laden der Zahlenformate
    void Save( SvStream& rStream, CharSet eByteStrSet ) const;   // Speichern der Zahlenformate
    void PutFormatIndex(ULONG nFormat, SvNumberFormatter& rFormatter);
    ULONG GetFormatIndex( SvNumberFormatter& rFormatter);
    inline int operator==(const ScNumFormatAbbrev& rNumFormat) const
    {
        return ((sFormatstring == rNumFormat.sFormatstring)
            && (eLnge == rNumFormat.eLnge)
            && (eSysLnge == rNumFormat.eSysLnge));
    }
    inline ScNumFormatAbbrev& operator=(const ScNumFormatAbbrev& rNumFormat)
    {
        sFormatstring = rNumFormat.sFormatstring;
        eLnge = rNumFormat.eLnge;
        eSysLnge = rNumFormat.eSysLnge;
        return *this;
    }
    inline LanguageType GetLanguage() const { return eLnge; }
};



#endif


