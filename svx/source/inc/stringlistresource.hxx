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

#ifndef INCLUDED_SVX_SOURCE_INC_STRINGLISTRESOURCE_HXX
#define INCLUDED_SVX_SOURCE_INC_STRINGLISTRESOURCE_HXX

#include <tools/rc.hxx>
#include <svx/svxdllapi.h>

#include <memory>


namespace svx
{

    /** loads a list of strings from a resource, where the resource is of type RSC_RESOURCE,
        and has sub resources of type string, numbered from 1 to n
    */
    class StringListResource : public Resource
    {
    public:
        SVX_DLLPUBLIC StringListResource( const ResId& _rResId );
        SVX_DLLPUBLIC ~StringListResource();

        inline void get( ::std::vector< OUString >& _rStrings )
        {
            _rStrings = m_aStrings;
        }

        const OUString& operator[]( size_t _index ) const { return m_aStrings[ _index ]; }

    private:
        ::std::vector< OUString > m_aStrings;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_STRINGLISTRESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
