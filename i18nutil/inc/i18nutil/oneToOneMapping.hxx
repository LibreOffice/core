/*************************************************************************
 *
 *  $RCSfile: oneToOneMapping.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 12:25:59 $
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
