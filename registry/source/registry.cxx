/*************************************************************************
 *
 *  $RCSfile: registry.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:42 $
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

#include    <registry/registry.h>

#ifndef _REGISTRY_REGISTRY_HXX_
#include    <registry/registry.hxx>
#endif

#ifndef _REGIMPL_HXX_
#include    "regimpl.hxx"
#endif

#ifndef _REGKEY_HXX_
#include    "keyimpl.hxx"
#endif

#include <vos/process.hxx>

#if defined(WIN32) || defined(WNT) || defined(__OS2__)
#include <io.h>
#endif

#if defined(UNX)
#include <stdlib.h>
#include <unistd.h>
#endif

using namespace vos;

#if defined ( GCC ) && ( defined ( SCO ) || defined ( NETBSD ) )
ORealDynamicLoader* ODynamicLoader<Registry_Api>::m_pLoader = NULL;
#endif

OString getTempName()
{
    OUString    uTmpPattern;
    sal_Char    tmpPattern[512] = "";
    sal_Char    *pTmpName = NULL;

    OStartupInfo StartupInfo;

    if (StartupInfo.getEnvironment(OUString( RTL_CONSTASCII_USTRINGPARAM("TMP") ),
            uTmpPattern) != OStartupInfo::E_None)
    {
        if (StartupInfo.getEnvironment( OUString( RTL_CONSTASCII_USTRINGPARAM("TEMP") ),
                uTmpPattern) != NAMESPACE_VOS(OStartupInfo)::E_None)
        {
#if defined(WIN32) || defined(WNT) || defined(OS2)
            strcpy(tmpPattern, ".");
#else
            strcpy(tmpPattern, "/tmp");
#endif
        }
    }

    if ( uTmpPattern.getLength() )
    {
        strcpy(tmpPattern, OUStringToOString(uTmpPattern, RTL_TEXTENCODING_UTF8).getStr());
    }

#if defined(WIN32) || defined(WNT)
    strcat(tmpPattern, "\\reg_XXXXXX");
    pTmpName = mktemp(tmpPattern);
#endif

#ifdef __OS2__
    strcpy(tmpPattern, tempnam(NULL, "reg_"));
    pTmpName = tmpPattern;
#endif

#ifdef UNX
    strcat(tmpPattern, "/reg_XXXXXX");
    pTmpName = mktemp(tmpPattern);
#endif

    return OString(pTmpName);
}

//*********************************************************************
//  acquire
//
static void REGISTRY_CALLTYPE acquire(RegHandle hReg)
{
    ORegistry* pReg = (ORegistry*) hReg;

    if (pReg != NULL)
        pReg->acquire();
}


//*********************************************************************
//  release
//
static void REGISTRY_CALLTYPE release(RegHandle hReg)
{
    ORegistry* pReg = (ORegistry*) hReg;

    if (pReg)
    {
        if (pReg->release() == 0)
        {
            delete pReg;
            hReg = NULL;
        }
    }
}


//*********************************************************************
//  getName
//
static RegError REGISTRY_CALLTYPE getName(RegHandle hReg, rtl_uString** pName)
{
    ORegistry*  pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if ( pReg->isOpen() )
        {
            rtl_uString_assign(pName, pReg->getName().pData);
            return REG_NO_ERROR;
        } else
        {
            rtl_uString_new(pName);
            return REG_REGISTRY_NOT_OPEN;
        }
    }

    rtl_uString_new(pName);
    return REG_INVALID_REGISTRY;
}


//*********************************************************************
//  isReadOnly
//
static sal_Bool REGISTRY_CALLTYPE isReadOnly(RegHandle hReg)
{
    if (hReg)
        return ((ORegistry*)hReg)->isReadOnly();
    else
        return sal_False;
}


//*********************************************************************
//  createRegistry
//
static RegError REGISTRY_CALLTYPE createRegistry(rtl_uString* registryName,
                                                 RegHandle* phRegistry)
{
    RegError ret;

    ORegistry* pReg = new ORegistry();
    if (ret = pReg->initRegistry(registryName, REG_CREATE))
    {
        *phRegistry = NULL;
        return ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  openRootKey
//
static RegError REGISTRY_CALLTYPE openRootKey(RegHandle hReg,
                                              RegKeyHandle* phRootKey)
{
    ORegistry* pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        phRootKey = NULL;
        return REG_INVALID_REGISTRY;
    }

    *phRootKey = pReg->getRootKey();

    return REG_NO_ERROR;
}


//*********************************************************************
//  openRegistry
//
static RegError REGISTRY_CALLTYPE openRegistry(rtl_uString* registryName,
                                               RegHandle* phRegistry,
                                               RegAccessMode accessMode)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if (_ret = pReg->initRegistry(registryName, accessMode))
    {
        *phRegistry = NULL;
        delete pReg;
        return _ret;
    }


    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  closeRegistry
//
static RegError REGISTRY_CALLTYPE closeRegistry(RegHandle hReg)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;

        RegError ret = REG_NO_ERROR;
        if (pReg->release() == 0)
        {
            delete(pReg);
            hReg = NULL;
        }
        else
            ret = pReg->closeRegistry();

        return ret;
    } else
    {
        return REG_INVALID_REGISTRY;
    }
}


//*********************************************************************
//  destroyRegistry
//
static RegError REGISTRY_CALLTYPE destroyRegistry(RegHandle hReg,
                                                  rtl_uString* registryName)
{
    ORegistry   *pReg;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_INVALID_REGISTRY;

        RegError ret = REG_NO_ERROR;
        if (!(ret = pReg->destroyRegistry(registryName)))
        {
            if (!registryName->length)
            {
                delete(pReg);
                hReg = NULL;
            }
        }
        return ret;
    } else
    {
        return REG_INVALID_REGISTRY;
    }
}


//*********************************************************************
//  loadRegKey
//
static RegError REGISTRY_CALLTYPE loadKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    if (!pKey->openKey(keyName, (RegKeyHandle*)&pNewKey))
    {
        pKey->closeKey(pNewKey);
        pKey->deleteKey(keyName);
    }

    if (_ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey))
        return _ret;

    if (_ret = pReg->loadKey(pNewKey, regFileName))
    {
        pKey->closeKey(pNewKey);
        pKey->deleteKey(keyName);
        return _ret;
    }

    return pKey->closeKey(pNewKey);
}

//*********************************************************************
//  saveKey
//
static RegError REGISTRY_CALLTYPE saveKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if (_ret = pKey->openKey(keyName, (RegKeyHandle*)&pNewKey))
        return _ret;

    if (_ret = pReg->saveKey(pNewKey, regFileName))
    {
        pKey->closeKey(pNewKey);
        return _ret;
    }

    return pKey->closeKey(pNewKey);
}

//*********************************************************************
//  mergeKey
//
static RegError REGISTRY_CALLTYPE mergeKey(RegHandle hReg,
                                              RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           rtl_uString* regFileName,
                                           sal_Bool bWarnings,
                                           sal_Bool bReport)
{
    ORegistry   *pReg;
    ORegKey     *pKey, *pNewKey;
    RegError    _ret;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return(REG_REGISTRY_NOT_OPEN);
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    if (keyName->length)
    {
        if (_ret = pKey->createKey(keyName, (RegKeyHandle*)&pNewKey) )
            return _ret;
    } else
    {
        pNewKey = pKey;
    }

    /*
    OString tmpName = getTempName();
    pReg->saveKey(pNewKey, tmpName, sal_False, sal_False);
    */
    _ret = pKey->getRegistry()->loadKey(pNewKey, regFileName, bWarnings, bReport);
    /*
    if (_ret)
    {
        if (_ret == REG_MERGE_ERROR ||
            (_ret == REG_MERGE_CONFLICT && bWarnings))
        {
            pKey->closeKey(pNewKey);
            pKey->deleteKey(keyName);
            pKey->createKey(keyName, (RegKeyHandle*)&pNewKey);
            pReg->loadKey(pNewKey, tmpName, sal_False, sal_False);
        }

        destroyRegistry(hReg, tmpName);

        pKey->closeKey(pNewKey);
        return _ret;
    }

    destroyRegistry(hReg, tmpName);
    */
    if ( pKey != pNewKey )
        _ret = pKey->closeKey(pNewKey);

    return _ret;
}


