/*************************************************************************
 *
 *  $RCSfile: idlcmain.cxx,v $
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

#ifdef SAL_UNX
sal_Char SEPERATOR = '/';
#else
sal_Char SEPERATOR = '\\';
#endif

using namespace ::rtl;

void SAL_CALL main( int argc, char** argv )
{
    Options options;

    try
    {
        if (!options.initOptions(argc, argv))
            exit(1);
    }
    catch( IllegalArgument& e)
    {
        fprintf(stderr, "Illegal argument: %s\n%s",
                e.m_message.getStr(),
                options.prepareVersion().getStr());
        exit(99);
    }

    setIdlc(&options);

    const StringVector& files = options.getInputFiles();
    sal_Int32   nFiles = files.size();
    sal_Int32   nErrors = 0;
    for ( sal_Int32 i=0; i < nFiles; i++ )
    {
        fprintf(stdout, "%s: compile '%s' ... \n",
                options.getProgramName().getStr(), files[i].getStr());
        nErrors = compileFile(files[i]);
        if ( nErrors )
        {
            OString outputName;
            if ( options.isValid("-O") )
            {
                outputName = options.getOption("-O");
                sal_Char c = outputName.getStr()[outputName.getLength()-1];

                if ( c != SEPERATOR )
                    outputName += OString::valueOf(SEPERATOR);
            }

            OString strippedFileName(files[i].copy(files[i].lastIndexOf(SEPERATOR) + 1));
            outputName += strippedFileName.replaceAt(strippedFileName.getLength() -3 , 3, "urd");
            removeIfExists(outputName);
        } else
            nErrors = produceFile(files[i]);

        idlc()->reset();
        if ( nErrors > 0 )
            break;
    }

    if ( nErrors > 0 )
    {
        fprintf(stdout, "%s: detected %d errors in file '%s'%s",
                options.getProgramName().getStr(), nErrors,
                files[i].getStr(), options.prepareVersion().getStr());
    } else
    {
        fprintf(stdout, "%s: returned with %d errors%s",
                options.getProgramName().getStr(), nErrors,
                options.prepareVersion().getStr());
    }
    exit(nErrors);
}
