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

namespace binfilter
{

#if defined(OV_DEBUG)

void ImpWriteLogFile(char*,char*);
void ImpAddHSZ( HSZ, String& );
void ImpWriteDdeStatus(char*, char* );
void ImpWriteDdeData(char*, DDESTRUCT*);

#ifdef LOGFILE
#define WRITELOG(aString) ImpWriteLogFile("\\ddeml.log",aString);
#else
#define WRITELOG(bla)
#endif
#ifdef STATUSFILE
#define WRITESTATUS(aContext) ImpWriteDdeStatus("\\ddeml.sts",aContext);
#else
#define WRITESTATUS(bla)
#endif
#ifdef DDEDATAFILE
#define WRITEDATA(data) ImpWriteDdeData("\\ddeml.dat",data);
#else
#define WRITEDATA(bla)
#endif

#else

#define WRITELOG(bla)
#define WRITESTATUS(bla)
#define WRITEDATA(bla)

#endif

APIRET MyDosAllocSharedMem(void** ppBaseAddress, char* pszName, unsigned long ulObjectSize,
    unsigned long ulFlags, char* pContextStr );

APIRET MyDosAllocMem(void** ppBaseAddress, unsigned long ulObjectSize,
    unsigned long ulFlags, char* pContextStr );

APIRET MyDosFreeMem( void* pBaseAddress, char* pContextStr );

}
