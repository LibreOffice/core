/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibilityoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:10:56 $
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
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#define INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

class SvtAccessibilityOptions_Impl;

class SVT_DLLPUBLIC SvtAccessibilityOptions:
    public svt::detail::Options, public SfxBroadcaster, private SfxListener
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

