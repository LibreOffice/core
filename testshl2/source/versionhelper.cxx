/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: versionhelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:28:57 $
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

#include <stdlib.h>
#include "versionhelper.hxx"

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

