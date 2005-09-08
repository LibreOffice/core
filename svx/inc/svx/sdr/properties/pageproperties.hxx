/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pageproperties.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:08:45 $
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

#ifndef _SDR_PROPERTIES_PAGEPROPERTIES_HXX
#define _SDR_PROPERTIES_PAGEPROPERTIES_HXX

#ifndef _SDR_PROPERTIES_EMPTYPROPERTIES_HXX
#include <svx/sdr/properties/emptyproperties.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        class PageProperties : public EmptyProperties
        {
        protected:
            // create a new object specific itemset with object specific ranges.
            virtual SfxItemSet& CreateObjectSpecificItemSet(SfxItemPool& pPool);

            // Do the ItemChange, may do special handling
            virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);

        public:
            // basic constructor
            PageProperties(SdrObject& rObj);

            // constructor for copying, but using new object
            PageProperties(const PageProperties& rProps, SdrObject& rObj);

            // destructor
            virtual ~PageProperties();

            // Clone() operator, normally just calls the local copy constructor
            virtual BaseProperties& Clone(SdrObject& rObj) const;

            // get itemset. Overloaded here to allow creating the empty itemset
            // without asserting
            virtual const SfxItemSet& GetObjectItemSet() const;

            // get the installed StyleSheet
            virtual SfxStyleSheet* GetStyleSheet() const;

            // clear single item
            virtual void ClearObjectItem(const sal_uInt16 nWhich = 0);
        };
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_PROPERTIES_PAGEPROPERTIES_HXX

// eof


