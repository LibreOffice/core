/*************************************************************************
 *
 *  $RCSfile: locale.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:24 $
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

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _RTL_LOCALE_H_
#include <rtl/locale.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

static sal_Int32 RTL_HASHTABLE_SIZE[] =
{
    7, 31, 127, 251, 509, 1021, 2039, 4093
};

typedef struct rtl_hashentry RTL_HASHENTRY;

struct rtl_hashentry
{
    rtl_Locale* Entry;
    RTL_HASHENTRY* Next;
};

typedef struct rtl_hashtable
{
    sal_Int8        iSize;
    sal_Int32       Size;
    sal_Int32       Elements;
    RTL_HASHENTRY** Table;
} RTL_HASHTABLE;

static RTL_HASHTABLE* pLocaleTable = NULL;

static rtl_Locale* pDefaultLocale = NULL;


void rtl_hashentry_destroy(RTL_HASHENTRY* entry)
{
    rtl_uString_release(entry->Entry->Language);
    rtl_uString_release(entry->Entry->Country);
    rtl_uString_release(entry->Entry->Variant);
    if (entry->Next)
        rtl_hashentry_destroy(entry->Next);

    rtl_freeMemory(entry->Entry);
}

void rtl_hashtable_destroy(RTL_HASHTABLE* table)
{
    sal_Int32 size = 0;

    if (!table)
        return;

    size = table->Size;

    while (size)
    {
        if (table->Table[size - 1])
            rtl_hashentry_destroy(table->Table[size - 1]);
        size--;
    }

    rtl_freeMemory(table->Table);
    rtl_freeMemory(table);
}

void rtl_hashtable_init(RTL_HASHTABLE** table, sal_Int8 sizeIndex)
{
    sal_Int32 nSize = RTL_HASHTABLE_SIZE[sizeIndex];

    if (*table)
        rtl_hashtable_destroy(*table);

    *table = (RTL_HASHTABLE*)rtl_allocateMemory( sizeof(RTL_HASHTABLE) );

    (*table)->iSize = sizeIndex;
    (*table)->Size = nSize;
    (*table)->Elements = 0;
    (*table)->Table = (RTL_HASHENTRY**)rtl_allocateMemory( (*table)->Size * sizeof(RTL_HASHENTRY*) );

    while (nSize)
    {
        (*table)->Table[nSize - 1] = NULL;
        nSize--;
    }
}

sal_Int32 rtl_hashfunc(RTL_HASHTABLE* table, sal_Int32 key)
{
    return (key % table->Size);
}

sal_Bool rtl_hashtable_grow(RTL_HASHTABLE** table);

rtl_Locale* rtl_hashtable_add(RTL_HASHTABLE** table, rtl_Locale* value)
{
    sal_Int32 key = 0;

    if (!(*table))
        return NULL;

    if ((*table)->Elements > ((*table)->Size / 2))
        rtl_hashtable_grow(table);

    key = rtl_hashfunc(*table, value->HashCode);

    if (!(*table)->Table[key])
    {
        RTL_HASHENTRY *newEntry = (RTL_HASHENTRY*)rtl_allocateMemory( sizeof(RTL_HASHENTRY) );
        newEntry->Entry = value;
        newEntry->Next = NULL;
        (*table)->Table[key] = newEntry;
        (*table)->Elements++;
        return NULL;
    } else
    {
        RTL_HASHENTRY *pEntry = (*table)->Table[key];
        RTL_HASHENTRY *newEntry = NULL;

        while (pEntry)
        {
            if (value->HashCode == pEntry->Entry->HashCode)
                return pEntry->Entry;

            if (!pEntry->Next)
                break;

            pEntry = pEntry->Next;
        }

        newEntry = (RTL_HASHENTRY*)rtl_allocateMemory( sizeof(RTL_HASHENTRY) );
        newEntry->Entry = value;
        newEntry->Next = NULL;
        pEntry->Next = newEntry;
        (*table)->Elements++;
        return NULL;
    }
}

sal_Bool rtl_hashtable_grow(RTL_HASHTABLE** table)
{
    RTL_HASHTABLE* pNewTable = NULL;
    sal_Int32 i = 0;

    rtl_hashtable_init(&pNewTable, (sal_Int8)((*table)->iSize + 1));

    while (i < (*table)->Size)
    {
        if ((*table)->Table[i])
        {
            RTL_HASHENTRY *pNext;
            RTL_HASHENTRY *pEntry = (*table)->Table[i];

            rtl_hashtable_add(&pNewTable, pEntry->Entry);

            while (pEntry->Next)
            {
                rtl_hashtable_add(&pNewTable, pEntry->Next->Entry);
                pNext = pEntry->Next;
                rtl_freeMemory(pEntry);
                pEntry = pNext;
            }

            rtl_freeMemory(pEntry);
        }
        i++;
    }

    rtl_freeMemory((*table)->Table);
    rtl_freeMemory((*table));
    (*table) = pNewTable;

    return sal_True;
}

sal_Bool rtl_hashtable_find(sal_Int32 key, sal_Int32 hashCode, rtl_Locale** pValue)
{
    if (!pLocaleTable)
        return sal_False;

    if (pLocaleTable->Table[key])
    {
        RTL_HASHENTRY *pEntry = pLocaleTable->Table[key];

        while (pEntry && hashCode != pEntry->Entry->HashCode)
            pEntry = pEntry->Next;

        if (pEntry)
            *pValue = pEntry->Entry;
        else
            return sal_False;
    } else
        return sal_False;

    return sal_True;
}

/*************************************************************************
 *  rtl_locale_register
 */
