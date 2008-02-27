/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: taghelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:09:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef TAGHELPER_HXX
#define TAGHELPER_HXX

#include <map>
#include <sal/types.h>
#include <cppunit/nocopy.hxx>

/* Tags are a general mechanism of extensible data arrays for parameter
 * specification and property inquiry. In practice, tags are used in arrays,
 * or chain of arrays.
 *
 */

typedef sal_IntPtr Tag;
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


