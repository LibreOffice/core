/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optionsdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:30:12 $
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
#ifndef INCLUDED_SVTOOLS_OPTIONSDLG_HXX
#define INCLUDED_SVTOOLS_OPTIONSDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

class SvtOptionsDlgOptions_Impl;

class SVL_DLLPUBLIC SvtOptionsDialogOptions: public svt::detail::Options
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

