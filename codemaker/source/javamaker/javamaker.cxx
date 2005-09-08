/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: javamaker.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:17:32 $
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

#include <stdio.h>

#include "sal/main.h"

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include <codemaker/typemanager.hxx>
#endif
#include "codemaker/generatedtypeset.hxx"

#include "javaoptions.hxx"
#include "javatype.hxx"

using namespace rtl;

sal_Bool produceAllTypes(RegistryKey& rTypeKey, sal_Bool bIsExtraType,
                         TypeManager const & typeMgr,
                         codemaker::GeneratedTypeSet & generated,
                         JavaOptions* pOptions,
                         sal_Bool bFullScope)
    throw( CannotDumpException )
{
    OString typeName = typeMgr.getTypeName(rTypeKey);

    if (!produceType(rTypeKey, bIsExtraType, typeMgr, generated, pOptions))
    {
        fprintf(stderr, "%s ERROR: %s\n",
                pOptions->getProgramName().getStr(),
                OString("cannot dump Type '" + typeName + "'").getStr());
        exit(99);
    }

    RegistryKeyList typeKeys = typeMgr.getTypeKeys(typeName);
    RegistryKeyList::const_iterator iter = typeKeys.begin();
    RegistryKey key, subKey;
    RegistryKeyArray subKeys;

    while (iter != typeKeys.end())
    {
        key = (*iter).first;

        if (!(*iter).second  && !key.openSubKeys(OUString(), subKeys))
        {
            for (sal_uInt32 i = 0; i < subKeys.getLength(); i++)
            {
                subKey = subKeys.getElement(i);
                if (bFullScope)
                {
                    if (!produceAllTypes(
                            subKey, (*iter).second,
                            typeMgr, generated, pOptions, sal_True))
                        return sal_False;
                } else
                {
                    if (!produceType(subKey, (*iter).second,
                                     typeMgr, generated, pOptions))
                        return sal_False;
                }
            }
        }

        ++iter;
    }

    return sal_True;
}

sal_Bool produceAllTypes(const OString& typeName,
                         TypeManager const & typeMgr,
                         codemaker::GeneratedTypeSet & generated,
                         JavaOptions* pOptions,
                         sal_Bool bFullScope)
    throw( CannotDumpException )
{
    if (!produceType(typeName, typeMgr, generated, pOptions))
    {
        fprintf(stderr, "%s ERROR: %s\n",
                pOptions->getProgramName().getStr(),
                OString("cannot dump Type '" + typeName + "'").getStr());
        exit(99);
    }

    RegistryKeyList typeKeys = typeMgr.getTypeKeys(typeName);
    RegistryKeyList::const_iterator iter = typeKeys.begin();
    RegistryKey key, subKey;
    RegistryKeyArray subKeys;

    while (iter != typeKeys.end())
    {
        key = (*iter).first;
        if (!(*iter).second  && !key.openSubKeys(OUString(), subKeys))
        {
            for (sal_uInt32 i = 0; i < subKeys.getLength(); i++)
            {
                subKey = subKeys.getElement(i);
                if (bFullScope)
                {
                    if (!produceAllTypes(
                            subKey, (*iter).second,
                            typeMgr, generated, pOptions, sal_True))
                        return sal_False;
                } else
                {
                    if (!produceType(subKey, (*iter).second,
                                     typeMgr, generated, pOptions))
                        return sal_False;
                }
            }
        }

        ++iter;
    }

    return sal_True;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    JavaOptions options;

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

    try
    {
        if (options.isValid("-T"))
        {
            OString tOption(options.getOption("-T"));
            sal_Int32 nIndex = 0;

            codemaker::GeneratedTypeSet generated;
            OString typeName, tmpName;
            sal_Bool ret = sal_False;
            do
            {
                typeName = tOption.getToken(0, ';', nIndex);

                sal_Int32 nPos = typeName.lastIndexOf( '.' );
                tmpName = typeName.copy( nPos != -1 ? nPos+1 : 0 );
                if (tmpName == "*")
                {
                    // produce this type and his scope.
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
                    // related to task #116780# the scope is recursively
                    // generated.  bFullScope = true
                    ret = produceAllTypes(
                        tmpName, typeMgr, generated, &options, sal_True);
                } else
                {
                    // produce only this type
                    ret = produceType(
                        typeName.replace('.', '/'), typeMgr, generated,
                        &options);
                }

                if (!ret)
                {
                    fprintf(stderr, "%s ERROR: %s\n",
                            options.getProgramName().getStr(),
                            OString("cannot dump Type '" + typeName + "'").getStr());
                    exit(99);
                }
            } while( nIndex != -1 );
        } else
        {
            // produce all types
            codemaker::GeneratedTypeSet generated;
            if (!produceAllTypes("/", typeMgr, generated, &options, sal_True))
            {
                fprintf(stderr, "%s ERROR: %s\n",
                        options.getProgramName().getStr(),
                        "an error occurs while dumping all types.");
                exit(99);
            }
        }
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


