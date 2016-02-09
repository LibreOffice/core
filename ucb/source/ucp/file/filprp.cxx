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

#include "shell.hxx"
#include "prov.hxx"
#include "filprp.hxx"

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;

#include "filinl.hxx"

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

XPropertySetInfo_impl::XPropertySetInfo_impl( shell* pMyShell,const OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
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


beans::Property SAL_CALL
XPropertySetInfo_impl::getPropertyByName(
                     const OUString& aName )
  throw( beans::UnknownPropertyException,
     RuntimeException, std::exception)
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return m_seq[i];

  throw beans::UnknownPropertyException( THROW_WHERE );
}


Sequence< beans::Property > SAL_CALL
XPropertySetInfo_impl::getProperties(
                    void )
  throw( RuntimeException, std::exception )
{
  return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfo_impl::hasPropertyByName(
                     const OUString& aName )
  throw( RuntimeException, std::exception )
{
  for( sal_Int32 i = 0; i < m_seq.getLength(); ++i )
    if( m_seq[i].Name == aName ) return true;
  return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
