/*************************************************************************
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2002-06-24 15:17:55 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#include <comphelper/regpathhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif


using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


int main(int argc,char* argv[])
{
    Reference< XMultiServiceFactory > xFac;
    Reference< XComponent > xComponent;

    try
    {
        OUString workDir;
        osl_getProcessWorkingDir(&workDir.pData);
        OUString dir;
        osl::FileBase::getSystemPathFromFileURL(workDir,
                                                dir);
        OString oDir(dir.getStr(),
                     dir.getLength(),
                     RTL_TEXTENCODING_UTF8);
        fprintf(stdout,"\nworking directory: %s",oDir.getStr());

        OUString systemRegistryPath;
        Bootstrap::get(OUString::createFromAscii("SystemRegistryPath"),
                       systemRegistryPath,
                       comphelper::getPathToSystemRegistry());
        OString path(systemRegistryPath.getStr(),
                     systemRegistryPath.getLength(),
                     RTL_TEXTENCODING_UTF8);
        fprintf(stdout,"\nsystem registry path: %s\n",path.getStr());

        xFac = cppu::createRegistryServiceFactory( systemRegistryPath,
                                                   OUString(),
                                                   true);
        if(!xFac.is())
        {
            fprintf(stderr,"\ncould not create ServiceFactory");
            return 1;
        }

        comphelper::setProcessServiceFactory(xFac);
        xComponent = Reference< XComponent >(xFac,UNO_QUERY);
        if(!xComponent.is())
        {
            fprintf(stderr,"\nERROR: could not set processServiceFactory");
            fprintf(stderr,"\nERROR: P0-bug to ABI\n");
            return 1;
        }

        int anyerr = 0;

        xComponent->dispose();
        return anyerr;
    }
    catch ( const Exception& e)
    {
        fprintf(stderr,"\nERROR: any other error");
        fprintf(stderr,"\nERROR: P0-bug to ABI\n");
        return 1;
    }
    return 0;
}
