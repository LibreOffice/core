/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oneToOneMapping.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:39:08 $
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
#ifndef INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX
#define INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX

#include <utility>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

typedef std::pair< sal_Unicode, sal_Unicode > OneToOneMappingTable_t;

#define MAKE_PAIR(item1,item2) std::make_pair< sal_Unicode, sal_Unicode >((sal_Unicode)item1,(sal_Unicode)item2)

class oneToOneMapping
{
public:
        oneToOneMapping(OneToOneMappingTable_t *_table, const size_t _bytes);
        ~oneToOneMapping();

        // make index for fast search
        void makeIndex();
        // binary search / idex search
        sal_Unicode find(const sal_Unicode key) const;

        // translator
        sal_Unicode operator[] (const sal_Unicode key) const { return find (key); };

protected:
        OneToOneMappingTable_t *table;
        size_t max_size;
        int *index[256];
        sal_Bool hasIndex;
};

} } } }

#endif // _I18N_TRANSLITERATION_ONETOONEMAPPING_HXX_
