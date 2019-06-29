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

#include "filtask.hxx"
#include "prov.hxx"
#include "filprp.hxx"

#include <sal/log.hxx>

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

XPropertySetInfo_impl::XPropertySetInfo_impl( TaskManager* pMyShell,const OUString& aUnqPath )
    : m_pMyShell( pMyShell ),
      m_seq( 0 )
{
    m_pMyShell->m_pProvider->acquire();

    TaskManager::ContentMap::iterator it = m_pMyShell->m_aContent.find( aUnqPath );

    TaskManager::PropertySet& properties = it->second.properties;

    m_seq.realloc( properties.size() );

    sal_Int32 count = 0;
    for( const auto& rProp : properties )
    {
        m_seq[ count++ ] = beans::Property( rProp.getPropertyName(),
                                            rProp.getHandle(),
                                            rProp.getType(),
                                            rProp.getAttributes() );
    }
}


XPropertySetInfo_impl::XPropertySetInfo_impl( TaskManager* pMyShell,const Sequence< beans::Property >& seq )
    : m_pMyShell( pMyShell ),
      m_seq( seq )
{
    m_pMyShell->m_pProvider->acquire();
}


XPropertySetInfo_impl::~XPropertySetInfo_impl()
{
    m_pMyShell->m_pProvider->release();
}


beans::Property SAL_CALL
XPropertySetInfo_impl::getPropertyByName( const OUString& aName )
{
    auto pProp = std::find_if(m_seq.begin(), m_seq.end(),
        [&aName](const beans::Property& rProp) { return rProp.Name == aName; });
    if (pProp != m_seq.end())
        return *pProp;

    throw beans::UnknownPropertyException( THROW_WHERE );
}


Sequence< beans::Property > SAL_CALL
XPropertySetInfo_impl::getProperties()
{
  return m_seq;
}


sal_Bool SAL_CALL
XPropertySetInfo_impl::hasPropertyByName( const OUString& aName )
{
    return std::any_of(m_seq.begin(), m_seq.end(),
        [&aName](const beans::Property& rProp) { return rProp.Name == aName; });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
