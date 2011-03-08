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
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#define INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX

#include "svtools/svtdllapi.h"
#include <unotools/configitem.hxx>
#include <tools/string.hxx>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>

class SvtAccessibilityOptions_Impl;

class SVT_DLLPUBLIC SvtAccessibilityOptions:
    public utl::detail::Options, private SfxListener
{
private:
    static SvtAccessibilityOptions_Impl* volatile sm_pSingleImplConfig;
    static sal_Int32                     volatile sm_nAccessibilityRefCount;

public:
    SvtAccessibilityOptions();
    virtual ~SvtAccessibilityOptions();

    // get & set config entries
    sal_Bool    GetIsForDrawings() const;       // obsolete!
    sal_Bool    GetIsForBorders() const;        // obsolete!
    sal_Bool    GetIsForPagePreviews() const;
    sal_Bool    GetIsHelpTipsDisappear() const;
    sal_Bool    GetIsAllowAnimatedGraphics() const;
    sal_Bool    GetIsAllowAnimatedText() const;
    sal_Bool    GetIsAutomaticFontColor() const;
    sal_Bool    GetIsSystemFont() const;
    sal_Int16   GetHelpTipSeconds() const;
    sal_Bool    IsSelectionInReadonly() const;
    sal_Bool    GetAutoDetectSystemHC() const;

    void        SetIsForPagePreviews(sal_Bool bSet);
    void        SetIsHelpTipsDisappear(sal_Bool bSet);
    void        SetIsAllowAnimatedGraphics(sal_Bool bSet);
    void        SetIsAllowAnimatedText(sal_Bool bSet);
    void        SetIsAutomaticFontColor(sal_Bool bSet);
    void        SetIsSystemFont(sal_Bool bSet);
    void        SetHelpTipSeconds(sal_Int16 nSet);
    void        SetSelectionInReadonly(sal_Bool bSet);
    void        SetAutoDetectSystemHC(sal_Bool bSet);

    sal_Bool                IsModified() const;
    void                    Commit();

    //SfxListener:
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void        SetVCLSettings();
};

#endif // #ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
