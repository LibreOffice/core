/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_ASIANCFG_HXX
#define _SVX_ASIANCFG_HXX

#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/svldllapi.h>

namespace com{namespace sun{namespace star{
namespace lang{
    struct Locale;
}}}}
//-----------------------------------------------------------------------------
struct SvxAsianConfig_Impl;
class SVL_DLLPUBLIC SvxAsianConfig : public utl::ConfigItem
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
