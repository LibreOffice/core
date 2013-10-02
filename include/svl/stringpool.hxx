/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SVL_STRINGPOOL_HXX
#define SVL_STRINGPOOL_HXX

#include "svl/svldllapi.h"
#include "rtl/ustring.hxx"

#include <boost/unordered_set.hpp>

namespace svl {

class SVL_DLLPUBLIC StringPool
{
    typedef boost::unordered_set<OUString, OUStringHash> StrHashType;
    StrHashType maStrPool;
public:
    StringPool();

    rtl_uString* intern( const OUString& rStr );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
