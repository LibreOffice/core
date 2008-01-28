/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oneToOneMapping.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:32:00 $
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

#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

oneToOneMapping::oneToOneMapping( OneToOneMappingTable_t *rpTable, const size_t rnBytes, const size_t rnUnitSize )
    : mpTable( rpTable ),
      mnSize( rnBytes / rnUnitSize )
{
}

oneToOneMapping::~oneToOneMapping()
{
}

sal_Unicode oneToOneMapping::find(const sal_Unicode nKey) const
{
    if( mpTable )
    {
        // binary search
        int bottom = 0;
        int top = mnSize - 1;
        int current;

        for (;;) {
            current = (top + bottom) / 2;
            if( nKey < mpTable[current].first )
                top = current - 1;
            else if( nKey > mpTable[current].first )
                bottom = current + 1;
            else
                return mpTable[current].second;

            if( bottom > top )
                return sal_Unicode( nKey );
        }
    }
    else
        return sal_Unicode( nKey );
}

oneToOneMappingWithFlag::oneToOneMappingWithFlag( UnicodePairWithFlag *rpTableWF, const size_t rnSize, const UnicodePairFlag rnFlag )
    : oneToOneMapping( NULL, rnSize, sizeof(UnicodePairWithFlag) ),
      mpTableWF ( rpTableWF ),
      mnFlag    ( rnFlag ),
      mbHasIndex( sal_False )
{
}

oneToOneMappingWithFlag::~oneToOneMappingWithFlag()
{
    if( mbHasIndex )
        for( int i = 0; i < 256; i++ )
            if( mpIndex[i] )
                delete [] mpIndex[i];
}


void oneToOneMappingWithFlag::makeIndex()
{
    if( !mbHasIndex && mpTableWF )
    {
        int i, j, high, low, current = -1;

        for( i = 0; i < 256; i++ )
            mpIndex[i] = NULL;

        for( size_t k = 0; k < mnSize; k++ )
        {
            high = (mpTableWF[k].first >> 8) & 0xFF;
            low  = (mpTableWF[k].first)      & 0xFF;
            if( high != current )
            {
                current = high;
                mpIndex[high] = new UnicodePairWithFlag*[256];

                for( j = 0; j < 256; j++ )
                    mpIndex[high][j] = NULL;
            }
            mpIndex[high][low] = &mpTableWF[k];
        }

        mbHasIndex = sal_True;
    }
}

sal_Unicode oneToOneMappingWithFlag::find( const sal_Unicode nKey ) const
{
    if( mpTableWF )
    {
        if( mbHasIndex )
        {
            // index search
            int high, low;
            high = (nKey >> 8) & 0xFF;
            low = nKey & 0xFF;
            if( mpIndex[high] != NULL &&
                mpIndex[high][low] != NULL &&
                mpIndex[high][low]->flag & mnFlag )
                return mpIndex[high][low]->second;
            else
                return sal_Unicode( nKey );
        }
        else
        {
            // binary search
            int bottom = 0;
            int top = mnSize - 1;
            int current;

            for (;;) {
                current = (top + bottom) / 2;
                if( nKey < mpTableWF[current].first )
                    top = current - 1;
                else if( nKey > mpTableWF[current].first )
                    bottom = current + 1;
                else
                {
                    if( mpTableWF[current].flag & mnFlag )
                        return mpTableWF[current].second;
                    else
                        return sal_Unicode( nKey );
                }

                if( bottom > top )
                    return sal_Unicode( nKey );
            }
        }
    }
    else
        return sal_Unicode( nKey );
}


} } } }
