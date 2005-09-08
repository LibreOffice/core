/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: searchopt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 10:17:19 $
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

#ifndef _SVT_SEARCHOPT_HXX_
#define _SVT_SEARCHOPT_HXX_

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SOLAR_H
#include "tools/solar.h"
#endif

class SvtSearchOptions_Impl;

class SVL_DLLPUBLIC SvtSearchOptions
{
    SvtSearchOptions_Impl   *pImpl;

    // disallow copy-constructor and assignment-operator for now
    SvtSearchOptions( const SvtSearchOptions & );
    SvtSearchOptions & operator = ( const SvtSearchOptions & );

protected:

public:
    SvtSearchOptions();
    ~SvtSearchOptions();

    INT32   GetTransliterationFlags() const;

    //
    // General Options
    //

    BOOL    IsWholeWordsOnly() const;
    BOOL    IsBackwards() const;
    BOOL    IsUseRegularExpression() const;
    BOOL    IsSearchForStyles() const;
    BOOL    IsSimilaritySearch() const;
    BOOL    IsUseAsianOptions() const;
    BOOL    IsMatchCase() const;            // also Japanese search option

    void    SetWholeWordsOnly( BOOL bVal );
    void    SetBackwards( BOOL bVal );
    void    SetUseRegularExpression( BOOL bVal );
    void    SetSearchForStyles( BOOL bVal );
    void    SetSimilaritySearch( BOOL bVal );
    void    SetUseAsianOptions( BOOL bVal );
    void    SetMatchCase( BOOL bVal );      // also Japanese search option

    //
    // Japanese search options
    //

    BOOL    IsMatchFullHalfWidthForms() const;
    BOOL    IsMatchHiraganaKatakana() const;
    BOOL    IsMatchContractions() const;
    BOOL    IsMatchMinusDashChoon() const;
    BOOL    IsMatchRepeatCharMarks() const;
    BOOL    IsMatchVariantFormKanji() const;
    BOOL    IsMatchOldKanaForms() const;
    BOOL    IsMatchDiziDuzu() const;
    BOOL    IsMatchBavaHafa() const;
    BOOL    IsMatchTsithichiDhizi() const;
    BOOL    IsMatchHyuiyuByuvyu() const;
    BOOL    IsMatchSesheZeje() const;
    BOOL    IsMatchIaiya() const;
    BOOL    IsMatchKiku() const;
    BOOL    IsIgnorePunctuation() const;
    BOOL    IsIgnoreWhitespace() const;
    BOOL    IsIgnoreProlongedSoundMark() const;
    BOOL    IsIgnoreMiddleDot() const;

    void    SetMatchFullHalfWidthForms( BOOL bVal );
    void    SetMatchHiraganaKatakana( BOOL bVal );
    void    SetMatchContractions( BOOL bVal );
    void    SetMatchMinusDashChoon( BOOL bVal );
    void    SetMatchRepeatCharMarks( BOOL bVal );
    void    SetMatchVariantFormKanji( BOOL bVal );
    void    SetMatchOldKanaForms( BOOL bVal );
    void    SetMatchDiziDuzu( BOOL bVal );
    void    SetMatchBavaHafa( BOOL bVal );
    void    SetMatchTsithichiDhizi( BOOL bVal );
    void    SetMatchHyuiyuByuvyu( BOOL bVal );
    void    SetMatchSesheZeje( BOOL bVal );
    void    SetMatchIaiya( BOOL bVal );
    void    SetMatchKiku( BOOL bVal );
    void    SetIgnorePunctuation( BOOL bVal );
    void    SetIgnoreWhitespace( BOOL bVal );
    void    SetIgnoreProlongedSoundMark( BOOL bVal );
    void    SetIgnoreMiddleDot( BOOL bVal );
};


#endif

