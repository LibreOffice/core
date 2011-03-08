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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "shell.hxx"
#include "prov.hxx"
#include "filprp.hxx"

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;


#include "filinl.hxx"


XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const rtl::OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider ),
      m_count( 0 ),
      m_seq( 0 )
{
    m_pMyShell->m_pProvider->acquire();

    shell::ContentMap::iterator it = m_pMyShell->m_aContent.find( aUnqPath );

    shell::PropertySet& properties = *(it->second.properties);
    shell::PropertySet::iterator it1 = properties.begin();

    m_seq.realloc( properties.size() );

    while( it1 != properties.end() )
    {
        m_seq[ m_count++ ] = beans::Property( it1->getPropertyName(),
                                              it1->getHandle(),
                                              it1->getType(),
                                              it1->getAttributes() );
        ++it1;
    }
}


XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const Sequence< beans::Property >& seq )
    : m_pMyShell( pMyShell ),
      m_count( seq.getLength() ),
      m_seq( seq )
{
    m_pMyShell->m_pProvider->acquire();
}


XPropertySetInfo_impl::~XPropertySetInfo_impl()
{
    m_pMyShell->m_pProvider->release();
}


void SAL_CALL
XPropertySetInfo_impl::acquire(
                  void )
  throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
XPropertySetInfo_impl::release(
              void )
  throw()
{
  OWeakObject::release();
}



XTYPEPROVIDER_IMPL_2( XPropertySetInfo_impl,
                         lang::XTypeProvider,
                      beans::XPropertySetInfo )


Any SAL_CALL
XPropertySetInfo_impl::queryInterface(
                     const Type& rType )
  throw( RuntimeException )
{
  Any aRet = cppu::queryInterface( rType,
                    SAL_STATIC_CAST( lang::XTypeProvider*,this),
                    SAL_STATIC_CAST( beans::XPropertySetInfo*,this) );
  return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


beans::Property SAL_CALL
XPropertySetInfo_impl::getPropertyByName(
                     const rtl::OUString& aName )
  throw( beans::UnknownPropertyException,
     RuntimeException)
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return m_seq[i];

  throw beans::UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}



Sequence< beans::Property > SAL_CALL
XPropertySetInfo_impl::getProperties(
                    void )
  throw( RuntimeException )
{
  return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfo_impl::hasPropertyByName(
                     const rtl::OUString& aName )
  throw( RuntimeException )
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return true;
  return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
