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
#ifndef _SVX_ASIANCFG_HXX
#define _SVX_ASIANCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
namespace com{namespace sun{namespace star{
namespace lang{
    struct Locale;
}}}}
namespace binfilter {

//STRIP008 namespace com{namespace sun{namespace star{
//STRIP008 namespace lang{
//STRIP008 	struct Locale;
//STRIP008 }}}}
//-----------------------------------------------------------------------------
struct SvxAsianConfig_Impl;
class SvxAsianConfig : public ::utl::ConfigItem
{
    SvxAsianConfig_Impl* pImpl;

public:
    SvxAsianConfig(sal_Bool bEnableNotify = sal_True);
    virtual ~SvxAsianConfig();

void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
void Commit();

    void 			Load();

    sal_Bool 	IsKerningWesternTextOnly() const;

    sal_Int16	GetCharDistanceCompression() const;
    void 		SetCharDistanceCompression(sal_Int16 nSet);

    ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale>
                GetStartEndCharLocales();

};

}//end of namespace binfilter
#endif

