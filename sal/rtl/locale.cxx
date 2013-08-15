/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "rtl/locale.h"

#include "osl/diagnose.h"
#include "rtl/alloc.h"

#include "internal/rtllifecycle.h"

static const sal_Int32 RTL_HASHTABLE_SIZE[] =
{
    7, 31, 127, 251, 509, 1021, 2039, 4093
};

struct RTL_HASHENTRY
{
    rtl_Locale* Entry;
    RTL_HASHENTRY* Next;
};

struct RTL_HASHTABLE
{
    sal_Int8        iSize;
    sal_Int32       Size;
    sal_Int32       Elements;
    RTL_HASHENTRY** Table;
};

static RTL_HASHTABLE* g_pLocaleTable = NULL;

static rtl_Locale* g_pDefaultLocale = NULL;

/*************************************************************************
 */
extern "C" void rtl_hashentry_destroy(RTL_HASHENTRY* entry)
{
    rtl_uString_release(entry->Entry->Language);
    rtl_uString_release(entry->Entry->Country);
    rtl_uString_release(entry->Entry->Variant);
    if (entry->Next)
        rtl_hashentry_destroy(entry->Next);

    rtl_freeMemory(entry->Entry);
    rtl_freeMemory(entry);
}

extern "C" void rtl_hashtable_destroy(RTL_HASHTABLE* table)
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

extern "C" void rtl_hashtable_init(RTL_HASHTABLE** table, sal_Int8 sizeIndex)
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

extern "C" sal_Int32 rtl_hashfunc(RTL_HASHTABLE* table, sal_Int32 key)
{
    return ( (sal_uInt32) key % table->Size);
}

extern "C" sal_Bool rtl_hashtable_grow(RTL_HASHTABLE** table);

extern "C" rtl_Locale* rtl_hashtable_add(RTL_HASHTABLE** table, rtl_Locale* value)
{
    sal_Int32 key = 0;

    if (!(*table))
        return NULL;

    if ((*table)->Elements > ((*table)->Size / 2))
        rtl_hashtable_grow(table);

    key = rtl_hashfunc(*table, value->HashCode);

    RTL_HASHENTRY **pEntry = &(*table)->Table[key];
    while (*pEntry)
    {
        if (value->HashCode == (*pEntry)->Entry->HashCode)
            return (*pEntry)->Entry;
        pEntry = &(*pEntry)->Next;
    }

    RTL_HASHENTRY *newEntry = (RTL_HASHENTRY*)rtl_allocateMemory( sizeof(RTL_HASHENTRY) );
    newEntry->Entry = value;
    newEntry->Next = NULL;
    *pEntry = newEntry;
    (*table)->Elements++;
    return NULL;
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

extern "C" sal_Bool rtl_hashtable_find(RTL_HASHTABLE * table, sal_Int32 key, sal_Int32 hashCode, rtl_Locale** pValue)
{
    if (!table)
        return sal_False;

    if (table->Table[key])
    {
        RTL_HASHENTRY *pEntry = table->Table[key];

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
 *  rtl_locale_init
 */
void rtl_locale_init()
{
  OSL_ASSERT(g_pLocaleTable == 0);
  rtl_hashtable_init(&g_pLocaleTable, 1);
}

/*************************************************************************
 *  rtl_locale_fini
 */
void rtl_locale_fini()
{
  if (g_pLocaleTable != 0)
  {
    rtl_hashtable_destroy (g_pLocaleTable);
    g_pLocaleTable = 0;
  }
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

    ensureLocaleSingleton();
    if (!g_pLocaleTable)
      return NULL;

    hashCode = rtl_ustr_hashCode(language) ^ rtl_ustr_hashCode(country) ^ rtl_ustr_hashCode(variant);
    key = rtl_hashfunc(g_pLocaleTable, hashCode);

    if (rtl_hashtable_find(g_pLocaleTable, key, hashCode, &newLocale))
        return newLocale;

    rtl_uString_newFromStr(&sLanguage, language);
    rtl_uString_newFromStr(&sCountry, country);
    rtl_uString_newFromStr(&sVariant, variant);

    newLocale = (rtl_Locale*)rtl_allocateMemory( sizeof(rtl_Locale) );

    newLocale->Language = sLanguage;
    newLocale->Country = sCountry;
    newLocale->Variant = sVariant;
    newLocale->HashCode = hashCode;

    rtl_hashtable_add(&g_pLocaleTable, newLocale);

    return newLocale;
}

/*************************************************************************
 *  rtl_locale_getDefault
 */
rtl_Locale * SAL_CALL rtl_locale_getDefault()
{
    return g_pDefaultLocale;
}

/*************************************************************************
 *  rtl_locale_setDefault
 */
void SAL_CALL rtl_locale_setDefault( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant )
{
    g_pDefaultLocale = rtl_locale_register(language, country, variant);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
