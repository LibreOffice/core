/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regcomplazy.cxx,v $
 * $Revision: 1.7 $
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

#include <stdio.h>

#include "sal/main.h"
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>

#include <registry/registry.hxx>


#define OUSTR(x) ::rtl::OUString::createFromAscii( x )
#define OSToOUS(x) ::rtl::OStringToOUString(x, osl_getThreadTextEncoding())
#define OUSToOS(x) ::rtl::OUStringToOString(x, osl_getThreadTextEncoding())
using namespace ::rtl;

typedef ::std::vector< ::rtl::OString > OSVector;

typedef ::std::pair< ::rtl::OString, OSVector > DataPair;

typedef ::std::vector< DataPair > DataVector;

struct CompDescriptor {
    OString sImplementationName;
    OString sComponentName;
    OString sLoaderName;
    OSVector vSupportedServices;
    DataVector vData;
};

typedef ::std::vector< CompDescriptor > CDescrVector;

static void print_options() SAL_THROW( () )
{
    printf(
        "\nusage: regcomplazy [-v]registry_file cmp_descr_file ...\n\n"
        "Register a cmponent using a comp description file.\n"
        "Option -v prints verbose output on stdout.\n" );
}

static bool checkImplValue(RegistryValueList<sal_Char*>* pValueList, OString sImplName) {
    for (sal_uInt32 i=0; i < pValueList->getLength(); i++) {
        if (sImplName.equals(pValueList->getElement(i)))
            return true;
    }

    return false;
}

