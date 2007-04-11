/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: languageoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:26:22 $
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
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#define _SVTOOLS_LANGUAGEOPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

// class SvtLanguageOptions ----------------------------------------------------

// these defines can be ored
#define SCRIPTTYPE_LATIN        0x01
#define SCRIPTTYPE_ASIAN        0x02
#define SCRIPTTYPE_COMPLEX      0x04

class SvtCJKOptions;
class SvtCTLOptions;

class SVT_DLLPUBLIC SvtLanguageOptions : public SfxBroadcaster, public SfxListener
{
private:
    SvtCJKOptions*  m_pCJKOptions;
    SvtCTLOptions*  m_pCTLOptions;

public:
    enum EOption
    {
        // cjk options
        E_CJKFONT,
        E_VERTICALTEXT,
        E_ASIANTYPOGRAPHY,
        E_JAPANESEFIND,
        E_RUBY,
        E_CHANGECASEMAP,
        E_DOUBLELINES,
        E_EMPHASISMARKS,
        E_VERTICALCALLOUT,
        E_ALLCJK,
        // ctl options
        E_CTLFONT,
        E_CTLSEQUENCECHECKING,
        E_CTLCURSORMOVEMENT,
        E_CTLTEXTNUMERALS
    };

    // bDontLoad is for referencing purposes only
    SvtLanguageOptions( sal_Bool _bDontLoad = sal_False );
    ~SvtLanguageOptions();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // CJK options
    sal_Bool    IsCJKFontEnabled() const;
    sal_Bool    IsVerticalTextEnabled() const;
    sal_Bool    IsAsianTypographyEnabled() const;
    sal_Bool    IsJapaneseFindEnabled() const;
    sal_Bool    IsRubyEnabled() const;
    sal_Bool    IsChangeCaseMapEnabled() const;
    sal_Bool    IsDoubleLinesEnabled() const;
    sal_Bool    IsEmphasisMarksEnabled() const;
    sal_Bool    IsVerticalCallOutEnabled() const;
    void        SetAll( sal_Bool _bSet );
    sal_Bool    IsAnyEnabled() const;

    // CTL options
    void        SetCTLFontEnabled( sal_Bool _bEnabled );
    sal_Bool    IsCTLFontEnabled() const;

    void        SetCTLSequenceChecking( sal_Bool _bEnabled );
    sal_Bool    IsCTLSequenceChecking() const;

    void        SetCTLSequenceCheckingRestricted( sal_Bool _bEnable );
    sal_Bool    IsCTLSequenceCheckingRestricted( void ) const;

    void        SetCTLSequenceCheckingTypeAndReplace( sal_Bool _bEnable );
    sal_Bool    IsCTLSequenceCheckingTypeAndReplace() const;

    sal_Bool    IsReadOnly(EOption eOption) const;

    // returns for a language the scripttype
    static sal_uInt16 GetScriptTypeOfLanguage( sal_uInt16 nLang );
};
/** #i42730# Gives access to the Windows 16bit system locale
 */
class SVT_DLLPUBLIC SvtSystemLanguageOptions : public utl::ConfigItem
{
private:
    ::rtl::OUString m_sWin16SystemLocale;

public:
    SvtSystemLanguageOptions();
    ~SvtSystemLanguageOptions();

    virtual void    Commit();

    LanguageType GetWin16SystemLanguage();
};

#endif // _SVTOOLS_LANGUAGEOPTIONS_HXX

