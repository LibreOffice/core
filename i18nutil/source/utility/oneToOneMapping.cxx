/*************************************************************************
 *
 *  $RCSfile: oneToOneMapping.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 12:26:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

oneToOneMapping::oneToOneMapping(OneToOneMappingTable_t *_table, const size_t _bytes)
{
    table = _table;
    max_size = _bytes / sizeof(std::pair< sal_Unicode, sal_Unicode >);
    hasIndex = sal_False;
}

oneToOneMapping::~oneToOneMapping()
{
    if( hasIndex )
        for (int i = 0; i < 256; i++)
        if (index[i])
            delete index[i];
}

sal_Unicode oneToOneMapping::find(const sal_Unicode key) const
{
    if (hasIndex) {
        // index search
        int high, low;
        high = (key >> 8) & 0xFF;
        low = key & 0xFF;
        if (index[high] != (int*)0 && index[high][low] != 0) {
          return table[index[high][low] - 1].second;
        }
        else {
          return sal_Unicode(key);
        }
    } else {
        int bottom = 0;
        int top = max_size - 1;
        int current;

        for (;;) {
        current = (top + bottom) / 2;
        if (key < table[current].first) {
            top = current - 1;
        }
        else if (key > table[current].first) {
            bottom = current + 1;
        } else {
            return table[current].second;
        }
        if (bottom > top) {
            return sal_Unicode(key);
        }
        }
    }
}

void oneToOneMapping::makeIndex()
{
    if (!hasIndex) {
        int i, j, high, low, current = -1;
        hasIndex = sal_True;
        for (i = 0; i < 256; i++)
        index[i] = (int*)0;

        for (size_t k = 0; k < max_size; k++) {
        high = (table[k].first >> 8) & 0xFF;
        low = (table[k].first) & 0xFF;
        if (high != current) {
            current = high;
            index[high] = new int[256];
            for (j = 0; j < 256; j++)
            index[high][j] = 0;
        }
        index[high][low] = k + 1;
        }
    }
}

} } } }