//==================================================================================================
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if (argc < 3)
    {
        print_options();
        return 1;
    }

    bool bVerbose = false;
    int nPos = 1;
    if ('-' == argv[ nPos ][ 0 ] && 'v' == argv[ nPos ][ 1 ])
    {
        if ('\0' != argv[ nPos ][ 2 ])
        {
            print_options();
            return 1;
        }
        bVerbose = true;
        ++nPos;
    }

    OUString sys_path( OUSTR( argv[ nPos ] ) );
    OUString reg_url;
    oslFileError rc = osl_getFileURLFromSystemPath( sys_path.pData, &reg_url.pData );
    if (osl_File_E_None != rc)
    {
        if (bVerbose)
            fprintf( stderr, "\nERROR: cannot make file url out of %s\n", argv[nPos]);
        return 1;
    }

    FILE* fDescr = fopen(argv[ ++nPos ], "r");
    OStringBuffer sBuffer(512);

    if ( fDescr) {
        size_t totalSize = 0;
        size_t readSize  = 0;
        char   pBuffer[513];

        while ( !feof(fDescr) )
        {
            if ( (readSize = fread(pBuffer, 1, 512, fDescr)) > 0
                 && !ferror(fDescr) ) {
                totalSize += readSize;
                if (totalSize >= 512)
                    sBuffer.ensureCapacity(totalSize * 2);

                sBuffer.append(pBuffer, readSize);
            }
        }
    fclose(fDescr);
    fDescr = 0; // just to be sure noone tries to use the file ever after
    }

    OString sDescr = sBuffer.makeStringAndClear();
    sal_Int32 nTokenIndex = 0;

    CDescrVector vDescr;
    CompDescriptor descr;
    bool bFirst = true;

    do {
        OString sTmp = sDescr.getToken(0, '\x0A', nTokenIndex);
        OString sToken(sTmp);
        if (sTmp.pData->buffer[sTmp.getLength()-1] == '\x0D')
            sToken = sTmp.copy(0, sTmp.getLength()-1);

        if (sToken.indexOf("[Data]") >= 0) {
            do {
                OString sTmp2 = sDescr.getToken(0, '\x0A', nTokenIndex);
                OString sToken2(sTmp2);
                if (sTmp2.pData->buffer[sTmp2.getLength()-1] == '\x0D')
                    sToken2 = sTmp2.copy(0, sTmp2.getLength()-1);

                if ((sToken2.getLength() > 0) && (sToken2.pData->buffer[0] != '[')) {
                    OString dataKey(sToken2.copy(0, sToken2.indexOf('=')));
                    OString sValues(sToken2.copy(sToken2.indexOf('=')+1));
                    sal_Int32 nVIndex = 0;
                    OSVector vValues;
                    do {
                        OString sValue = sValues.getToken(0, ';', nVIndex);
                        vValues.push_back(sValue);
                    } while (nVIndex >= 0 );
                    descr.vData.push_back(DataPair(dataKey, vValues));
                } else
                    break;
            } while (nTokenIndex >= 0 );
        }
        if ( sToken.indexOf("[ComponentDescriptor]") >= 0) {
            if (bFirst)
                bFirst = false;
            else
                vDescr.push_back(descr);

            descr = CompDescriptor();
        }
        else if ( sToken.indexOf("ImplementationName=") >= 0) {
            descr.sImplementationName = sToken.copy(19);
        }
        else if ( sToken.indexOf("ComponentName=") >= 0) {
            descr.sComponentName = sToken.copy(14);
        }
        else if ( sToken.indexOf("LoaderName=") >= 0) {
            descr.sLoaderName = sToken.copy(11);
        }
        else if ( (sToken.indexOf("[SupportedServices]") < 0) &&
                  (sToken.getLength() > 0) &&
                  (sToken.pData->buffer[0] != '[') ) {
            descr.vSupportedServices.push_back(sToken);
        }
    } while (nTokenIndex >= 0 );
    // insert the last descriptor
    vDescr.push_back(descr);

    Registry *pReg = new Registry;

    RegistryKey rootKey, key, subKey, serviceKey;

    if (pReg->open(reg_url, REG_READWRITE))
    {
        if (pReg->create(reg_url))
        {
            if (bVerbose)
                fprintf(stderr, "ERROR: open registry \"%s\" failed\n", argv[1]);
            return 1;
        }
    }
    if (pReg->openRootKey(rootKey)) {
        if (bVerbose)
            fprintf(stderr, "ERROR: open root key failed\n");
        return 1;
    }

    CDescrVector::const_iterator comp_iter = vDescr.begin();
    do {
        OString sImplName = (*comp_iter).sImplementationName;
        OUStringBuffer sbImpl;
        sbImpl.appendAscii("/IMPLEMENTATIONS/");
        sbImpl.append(OSToOUS(sImplName));
        OUString sImplKeyName = sbImpl.makeStringAndClear();

        if (rootKey.openKey(sImplKeyName, key) == REG_NO_ERROR) {
            if (bVerbose) {
               fprintf(stderr, "WARNING: implementation entry for \"%s\" already exists, existing entries are overwritten\n", sImplName.getStr());
            }
        } else {
            if (rootKey.createKey(sImplKeyName, key)) {
                if (bVerbose) {
                    fprintf(stderr, "ERROR: can't create new implementation entry \"%s\".\n", sImplName.getStr());
                }
                return 1;
            }
        }

        OString sLoaderName = (*comp_iter).sLoaderName;
        OUString usKeyName(OUSTR("UNO/ACTIVATOR"));
        key.createKey(usKeyName, subKey);
        subKey.setValue(OUString(), RG_VALUETYPE_STRING,
                        (sal_Char*)sLoaderName.getStr(), sLoaderName.getLength()+1);

        OString sCompName = (*comp_iter).sComponentName;
        usKeyName = OUSTR("UNO/LOCATION");
        key.createKey(usKeyName, subKey);
        subKey.setValue(OUString(), RG_VALUETYPE_STRING,
                        (sal_Char*)sCompName.getStr(), sCompName.getLength()+1);

        if ((*comp_iter).vData.size() > 0) {
            usKeyName = OUSTR("DATA");
            RegistryKey dataKey, valueKey;
            key.createKey(usKeyName, dataKey);

            DataVector::const_iterator data_iter = ((*comp_iter).vData).begin();
            do {
                OUString sDataKey(OSToOUS((*data_iter).first));
                dataKey.createKey(sDataKey, valueKey);

                OSVector::const_iterator value_iter = ((*data_iter).second).begin();
                int vlen = (*data_iter).second.size();
                sal_Char** pValueList = (sal_Char**)rtl_allocateZeroMemory(
                    vlen * sizeof(sal_Char*));
                int i = 0;
                do {
                    pValueList[i] = (sal_Char*)rtl_allocateZeroMemory(
                        (*value_iter).getLength()+1 * sizeof(sal_Char));
                    rtl_copyMemory(pValueList[i], (sal_Char*)(*value_iter).getStr(),
                               (*value_iter).getLength()+1);
                    i++;
                    value_iter++;
                } while (value_iter != (*data_iter).second.end());

                valueKey.setStringListValue(OUString(), pValueList, vlen);

                // free memory
                for (i=0; i<vlen; i++)
                    rtl_freeMemory(pValueList[i]);
                rtl_freeMemory(pValueList);

                data_iter++;
            } while (data_iter != (*comp_iter).vData.end());
        }

        usKeyName = OUSTR("UNO/SERVICES");
        key.createKey(usKeyName, subKey);

        rootKey.createKey(OUSTR("/SERVICES"), serviceKey);

        OSVector::const_iterator serv_iter = ((*comp_iter).vSupportedServices).begin();
        OUString usServiceKeyName;
        do {
            usServiceKeyName = OSToOUS(*serv_iter);
            // write service key in impl section
            subKey.createKey(usServiceKeyName, key);

            if (serviceKey.openKey(usServiceKeyName, key) == REG_NO_ERROR) {
                RegistryValueList<sal_Char*> valueList;
                serviceKey.getStringListValue(usServiceKeyName, valueList);
                if ( checkImplValue(&valueList, sImplName) ) {
                    serv_iter++;
                    continue;
                }

                sal_uInt32 nServices = valueList.getLength()+1;
                sal_Char** pImplList = (sal_Char**)rtl_allocateZeroMemory(
                    nServices * sizeof(sal_Char*));
                pImplList[0] = (sal_Char*)rtl_allocateZeroMemory(
                    sImplName.getLength()+1 * sizeof(sal_Char));
                rtl_copyMemory(pImplList[0], (sal_Char*)sImplName.getStr(),
                               sImplName.getLength()+1);
                for (sal_uInt32 i=0; i < valueList.getLength(); i++) {
                    pImplList[i+1]=valueList.getElement(i);
                }
                key.setStringListValue(OUString(), pImplList, nServices);

                // free memory
                rtl_freeMemory(pImplList[0]);
                rtl_freeMemory(pImplList);

            } else {
                serviceKey.createKey(usServiceKeyName, key);

                sal_Char* pImplList[1];
                pImplList[0] = (sal_Char*)rtl_allocateZeroMemory(
                    sImplName.getLength()+1 * sizeof(sal_Char));
                rtl_copyMemory(pImplList[0], (sal_Char*)sImplName.getStr(),
                               sImplName.getLength()+1);
                key.setStringListValue(OUString(), pImplList, 1);

                // free memory
                rtl_freeMemory(pImplList[0]);
            }
            serv_iter++;
        } while (serv_iter != (*comp_iter).vSupportedServices.end());

        comp_iter++;
    } while (comp_iter != vDescr.end());

    key.closeKey();
    subKey.closeKey();
    serviceKey.closeKey();
    rootKey.closeKey();
    pReg->close();

    return 0;
}
