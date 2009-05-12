/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: versionhelper.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"

#include <stdlib.h>
#include "testshl/versionhelper.hxx"

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

// -----------------------------------------------------------------------------
VersionHelper::VersionHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions)
        :DynamicLibraryHelper(_sDLLName, _aOptions),
         m_pInfo(NULL)
{
    // try to get the entry pointer
    FktGetVersionInfoPtr pFunc = (FktGetVersionInfoPtr)
        m_pModule->getFunctionSymbol(
            rtl::OUString::createFromAscii( "GetVersionInfo" ) );

    if (pFunc)
    {
        const VersionInfo *pVersion = (pFunc)();
        m_pInfo = pVersion;
    }
}

//# void VersionHelper::print(std::ostream &stream)
//# {
//#     stream << "  Time:" << getTime()   << std::endl;
//#     stream << "  Date:" << getDate()   << std::endl;
//#     stream << "   Upd:" << getUpd()    << std::endl;
//#     stream << " Minor:" << getMinor()  << std::endl;
//#     stream << " Build:" << getBuild()  << std::endl;
//#     stream << "Inpath:" << getInpath() << std::endl;
//# }
//#
//# std::ostream & operator <<( std::ostream &stream, VersionHelper &_aVersion )
//# {
//#     _aVersion.print (stream);
//#     return stream;
//# }
//#
// -----------------------------------------------------------------------------

bool VersionHelper::isOk() const
{
    if (m_pInfo != NULL) return true;
    return false;
}

rtl::OString VersionHelper::getTime() const
{
    return m_pInfo->pTime;
}
rtl::OString VersionHelper::getDate() const
{
    return m_pInfo->pDate;
}
rtl::OString VersionHelper::getUpd() const
{
    return m_pInfo->pUpd;
}
rtl::OString VersionHelper::getMinor() const
{
    return m_pInfo->pMinor;
}
rtl::OString VersionHelper::getBuild() const
{
    return m_pInfo->pBuild;
}
rtl::OString VersionHelper::getInpath() const
{
    return m_pInfo->pInpath;
}



void VersionHelper::printall(FILE * out)
{
    if (isOk())
    {
        rtl::OString aStr = getTime();
        fprintf(out, "  Time:%s\n", aStr.getStr()  );
        fprintf(out, "  Date:%s\n", getDate().getStr()  );
        fprintf(out, "   Upd:%s\n", getUpd().getStr()   );
        fprintf(out, " Minor:%s\n", getMinor().getStr() );
        fprintf(out, " Build:%s\n", getBuild().getStr() );
        fprintf(out, "Inpath:%s\n", getInpath().getStr());

        fflush(out);
    }
    else
    {
        fprintf(stderr, "error: No version info found.\n");
    }
}

