/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef TAGHELPER_HXX
#define TAGHELPER_HXX

#include <map>
#include <sal/types.h>
#include <testshl/nocopy.hxx>
#include <cppunit/tagvalues.hxx>

/* Tags are a general mechanism of extensible data arrays for parameter
 * specification and property inquiry. In practice, tags are used in arrays,
 * or chain of arrays.
 *
 */

typedef sal_IntPtr TagData;

//# struct TagItem
//# {
//#     Tag     ti_Tag;     /* identifies the type of data */
//#     TagData ti_Data;    /* type-specific data          */
//# };

/* constants for Tag.ti_Tag, control tag values */
#define TAG_DONE   (Tag(0L))     /* terminates array of TagItems. ti_Data unused */
#define TAG_END    (Tag(0L))     /* synonym for TAG_DONE             */
#define TAG_IGNORE (Tag(1L))     /* ignore this item, not end of array       */
#define TAG_MORE   (Tag(2L))     /* ti_Data is pointer to another array of TagItems
               * note that this tag terminates the current array
               */
#define TAG_SKIP   (Tag(3L))      /* skip this and the next ti_Data items     */

/* differentiates user tags from control tags */
#define TAG_USER   (Tag(1L<<31))

// -----------------------------------------------------------------------------
class TagHelper /* : NOCOPY */
{
    typedef std::map<Tag, TagData> TagItems;
    TagItems m_aTagItems;

public:
    TagHelper(){}
    void insert(Tag _nTag, TagData _nData)
    {
        m_aTagItems[_nTag] = _nData;
    }
    // const TagItems& get() const { return m_aTagItems; }

    TagData GetTagData(Tag _aTagValue, TagData _aDefaultValue = 0 /* NULL */) const
    {
        TagItems::const_iterator it = m_aTagItems.find(_aTagValue);
        if (it != m_aTagItems.end())
            return (*it).second;
        else
            return _aDefaultValue;
    }
};

#endif


