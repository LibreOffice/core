/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: optionsdlg.hxx,v $
 * $Revision: 1.3 $
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
#ifndef INCLUDED_unotools_OPTIONSDLG_HXX
#define INCLUDED_unotools_OPTIONSDLG_HXX

#ifndef INCLUDED_unotoolsdllapi_H
#include "unotools/unotoolsdllapi.h"
#endif
#include <tools/string.hxx>
#include <unotools/options.hxx>

class SvtOptionsDlgOptions_Impl;

class UNOTOOLS_DLLPUBLIC SvtOptionsDialogOptions: public utl::detail::Options
{
private:
    SvtOptionsDlgOptions_Impl* m_pImp;

public:
                    SvtOptionsDialogOptions();
                    virtual ~SvtOptionsDialogOptions();

    sal_Bool        IsGroupHidden   (   const String& _rGroup ) const;
    sal_Bool        IsPageHidden    (   const String& _rPage,
                                        const String& _rGroup ) const;
    sal_Bool        IsOptionHidden  (   const String& _rOption,
                                        const String& _rPage,
                                        const String& _rGroup ) const;
};

#endif

