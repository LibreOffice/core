/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cassert>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include <dlfcn.h>

#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <typelib/typedescription.h>

#include <rtti.hxx>
#include <share.hxx>

namespace {

class RTTI
{
    typedef std::unordered_map< OUString, std::type_info *, OUStringHash > t_rtti_map;

    osl::Mutex m_mutex;
    t_rtti_map m_rttis;
    t_rtti_map m_generatedRttis;

    void * m_hApp;

public:
    RTTI();
    ~RTTI();

    std::type_info * getRTTI(typelib_TypeDescription const &);
};

RTTI::RTTI()
#if defined(FREEBSD) && __FreeBSD_version < 702104
    : m_hApp( dlopen( 0, RTLD_NOW | RTLD_GLOBAL ) )
#else
    : m_hApp( dlopen( nullptr, RTLD_LAZY ) )
#endif
{
}

RTTI::~RTTI()
{
    dlclose( m_hApp );
}

std::type_info * RTTI::getRTTI(typelib_TypeDescription const & pTypeDescr)
{
    std::type_info * rtti;

    OUString const & unoName = OUString::unacquired(&pTypeDescr.pTypeName);

    osl::MutexGuard guard( m_mutex );
    t_rtti_map::const_iterator iFind( m_rttis.find( unoName ) );
    if (iFind == m_rttis.end())
    {
        // RTTI symbol
        OStringBuffer buf( 64 );
        buf.append( "_ZTIN" );
        sal_Int32 index = 0;
        do
        {
            OUString token( unoName.getToken( 0, '.', index ) );
            buf.append( token.getLength() );
            OString c_token( OUStringToOString( token, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( c_token );
        }
        while (index >= 0);
        buf.append( 'E' );

        OString symName( buf.makeStringAndClear() );
#if defined(FREEBSD) && __FreeBSD_version < 702104 /* #i22253# */
        rtti = (std::type_info *)dlsym( RTLD_DEFAULT, symName.getStr() );
#else
        rtti = static_cast<std::type_info *>(dlsym( m_hApp, symName.getStr() ));
#endif

        if (rtti)
        {
            std::pair< t_rtti_map::iterator, bool > insertion (
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            SAL_WARN_IF( !insertion.second, "bridges", "key " << unoName << " already in rtti map" );
        }
        else
        {
            // try to lookup the symbol in the generated rtti map
            t_rtti_map::const_iterator iFind2( m_generatedRttis.find( unoName ) );
            if (iFind2 == m_generatedRttis.end())
            {
                // we must generate it !
                // symbol and rtti-name is nearly identical,
                // the symbol is prefixed with _ZTI
                char const * rttiName = symName.getStr() +4;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr,"generated rtti for %s\n", rttiName );
#endif
                switch (pTypeDescr.eTypeClass) {
                case typelib_TypeClass_EXCEPTION:
                    {
                        typelib_CompoundTypeDescription const & ctd
                            = reinterpret_cast<
                                typelib_CompoundTypeDescription const &>(
                                    pTypeDescr);
                        if (ctd.pBaseTypeDescription)
                        {
                            // ensure availability of base
                            std::type_info * base_rtti = getRTTI(
                                ctd.pBaseTypeDescription->aBase);
                            rtti = new __cxxabiv1::__si_class_type_info(
                                strdup( rttiName ), static_cast<__cxxabiv1::__class_type_info *>(base_rtti) );
                        }
                        else
                        {
                            // this class has no base class
                            rtti = new __cxxabiv1::__class_type_info( strdup( rttiName ) );
                        }
                        break;
                    }
                case typelib_TypeClass_INTERFACE:
                    {
                        typelib_InterfaceTypeDescription const & itd
                            = reinterpret_cast<
                                typelib_InterfaceTypeDescription const &>(
                                    pTypeDescr);
                        std::vector<std::type_info *> bases;
                        for (sal_Int32 i = 0; i != itd.nBaseTypes; ++i) {
                            bases.push_back(getRTTI(itd.ppBaseTypes[i]->aBase));
                        }
                        switch (itd.nBaseTypes) {
                        case 0:
                            rtti = new __cxxabiv1::__class_type_info(
                                strdup(rttiName));
                            break;
                        case 1:
                            rtti = new __cxxabiv1::__si_class_type_info(
                                strdup(rttiName),
                                static_cast<__cxxabiv1::__class_type_info *>(
                                    bases[0]));
                            break;
                        default:
                            {
                                char * pad = new char[
                                    sizeof (__cxxabiv1::__vmi_class_type_info)
                                    + ((itd.nBaseTypes - 1)
                                       * sizeof (
                                           __cxxabiv1::__base_class_type_info))];
                                __cxxabiv1::__vmi_class_type_info * info
                                    = new(pad)
                                        __cxxabiv1::__vmi_class_type_info(
                                            strdup(rttiName),
                                            __cxxabiv1::__vmi_class_type_info::__flags_unknown_mask);
                                info->__base_count = itd.nBaseTypes;
                                for (sal_Int32 i = 0; i != itd.nBaseTypes; ++i)
                                {
                                    info->__base_info[i].__base_type
                                        = static_cast<
                                            __cxxabiv1::__class_type_info *>(
                                                bases[i]);
                                    info->__base_info[i].__offset_flags
                                        = (__cxxabiv1::__base_class_type_info::__public_mask
                                           | ((8 * i) << __cxxabiv1::__base_class_type_info::__offset_shift));
                                }
                                rtti = info;
                                break;
                            }
                        }
                        break;
                    }
                default:
                    assert(false); // cannot happen
                }
                if (rtti != nullptr) {
                    std::pair< t_rtti_map::iterator, bool > insertion (
                        m_generatedRttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
                    SAL_WARN_IF( !insertion.second, "bridges", "key " << unoName << " already in generated rtti map" );
                }
            }
            else // taking already generated rtti
            {
                rtti = iFind2->second;
            }
        }
    }
    else
    {
        rtti = iFind->second;
    }

    return rtti;
}

struct theRttiFactory: public rtl::Static<RTTI, theRttiFactory> {};

}

std::type_info * x86_64::getRtti(typelib_TypeDescription const & type) {
    return theRttiFactory::get().getRTTI(type);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
