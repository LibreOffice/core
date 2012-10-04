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



#ifndef _WEBDAV_UCP_PROPERTYMAP_HXX
#define _WEBDAV_UCP_PROPERTYMAP_HXX

#include <hash_set>
#include <com/sun/star/beans/Property.hpp>

namespace http_dav_ucp {

//=========================================================================

struct equalPropertyName
{
  bool operator()( const ::com::sun::star::beans::Property & p1,
                   const ::com::sun::star::beans::Property & p2 ) const
  {
        return !!( p1.Name == p2.Name );
  }
};

struct hashPropertyName
{
    size_t operator()( const ::com::sun::star::beans::Property & p ) const
    {
        return p.Name.hashCode();
    }
};

typedef std::hash_set
<
    ::com::sun::star::beans::Property,
    hashPropertyName,
    equalPropertyName
>
PropertyMap;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
