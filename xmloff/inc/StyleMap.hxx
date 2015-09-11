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

#ifndef INCLUDED_XMLOFF_INC_STYLEMAP_HXX
#define INCLUDED_XMLOFF_INC_STYLEMAP_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <unordered_map>

struct StyleNameKey_Impl
{
    sal_uInt16 m_nFamily;
    OUString m_aName;

    inline StyleNameKey_Impl( sal_uInt16 nFamily,
                               const OUString& rName ) :
        m_nFamily( nFamily ),
        m_aName( rName )
    {
    }

    inline StyleNameKey_Impl() :
        m_nFamily( 0 )
    {
    }
};

struct StyleNameHash_Impl
{
    inline size_t operator()( const StyleNameKey_Impl& r ) const;
    inline bool operator()( const StyleNameKey_Impl& r1,
                               const StyleNameKey_Impl& r2 ) const;
};

inline size_t StyleNameHash_Impl::operator()( const StyleNameKey_Impl& r ) const
{
    return static_cast< size_t >( r.m_nFamily ) +
           static_cast< size_t >( r.m_aName.hashCode() );
}

inline bool StyleNameHash_Impl::operator()(
        const StyleNameKey_Impl& r1,
        const StyleNameKey_Impl& r2 ) const
{
    return r1.m_nFamily == r2.m_nFamily && r1.m_aName == r2.m_aName;
}

class StyleMap :
    public ::cppu::WeakImplHelper< ::com::sun::star::lang::XUnoTunnel>,
    public std::unordered_map< StyleNameKey_Impl, OUString,
                            StyleNameHash_Impl, StyleNameHash_Impl >
{

public:

    StyleMap();
    virtual ~StyleMap();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    static StyleMap* getImplementation(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > ) throw();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
                const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // INCLUDED_XMLOFF_INC_STYLEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
