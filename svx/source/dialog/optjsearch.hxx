/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optjsearch.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:47:02 $
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

#ifndef _SVX_OPTJSEARCH_HXX_
#define _SVX_OPTJSEARCH_HXX_

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

class Window;
class SfxItemSet;

//////////////////////////////////////////////////////////////////////

class SvxJSearchOptionsPage : public SfxTabPage
{
private:
    FixedLine   aTreatAsEqual;
    CheckBox    aMatchCase;
    CheckBox    aMatchFullHalfWidth;
    CheckBox    aMatchHiraganaKatakana;
    CheckBox    aMatchContractions;
    CheckBox    aMatchMinusDashChoon;
    CheckBox    aMatchRepeatCharMarks;
    CheckBox    aMatchVariantFormKanji;
    CheckBox    aMatchOldKanaForms;
    CheckBox    aMatchDiziDuzu;
    CheckBox    aMatchBavaHafa;
    CheckBox    aMatchTsithichiDhizi;
    CheckBox    aMatchHyuiyuByuvyu;
    CheckBox    aMatchSesheZeje;
    CheckBox    aMatchIaiya;
    CheckBox    aMatchKiku;
    CheckBox    aMatchProlongedSoundMark;
    FixedLine   aIgnore;
    CheckBox    aIgnorePunctuation;
    CheckBox    aIgnoreWhitespace;
    CheckBox    aIgnoreMiddleDot;

    INT32       nTransliterationFlags;
    BOOL        bSaveOptions;

    INT32               GetTransliterationFlags_Impl();

protected:
                        SvxJSearchOptionsPage( Window* pParent, const SfxItemSet& rSet );

public:
                        ~SvxJSearchOptionsPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );

    virtual void        Reset( const SfxItemSet& rSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );

    BOOL                IsSaveOptions() const           { return bSaveOptions; }
    void                EnableSaveOptions( BOOL bVal )  { bSaveOptions = bVal; }

    INT32               GetTransliterationFlags() const { return nTransliterationFlags; }
    void                SetTransliterationFlags( INT32 nSettings );
};

//////////////////////////////////////////////////////////////////////

#endif

