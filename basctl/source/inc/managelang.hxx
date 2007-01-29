/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: managelang.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 16:53:20 $
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

#ifndef _BASCTL_MANAGELANG_HXX
#define _BASCTL_MANAGELANG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX
#include <svx/checklbx.hxx>
#endif

class LocalizationMgr;

struct LanguageEntry
{
    String                          m_sLanguage;
    ::com::sun::star::lang::Locale  m_aLocale;
    bool                            m_bIsDefault;

    LanguageEntry( const String& _rLanguage,
                   const ::com::sun::star::lang::Locale& _rLocale,
                   bool _bIsDefault ) :
        m_sLanguage( _rLanguage ),
        m_aLocale( _rLocale ),
        m_bIsDefault( _bIsDefault ) {}
};

extern bool localesAreEqual( const ::com::sun::star::lang::Locale& rLocaleLeft,
                             const ::com::sun::star::lang::Locale& rLocaleRight );

class ManageLanguageDialog : public ModalDialog
{
private:
    FixedText           m_aLanguageFT;
    ListBox             m_aLanguageLB;
    PushButton          m_aAddPB;
    PushButton          m_aDeletePB;
    PushButton          m_aMakeDefPB;
    FixedText           m_aInfoFT;

    FixedLine           m_aBtnLine;
    HelpButton          m_aHelpBtn;
    OKButton            m_aCloseBtn;

    LocalizationMgr*    m_pLocalizationMgr;

    String              m_sDefLangStr;
    String              m_sDeleteStr;
    String              m_sCreateLangStr;

    void                Init();
    void                CalcInfoSize();
    void                FillLanguageBox();
    void                ClearLanguageBox();

    DECL_LINK(          AddHdl, Button * );
    DECL_LINK(          DeleteHdl, Button * );
    DECL_LINK(          MakeDefHdl, Button * );
    DECL_LINK(          SelectHdl, ListBox * );

public:
    ManageLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr );
    ~ManageLanguageDialog();
};

class SetDefaultLanguageDialog : public ModalDialog
{
private:
    FixedText           m_aLanguageFT;
    SvxLanguageBox*     m_pLanguageLB;
    SvxCheckListBox*    m_pCheckLangLB;
    FixedText           m_aInfoFT;

    FixedLine           m_aBtnLine;
    OKButton            m_aOKBtn;
    CancelButton        m_aCancelBtn;
    HelpButton          m_aHelpBtn;

    bool                m_bIsDefaultMode;
    LocalizationMgr*    m_pLocalizationMgr;

    void                FillLanguageBox();
    void                CalcInfoSize();

public:
    SetDefaultLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr );
    ~SetDefaultLanguageDialog();

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >   GetLocales() const;
};

#endif //_BASCTL_MANAGELANG_HXX