rtl_Locale * SAL_CALL rtl_locale_register( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant )
{
    sal_Unicode c = 0;
    rtl_uString* sLanguage = NULL;
    rtl_uString* sCountry = NULL;
    rtl_uString* sVariant = NULL;
    rtl_Locale *newLocale = NULL;
    sal_Int32 hashCode = -1;
    sal_Int32 key = 0;

    if ( !country )
        country = &c;
    if ( !variant )
        variant = &c;

    if (!pLocaleTable)
        rtl_hashtable_init(&pLocaleTable, 1);

    hashCode = rtl_ustr_hashCode(language) ^ rtl_ustr_hashCode(country) ^ rtl_ustr_hashCode(variant);
    key = rtl_hashfunc(pLocaleTable, hashCode);

    if (rtl_hashtable_find(key, hashCode, &newLocale))
        return newLocale;

    rtl_uString_newFromStr(&sLanguage, language);
    rtl_uString_newFromStr(&sCountry, country);
    rtl_uString_newFromStr(&sVariant, variant);

    newLocale = (rtl_Locale*)rtl_allocateMemory( sizeof(rtl_Locale) );

    newLocale->Language = sLanguage;
    newLocale->Country = sCountry;
    newLocale->Variant = sVariant;
    newLocale->HashCode = hashCode;

    rtl_hashtable_add(&pLocaleTable, newLocale);

    return newLocale;
}

/*************************************************************************
 *  rtl_locale_getDefault
 */
rtl_Locale * SAL_CALL rtl_locale_getDefault()
{
    return pDefaultLocale;
}

/*************************************************************************
 *  rtl_locale_setDefault
 */
void SAL_CALL rtl_locale_setDefault( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant )
{
    pDefaultLocale = rtl_locale_register(language, country, variant);
}

/*************************************************************************
 *  rtl_locale_getLanguage
 */
rtl_uString * SAL_CALL rtl_locale_getLanguage( rtl_Locale * This )
{
    rtl_uString_acquire(This->Language);
    return This->Language;
}

/*************************************************************************
 *  rtl_locale_getCountry
 */
rtl_uString * SAL_CALL rtl_locale_getCountry( rtl_Locale * This )
{
    rtl_uString_acquire(This->Country);
    return This->Country;
}

/*************************************************************************
 *  rtl_locale_getVariant
 */
rtl_uString * SAL_CALL rtl_locale_getVariant( rtl_Locale * This )
{
    rtl_uString_acquire(This->Variant);
    return This->Variant;
}

/*************************************************************************
 *  rtl_locale_hashCode
 */
sal_Int32 SAL_CALL rtl_locale_hashCode( rtl_Locale * This )
{
    return This->HashCode;
}

/*************************************************************************
 *  rtl_locale_equals
 */
sal_Int32 SAL_CALL rtl_locale_equals( rtl_Locale * This, rtl_Locale * obj  )
{
    return This == obj;
}

