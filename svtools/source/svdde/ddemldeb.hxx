/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddemldeb.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:45:01 $
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

