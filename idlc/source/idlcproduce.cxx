/*************************************************************************
 *
 *  $RCSfile: idlcproduce.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
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
#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _IDLC_ASTMODULE_HXX_
#include <idlc/astmodule.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#if defined(SAL_W32) || defined(SAL_OS2)
#include <io.h>
#include <direct.h>
#include <errno.h>
#endif

#ifdef SAL_UNX
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif

#ifdef SAL_UNX
extern sal_Char SEPERATOR;
#else
extern sal_Char SEPERATOR;
#endif

using namespace ::rtl;

StringList* pCreatedDirectories = NULL;

static sal_Bool checkOutputPath(const OString& completeName)
{
    OStringBuffer buffer(completeName.getLength());
    sal_Int32 count = completeName.getTokenCount(SEPERATOR) - 1;
    sal_Int32 offset=0;

    if ( !count )
        return sal_True;

    OString token(completeName.getToken(0, SEPERATOR));
    const sal_Char* p = token.getStr();
    if (strcmp(p, "..") == 0
        || *(p+1) == ':'
        || strcmp(p, ".") == 0)
    {
        offset++;
        buffer.append(token);
        buffer.append(SEPERATOR);
    }

    for (sal_Int32 i=offset; i < count; i++)
    {
        buffer.append(completeName.getToken(i, SEPERATOR));

        if ( buffer.getLength() > 0 )
        {
#ifdef SAL_UNX
            if (mkdir((char*)buffer.getStr(), 0777) == -1)
#else
            if (mkdir((char*)buffer.getStr()) == -1)
#endif
            {
                if (errno == ENOENT)
                {
                    fprintf(stderr, "%s: cannot create directory '%s'\n",
                            idlc()->getOptions()->getProgramName().getStr(), buffer.getStr());
                    return sal_False;
                }
            } else
            {
                if ( !pCreatedDirectories )
                    pCreatedDirectories = new StringList();
                pCreatedDirectories->push_front(buffer.getStr());
            }
        }
        if ( i+1 < count )
            buffer.append(SEPERATOR);
    }
    return sal_True;
}

static sal_Bool cleanPath()
{
    if ( pCreatedDirectories )
    {
        StringList::iterator iter = pCreatedDirectories->begin();
        StringList::iterator end = pCreatedDirectories->end();
        while ( iter != end )
        {
//#ifdef SAL_UNX
//          if (rmdir((char*)(*iter).getStr(), 0777) == -1)
//#else
            if (rmdir((char*)(*iter).getStr()) == -1)
//#endif
            {
                fprintf(stderr, "%s: cannot remove directory '%s'\n",
                        idlc()->getOptions()->getProgramName().getStr(), (*iter).getStr());
                return sal_False;
            }
            iter++;
        }
        delete pCreatedDirectories;
    }
    return sal_True;
}

void SAL_CALL removeIfExists(const OString& fileName)
{
    FILE* pDest = fopen(fileName.getStr(), "r");
    if ( pDest )
    {
        fclose(pDest);
        unlink(fileName.getStr());
    }
}

sal_Int32 SAL_CALL produceFile(const OString& fileName)
{
    Options* pOptions = idlc()->getOptions();

    OString regFileName;
    if ( pOptions->isValid("-O") )
    {
        regFileName = pOptions->getOption("-O");
        sal_Char c = regFileName.getStr()[regFileName.getLength()-1];

        if ( c != SEPERATOR )
            regFileName += OString::valueOf(SEPERATOR);
    }

    OString regTmpName(regFileName);
    OString strippedFileName(fileName.copy(fileName.lastIndexOf(SEPERATOR) + 1));
    regFileName += strippedFileName.replaceAt(strippedFileName.getLength() -3 , 3, "urd");
    regTmpName += strippedFileName.replaceAt(strippedFileName.getLength() -3 , 3, "_idlc_");

    if ( !checkOutputPath(regTmpName) )
    {
        fprintf(stderr, "%s: could not create path of registry file '%s'.\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regFileName);
        return 1;
    }

    RegistryLoader              regLoader;
    RegistryTypeWriterLoader    writerLoader;

    if ( !regLoader.isLoaded() || !writerLoader.isLoaded() )
    {
        fprintf(stderr, "%s: could not load registry dll.\n",
                pOptions->getProgramName().getStr());
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    Registry regFile(regLoader);

    removeIfExists(regTmpName);
    if ( regFile.create(OStringToOUString(regTmpName, RTL_TEXTENCODING_UTF8)) )
    {
        fprintf(stderr, "%s: could not create registry file '%s'\n",
                pOptions->getProgramName().getStr(), regTmpName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    RegistryKey rootKey;
    if ( regFile.openRootKey(rootKey) )
    {
        fprintf(stderr, "%s: could not open root of registry file '%s'\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    // produce registry file
    if ( !idlc()->getRoot()->dump(rootKey, &writerLoader) )
    {
        rootKey.closeKey();
        regFile.close();
        regFile.destroy(OStringToOUString(regFileName, RTL_TEXTENCODING_UTF8));
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    if ( rootKey.closeKey() )
    {
        fprintf(stderr, "%s: could not close root of registry file '%s'\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }
    if ( regFile.close() )
    {
        fprintf(stderr, "%s: could not close registry file '%s'\n",
                pOptions->getProgramName().getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        removeIfExists(regFileName);
        cleanPath();
        return 1;
    }

    removeIfExists(regFileName);
    if ( rename(regTmpName.getStr(), regFileName.getStr()) != 0 )
    {
        fprintf(stderr, "%s: cannot rename temporary registry '%s' to '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                regTmpName.getStr(), regFileName.getStr());
        removeIfExists(regTmpName);
        cleanPath();
        return 1;
    }

    return 0;
}
