/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontsubstconfig.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:32:03 $
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
#ifndef _SVT_FONTSUBSTCONFIG_HXX
#define _SVT_FONTSUBSTCONFIG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

struct SvtFontSubstConfig_Impl;

//-----------------------------------------------------------------------------
struct SubstitutionStruct
{
    rtl::OUString   sFont;
    rtl::OUString   sReplaceBy;
    sal_Bool        bReplaceAlways;
    sal_Bool        bReplaceOnScreenOnly;
};
//-----------------------------------------------------------------------------
class SVT_DLLPUBLIC SvtFontSubstConfig : public utl::ConfigItem
{
    sal_Bool                    bIsEnabled;
    SvtFontSubstConfig_Impl*    pImpl;
public:
    SvtFontSubstConfig();
    virtual ~SvtFontSubstConfig();

    virtual void            Commit();

    sal_Bool                IsEnabled() const {return bIsEnabled;}
    void                    Enable(sal_Bool bSet)  {bIsEnabled = bSet; SetModified();}

    sal_Int32                   SubstitutionCount() const;
    void                        ClearSubstitutions();
    const SubstitutionStruct*   GetSubstitution(sal_Int32 nPos);
    void                        AddSubstitution(const SubstitutionStruct&   rToAdd);
    void                        Apply();
};

#endif



