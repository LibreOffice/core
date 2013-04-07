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

#ifndef _CONNECTIVITY_MAB_NAMEMAPPER_HXX_
#define _CONNECTIVITY_MAB_NAMEMAPPER_HXX_

#include <map>

// Mozilla includes
#include <MNSInclude.hxx>

// Star Includes
#include <rtl/ustring.hxx>

namespace connectivity
{
    namespace mozab
    {
        class MNameMapper
        {
            private:

                struct ltstr
                {
                    bool operator()( const OUString &s1, const OUString &s2) const;
                };


                typedef ::std::multimap< OUString, nsIAbDirectory *, ltstr > dirMap;
                typedef ::std::multimap< OUString, nsIAbDirectory *, ltstr > uriMap;

                static MNameMapper    *instance;
                dirMap                     *mDirMap;
                uriMap                     *mUriMap;

                //clear dirs
                void clear();

            public:
                static MNameMapper* getInstance();

                MNameMapper();
                ~MNameMapper();

                // May modify the name passed in so that it's unique
                nsresult add( OUString& str, nsIAbDirectory* abook );

                //reset dirs
                void reset();

                // Get the directory corresponding to str
                bool getDir( const OUString& str, nsIAbDirectory* *abook );

        };

    }
}

#endif //_CONNECTIVITY_MAB_NAMEMAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
