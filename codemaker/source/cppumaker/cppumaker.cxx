/*************************************************************************
 *
 *  $RCSfile: cppumaker.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:45:39 $
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

#include <stdio.h>

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include <codemaker/typemanager.hxx>
#endif
#include "codemaker/generatedtypeset.hxx"

#include "cppuoptions.hxx"
#include "cpputype.hxx"

using namespace rtl;

namespace {

void failed(rtl::OString const & typeName, CppuOptions * options) {
    fprintf(stderr, "%s ERROR: %s\n", options->getProgramName().getStr(),
            rtl::OString("cannot dump Type '" + typeName + "'").getStr());
    exit(99);
}

void produce(
    rtl::OString const & typeName, TypeManager const & typeMgr,
    codemaker::GeneratedTypeSet & generated, CppuOptions * options)
{
    if (!produceType(typeName, typeMgr, generated, options)) {
        failed(typeName, options);
    }
}

void produceAllTypes(
    rtl::OString const & typeName, TypeManager const & typeMgr,
    codemaker::GeneratedTypeSet & generated, CppuOptions * pOptions,
    bool fullScope)
{
    produce(typeName, typeMgr, generated, pOptions);

    RegistryKey typeKey = typeMgr.getTypeKey(typeName);
    RegistryKeyNames subKeys;

    if (typeKey.getKeyNames(OUString(), subKeys)) {
        failed(typeName, pOptions);
    }

    OString tmpName;
    for (sal_uInt32 i=0; i < subKeys.getLength(); i++)
    {
        tmpName = OUStringToOString(subKeys.getElement(i), RTL_TEXTENCODING_UTF8);

        if (pOptions->isValid("-B"))
            tmpName = tmpName.copy(tmpName.indexOf('/', 1) + 1);
        else
            tmpName = tmpName.copy(1);

        if (fullScope) {
            produceAllTypes(tmpName, typeMgr, generated, pOptions, true);
        } else {
            produce(tmpName, typeMgr, generated, pOptions);
        }
    }
}

}

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    CppuOptions options;

    try
    {
        if (!options.initOptions(argc, argv))
        {
            exit(1);
        }
    }
    catch( IllegalArgument& e)
    {
        fprintf(stderr, "Illegal option: %s\n", e.m_message.getStr());
        exit(99);
    }

    RegistryTypeManager typeMgr;

    if (!typeMgr.init(options.getInputFiles(), options.getExtraInputFiles()))
    {
        fprintf(stderr, "%s : init registries failed, check your registry files.\n", options.getProgramName().getStr());
        exit(99);
    }

    if (options.isValid("-B"))
    {
        typeMgr.setBase(options.getOption("-B"));
    }

    codemaker::GeneratedTypeSet generated;
    try
    {
        if (options.isValid("-T"))
        {
            OString tOption(options.getOption("-T"));

            OString typeName, tmpName;
            sal_Int32 nIndex = 0;
            do
            {
                typeName = tOption.getToken(0, ';', nIndex);

                sal_Int32 nPos = typeName.lastIndexOf( '.' );
                tmpName = typeName.copy( nPos != -1 ? nPos+1 : 0 );
                if (tmpName == "*")
                {
                    // produce this type and his scope, but the scope is not recursively  generated.
                    if (typeName.equals("*"))
                    {
                        tmpName = "/";
                    } else
                    {
                        tmpName = typeName.copy(0, typeName.lastIndexOf('.')).replace('.', '/');
                        if (tmpName.getLength() == 0)
                            tmpName = "/";
                        else
                            tmpName.replace('.', '/');
                    }
                    produceAllTypes(
                        tmpName, typeMgr, generated, &options, false);
                } else
                {
                    // produce only this type
                    produce(
                        typeName.replace('.', '/'), typeMgr, generated,
                        &options);
                }
            } while( nIndex != -1 );
        } else
        {
            // produce all types
            produceAllTypes("/", typeMgr, generated, &options, true);
        }
        // C++ header files generated for the following UNO types are included
        // in header files in cppu/inc/com/sun/star/uno (Any.hxx, Reference.hxx,
        // Type.h), so it seems best to always generate those C++ header files:
        produce(
            "com/sun/star/uno/RuntimeException", typeMgr, generated, &options);
        produce("com/sun/star/uno/TypeClass", typeMgr, generated, &options);
        produce("com/sun/star/uno/XInterface", typeMgr, generated, &options);
    }
    catch( CannotDumpException& e)
    {
        fprintf(stderr, "%s ERROR: %s\n",
                options.getProgramName().getStr(),
                e.m_message.getStr());
        exit(99);
    }

    return 0;
}


