/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_FILTER_XML_CACHEDATTRACCESS_HXX__
#define __SC_FILTER_XML_CACHEDATTRACCESS_HXX__

#include "sal/types.h"

class ScDocument;

/**
 * Wrapper for accessing hidden and filtered row attributes.  It caches last
 * accessed values for a current range, to avoid fetching values for every
 * single row.
 */
class ScXMLCachedRowAttrAccess
{
    struct Cache
    {
        sal_Int32   mnTab;
        sal_Int32   mnRow1;
        sal_Int32   mnRow2;
        bool        mbValue;
        Cache();
        bool hasCache(sal_Int32 nTab, sal_Int32 nRow) const;
    };

public:
    ScXMLCachedRowAttrAccess(ScDocument* pDoc);

    bool rowHidden(sal_Int32 nTab, sal_Int32 nRow);
    bool rowFiltered(sal_Int32 nTab, sal_Int32 nRow);
private:
    Cache maHidden;
    Cache maFiltered;
    ScDocument* mpDoc;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
