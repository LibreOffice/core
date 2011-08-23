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
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#define _SVTOOLS_LANGUAGEOPTIONS_HXX

#include "bf_svtools/svtdllapi.h"

#include <sal/types.h>
#include <bf_svtools/brdcst.hxx>
#include <bf_svtools/lstner.hxx>
#include <unotools/configitem.hxx>
#include <i18npool/lang.h>

namespace binfilter
{

// class SvtLanguageOptions ----------------------------------------------------

// these defines can be ored
#define SCRIPTTYPE_LATIN		0x01
#define SCRIPTTYPE_ASIAN		0x02
#define SCRIPTTYPE_COMPLEX		0x04

class SvtCJKOptions;
class SvtCTLOptions;

class  SvtLanguageOptions : public SfxBroadcaster, public SfxListener
{
private:
    SvtCJKOptions*	m_pCJKOptions;
    SvtCTLOptions*	m_pCTLOptions;

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

    // returns for a language the scripttype
    static sal_uInt16 GetScriptTypeOfLanguage( sal_uInt16 nLang );
};
/** #i42730# Gives access to the Windows 16bit system locale
 */
class  SvtSystemLanguageOptions : public utl::ConfigItem
{
private:
    ::rtl::OUString m_sWin16SystemLocale;

public:
    SvtSystemLanguageOptions();
    ~SvtSystemLanguageOptions();

    virtual void    Commit();
    void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );

    LanguageType GetWin16SystemLanguage();
};

}

#endif // _SVTOOLS_LANGUAGEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
