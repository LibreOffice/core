/*************************************************************************
 *
 *  $RCSfile: versionhelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:36:43 $
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
    FktGetVersionInfoPtr pFunc = (FktGetVersionInfoPtr) m_pModule->getSymbol( rtl::OUString::createFromAscii( "GetVersionInfo" ) );

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

