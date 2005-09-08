/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asiancfg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:16:39 $
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
#ifndef _SVX_ASIANCFG_HXX
#define _SVX_ASIANCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace com{namespace sun{namespace star{
namespace lang{
    struct Locale;
}}}}
//-----------------------------------------------------------------------------
struct SvxAsianConfig_Impl;
class SVX_DLLPUBLIC SvxAsianConfig : public utl::ConfigItem
{
    SvxAsianConfig_Impl* pImpl;

public:
    SvxAsianConfig(sal_Bool bEnableNotify = sal_True);
    virtual ~SvxAsianConfig();

    void            Load();
    virtual void    Commit();
    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    sal_Bool    IsKerningWesternTextOnly() const;
    void        SetKerningWesternTextOnly(sal_Bool bSet);

    sal_Int16   GetCharDistanceCompression() const;
    void        SetCharDistanceCompression(sal_Int16 nSet);

    com::sun::star::uno::Sequence<com::sun::star::lang::Locale>
                GetStartEndCharLocales();

    sal_Bool    GetStartEndChars( const com::sun::star::lang::Locale& rLocale,
                                    rtl::OUString& rStartChars,
                                    rtl::OUString& rEndChars );
    void        SetStartEndChars( const com::sun::star::lang::Locale& rLocale,
                                    const rtl::OUString* pStartChars,
                                    const rtl::OUString* pEndChars );
};

#endif

