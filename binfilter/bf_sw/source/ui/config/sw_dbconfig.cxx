/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <dbconfig.hxx>

#include <tools/debug.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <swdbdata.hxx>
namespace binfilter {
using namespace utl;
using namespace rtl;
using namespace ::com::sun::star::uno;

/*N*/ #define C2U(cChar) OUString::createFromAscii(cChar)
/*--------------------------------------------------------------------
     Beschreibung: Ctor
 --------------------------------------------------------------------*/

/*N*/ const Sequence<OUString>& SwDBConfig::GetPropertyNames()
/*N*/ {
/*N*/ 	static Sequence<OUString> aNames;
/*N*/ 	if(!aNames.getLength())
/*N*/ 	{
/*N*/ 		static const char* aPropNames[] =
/*N*/ 		{
/*N*/             "AddressBook/DataSourceName",        //  0
/*N*/             "AddressBook/Command",              //  1
/*N*/             "AddressBook/CommandType",          //  2
/*N*/             "Bibliography/CurrentDataSource/DataSourceName",        //  4
/*N*/             "Bibliography/CurrentDataSource/Command",              //  5
/*N*/             "Bibliography/CurrentDataSource/CommandType"          //  6
/*N*/         };
/*N*/         const int nCount = sizeof(aPropNames)/sizeof(const char*);
/*N*/ 		aNames.realloc(nCount);
/*N*/ 		OUString* pNames = aNames.getArray();
/*N*/ 		for(int i = 0; i < nCount; i++)
/*N*/ 			pNames[i] = OUString::createFromAscii(aPropNames[i]);
/*N*/ 	}
/*N*/ 	return aNames;
/*N*/ }
/* -----------------------------06.09.00 16:44--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDBConfig::SwDBConfig() :
/*N*/     ConfigItem(C2U("Office.DataAccess"),
/*N*/         CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
/*N*/     pAdrImpl(0),
/*N*/     pBibImpl(0)
/*N*/ {
/*N*/ };
/* -----------------------------06.09.00 16:50--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwDBConfig::~SwDBConfig()
/*N*/ {
/*N*/     delete pAdrImpl;
/*N*/     delete pBibImpl;
/*N*/ }
/* -----------------------------20.02.01 12:32--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwDBConfig::Load()
/*N*/ {
/*N*/ 	const Sequence<OUString>& rNames = GetPropertyNames();
/*N*/     if(!pAdrImpl)
/*N*/ 	{
/*N*/ 
/*N*/         pAdrImpl = new SwDBData;
/*N*/         pAdrImpl->nCommandType = 0;
/*N*/         pBibImpl = new SwDBData;
/*N*/         pBibImpl->nCommandType = 0;
/*N*/     }
/*N*/ 	Sequence<Any> aValues = GetProperties(rNames);
/*N*/ 	const Any* pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
/*N*/ 	if(aValues.getLength() == rNames.getLength())
/*N*/ 	{
/*N*/ 		for(int nProp = 0; nProp < rNames.getLength(); nProp++)
/*N*/ 		{
/*N*/ 			switch(nProp)
/*N*/ 			{
/*N*/                 case  0: pValues[nProp] >>= pAdrImpl->sDataSource;  break;
/*N*/                 case  1: pValues[nProp] >>= pAdrImpl->sCommand;     break;
/*N*/                 case  2: pValues[nProp] >>= pAdrImpl->nCommandType; break;
/*N*/                 case  3: pValues[nProp] >>= pBibImpl->sDataSource;  break;
/*N*/                 case  4: pValues[nProp] >>= pBibImpl->sCommand;     break;
/*N*/                 case  5: pValues[nProp] >>= pBibImpl->nCommandType; break;
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/* -----------------------------20.02.01 12:36--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const SwDBData& SwDBConfig::GetAddressSource()
/*N*/ {
/*N*/     if(!pAdrImpl)
/*N*/ 		Load();
/*N*/     return *pAdrImpl;
/*N*/ }

    void SwDBConfig::Commit() {}
    void SwDBConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