//*********************************************************************
//  dumpRegistry
//
static RegError REGISTRY_CALLTYPE dumpRegistry(RegHandle hReg,
                                               RegKeyHandle hKey)
{
    ORegistry   *pReg;
    ORegKey     *pKey;

    if (hReg)
    {
        pReg = (ORegistry*)hReg;
        if (!pReg->isOpen())
            return REG_REGISTRY_NOT_OPEN;
    } else
    {
        return REG_INVALID_REGISTRY;
    }

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->getRegistry() != pReg || pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
    {
        return REG_INVALID_KEY;
    }

    return pReg->dumpRegistry(hKey);
}


//*********************************************************************
//  initRegistry_Api
//
void REGISTRY_CALLTYPE acquireKey(RegKeyHandle);
void REGISTRY_CALLTYPE releaseKey(RegKeyHandle);
sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle);
RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle, rtl_uString**);
RegError REGISTRY_CALLTYPE createKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openSubKeys(RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle*, sal_uInt32);
RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle);
RegError REGISTRY_CALLTYPE setValue(RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE setLongListValue(RegKeyHandle, rtl_uString*, sal_Int32*, sal_uInt32);
RegError REGISTRY_CALLTYPE setStringListValue(RegKeyHandle, rtl_uString*, sal_Char**, sal_uInt32);
RegError REGISTRY_CALLTYPE setUnicodeListValue(RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
RegError REGISTRY_CALLTYPE getValueInfo(RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
RegError REGISTRY_CALLTYPE getValue(RegKeyHandle, rtl_uString*, RegValue);
RegError REGISTRY_CALLTYPE getLongListValue(RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
RegError REGISTRY_CALLTYPE getStringListValue(RegKeyHandle, rtl_uString*, sal_Char***, sal_uInt32*);
RegError REGISTRY_CALLTYPE getUnicodeListValue(RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeValueList(RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE createLink(RegKeyHandle, rtl_uString*, rtl_uString*);
RegError REGISTRY_CALLTYPE deleteLink(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE getKeyType(RegKeyHandle, rtl_uString*, RegKeyType*);
RegError REGISTRY_CALLTYPE getLinkTarget(RegKeyHandle, rtl_uString*, rtl_uString**);
RegError REGISTRY_CALLTYPE getResolvedKeyName(RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
RegError REGISTRY_CALLTYPE getKeyNames(RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString**, sal_uInt32);


extern "C" Registry_Api* REGISTRY_CALLTYPE initRegistry_Api(void)
{
    static Registry_Api aApi= {&acquire,
                               &release,
                               &isReadOnly,
                               &openRootKey,
                               &getName,
                               &createRegistry,
                               &openRegistry,
                               &closeRegistry,
                               &destroyRegistry,
                               &loadKey,
                               &saveKey,
                               &mergeKey,
                               &dumpRegistry,
                               &acquireKey,
                               &releaseKey,
                               &isKeyReadOnly,
                               &getKeyName,
                               &createKey,
                               &openKey,
                               &openSubKeys,
                               &closeSubKeys,
                               &deleteKey,
                               &closeKey,
                               &setValue,
                               &setLongListValue,
                               &setStringListValue,
                               &setUnicodeListValue,
                               &getValueInfo,
                               &getValue,
                               &getLongListValue,
                               &getStringListValue,
                               &getUnicodeListValue,
                               &freeValueList,
                               &createLink,
                               &deleteLink,
                               &getKeyType,
                               &getLinkTarget,
                               &getResolvedKeyName,
                               &getKeyNames,
                               &freeKeyNames};

    return (&aApi);
}


//*********************************************************************
//  reg_loadRegKey
//
RegError REGISTRY_CALLTYPE reg_loadKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       rtl_uString* regFileName)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return loadKey(pKey->getRegistry(), hKey, keyName, regFileName);
}

//*********************************************************************
//  reg_saveKey
//
RegError REGISTRY_CALLTYPE reg_saveKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       rtl_uString* regFileName)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return saveKey(pKey->getRegistry(), hKey, keyName, regFileName);
}

//*********************************************************************
//  reg_mergeKey
//
RegError REGISTRY_CALLTYPE reg_mergeKey(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        rtl_uString* regFileName,
                                        sal_Bool bWarnings,
                                        sal_Bool bReport)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return mergeKey(pKey->getRegistry(), hKey, keyName, regFileName, bWarnings, bReport);
}

//*********************************************************************
//  reg_createRegistry
//
RegError REGISTRY_CALLTYPE reg_createRegistry(rtl_uString* registryName,
                                              RegHandle* phRegistry)
{
    RegError ret;

    ORegistry* pReg = new ORegistry();
    if (ret = pReg->initRegistry(registryName, REG_CREATE))
    {
        *phRegistry = NULL;
        return ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  reg_openRootKey
//
RegError REGISTRY_CALLTYPE reg_openRootKey(RegHandle hRegistry,
                                          RegKeyHandle* phRootKey)
{
    return openRootKey(hRegistry, phRootKey);
}


//*********************************************************************
//  reg_getName
//
const RegError REGISTRY_CALLTYPE reg_getName(RegHandle hRegistry, rtl_uString** pName)
{
    return getName(hRegistry, pName);
}


//*********************************************************************
//  reg_isReadOnly
//
sal_Bool REGISTRY_CALLTYPE reg_isReadOnly(RegHandle hRegistry)
{
    return isReadOnly(hRegistry);
}


//*********************************************************************
//  reg_openRegistry
//
RegError REGISTRY_CALLTYPE reg_openRegistry(rtl_uString* registryName,
                                            RegHandle* phRegistry,
                                            RegAccessMode accessMode)
{
    RegError _ret;

    ORegistry* pReg = new ORegistry();
    if (_ret = pReg->initRegistry(registryName, accessMode))
    {
        *phRegistry = NULL;
        return _ret;
    }

    *phRegistry = pReg;

    return REG_NO_ERROR;
}

//*********************************************************************
//  reg_closeRegistry
//
RegError REGISTRY_CALLTYPE reg_closeRegistry(RegHandle hRegistry)
{
    ORegistry* pReg;

    if (hRegistry)
    {
        pReg = (ORegistry*)hRegistry;
        delete(pReg);
        return REG_NO_ERROR;
    } else
    {
        return REG_REGISTRY_NOT_OPEN;
    }
}


//*********************************************************************
//  reg_destroyRegistry
//
RegError REGISTRY_CALLTYPE reg_destroyRegistry(RegHandle hRegistry,
                                               rtl_uString* registryName)
{
    return destroyRegistry(hRegistry, registryName);
}


//*********************************************************************
//  reg_dumpRegistry
//
RegError REGISTRY_CALLTYPE reg_dumpRegistry(RegKeyHandle hKey)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return dumpRegistry(pKey->getRegistry(), hKey);
}


