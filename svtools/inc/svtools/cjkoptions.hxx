/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cjkoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:13:20 $
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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#define _SVTOOLS_CJKOPTIONS_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

class SvtCJKOptions_Impl;

// class SvtCJKOptions --------------------------------------------------

class SVT_DLLPUBLIC SvtCJKOptions: public svt::detail::Options
{
private:
    SvtCJKOptions_Impl*    pImp;

public:

    enum EOption
    {
        E_CJKFONT,
        E_VERTICALTEXT,
        E_ASIANTYPOGRAPHY,
        E_JAPANESEFIND,
        E_RUBY,
        E_CHANGECASEMAP,
        E_DOUBLELINES,
        E_EMPHASISMARKS,
        E_VERTICALCALLOUT,
        E_ALL               // special one for IsAnyEnabled()/SetAll() functionality
    };

    // bDontLoad is for referencing purposes only
    SvtCJKOptions(sal_Bool bDontLoad = sal_False);
    virtual ~SvtCJKOptions();

    sal_Bool IsCJKFontEnabled() const;
    sal_Bool IsVerticalTextEnabled() const;
    sal_Bool IsAsianTypographyEnabled() const;
    sal_Bool IsJapaneseFindEnabled() const;
    sal_Bool IsRubyEnabled() const;
    sal_Bool IsChangeCaseMapEnabled() const;
    sal_Bool IsDoubleLinesEnabled() const;
    sal_Bool IsEmphasisMarksEnabled() const;
    sal_Bool IsVerticalCallOutEnabled() const;

    void        SetAll(sal_Bool bSet);
    sal_Bool    IsAnyEnabled() const;
    sal_Bool    IsReadOnly(EOption eOption) const;
};

#endif // _SVTOOLS_CJKOPTIONS_HXX

