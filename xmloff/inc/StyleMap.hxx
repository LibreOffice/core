/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_STYLEMAP_HXX
#define _XMLOFF_STYLEMAP_HXX

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase1.hxx>
#include <boost/unordered_map.hpp>

struct StyleNameKey_Impl
{
    sal_uInt16 m_nFamily;
    ::rtl::OUString m_aName;

    inline StyleNameKey_Impl( sal_uInt16 nFamily,
                               const ::rtl::OUString& rName ) :
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
    public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XUnoTunnel>,
    public ::boost::unordered_map< StyleNameKey_Impl, ::rtl::OUString,
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
                const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
};

#endif  //  _XMLOFF_STYLEMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
