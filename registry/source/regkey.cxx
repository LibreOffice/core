/*************************************************************************
 *
 *  $RCSfile: regkey.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:43 $
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

#ifndef _REGISTRY_REGISTRY_HXX_
#include    <registry/registry.hxx>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _REGIMPL_HXX_
#include    "regimpl.hxx"
#endif

#ifndef _KEYIMPL_HXX_
#include    "keyimpl.hxx"
#endif


//*********************************************************************
//  acquireKey
//
void REGISTRY_CALLTYPE acquireKey(RegKeyHandle hKey)
{
    ORegKey* pKey = (ORegKey*) hKey;

    if (pKey != NULL)
        pKey->acquire();
}


//*********************************************************************
//  releaseKey
//
void REGISTRY_CALLTYPE releaseKey(RegKeyHandle hKey)
{
    ORegKey* pKey = (ORegKey*) hKey;

    if (pKey)
    {
        if (pKey->release() == 0)
            delete pKey;
    }
}


//*********************************************************************
//  isKeyReadOnly
//
sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle hKey)
{
    if (hKey)
    {
        return ((ORegKey*)hKey)->isReadOnly();
    } else
    {
        return sal_False;
    }
}


//*********************************************************************
//  getKeyName
//
RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle hKey, rtl_uString** pKeyName)
{
    if (hKey)
    {
        rtl_uString_assign( pKeyName, ((ORegKey*)hKey)->getName().pData );
        return REG_NO_ERROR;
    } else
    {
        rtl_uString_new(pKeyName);
        return REG_INVALID_KEY;
    }
}


//*********************************************************************
//  createKey
//
RegError REGISTRY_CALLTYPE createKey(RegKeyHandle hKey,
                                     rtl_uString* keyName,
                                     RegKeyHandle* phNewKey)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            phNewKey = NULL;
            return REG_INVALID_KEY;
        }
    } else
    {
        phNewKey = NULL;
        return REG_INVALID_KEY;
    }

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    return pKey->createKey(keyName, phNewKey);
}

//*********************************************************************
//  openKey
//
RegError REGISTRY_CALLTYPE openKey(RegKeyHandle hKey,
                                   rtl_uString* keyName,
                                   RegKeyHandle* phOpenKey)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            phOpenKey = NULL;
            return REG_INVALID_KEY;
        }
    } else
    {
        phOpenKey = NULL;
        return REG_INVALID_KEY;
    }

    return pKey->openKey(keyName, phOpenKey);
}

//*********************************************************************
//  openSubKeys
//
RegError REGISTRY_CALLTYPE openSubKeys(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegKeyHandle** pphSubKeys,
                                       sal_uInt32* pnSubKeys)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            pphSubKeys = NULL;
            return REG_INVALID_KEY;
        }
    } else
    {
        pphSubKeys = NULL;
        return REG_INVALID_KEY;
    }

    return pKey->openSubKeys(keyName, pphSubKeys, pnSubKeys);
}

//*********************************************************************
//  closeSubKeys
//
RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle* phSubKeys,
                                        sal_uInt32 nSubKeys)
{
    RegError _ret = REG_NO_ERROR;

    if (phSubKeys)
    {
        ORegistry* pReg = NULL;

        pReg = ((ORegKey*)phSubKeys[0])->getRegistry();

        for (int i=0; (i < nSubKeys) && (_ret == REG_NO_ERROR); i++)
        {
            _ret = pReg->closeKey(phSubKeys[i]);
        }

        rtl_freeMemory(phSubKeys);
    }
    else
        _ret = REG_INVALID_KEY;

    return _ret;
}


//*********************************************************************
//  deleteKey
//
RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle hKey,
                                     rtl_uString* keyName)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    return pKey->deleteKey(keyName);
}

//*********************************************************************
//  closeKey
//
RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle hKey)
{
    ORegKey*    pKey;
    ORegistry*  pReg;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        pReg = pKey->getRegistry();

        if (pReg->closeKey(hKey))
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    return REG_NO_ERROR;
}


//*********************************************************************
//  setValue
//
RegError REGISTRY_CALLTYPE setValue(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegValueType valueType,
                                       RegValue pData,
                                       sal_uInt32 valueSize)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
            return _ret1;

        pSubKey = (ORegKey*)hSubKey;
        if (_ret1 = pSubKey->setValue(valueName, valueType, pData, valueSize))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->setValue(valueName, valueType, pData, valueSize);
}

//*********************************************************************
//  setLongValueList
//
RegError REGISTRY_CALLTYPE setLongListValue(RegKeyHandle hKey,
                                                  rtl_uString* keyName,
                                                  sal_Int32* pValueList,
                                                  sal_uInt32 len)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
            return _ret1;

        pSubKey = (ORegKey*)hSubKey;
        if (_ret1 = pSubKey->setLongListValue(valueName, pValueList, len))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->setLongListValue(valueName, pValueList, len);
}

//*********************************************************************
//  setStringValueList
//
RegError REGISTRY_CALLTYPE setStringListValue(RegKeyHandle hKey,
                                                   rtl_uString* keyName,
                                                   sal_Char** pValueList,
                                                   sal_uInt32 len)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
            return _ret1;

        pSubKey = (ORegKey*)hSubKey;
        if (_ret1 = pSubKey->setStringListValue(valueName, pValueList, len))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->setStringListValue(valueName, pValueList, len);
}

//*********************************************************************
//  setUnicodeValueList
//
RegError REGISTRY_CALLTYPE setUnicodeListValue(RegKeyHandle hKey,
                                                     rtl_uString* keyName,
                                                     sal_Unicode** pValueList,
                                                     sal_uInt32 len)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
            return _ret1;

        pSubKey = (ORegKey*)hSubKey;
        if (_ret1 = pSubKey->setUnicodeListValue(valueName, pValueList, len))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
                return _ret2;
            else
                return _ret1;
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->setUnicodeListValue(valueName, pValueList, len);
}

//*********************************************************************
//  getValueInfo
//
RegError REGISTRY_CALLTYPE getValueInfo(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        RegValueType* pValueType,
                                        sal_uInt32* pValueSize)
{
    ORegKey*        pKey;
    RegValueType    valueType;
    sal_uInt32      valueSize;

    *pValueType = RG_VALUETYPE_NOT_DEFINED;
    *pValueSize = 0;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret;
        if (_ret = pKey->openKey(keyName, &hSubKey))
            return _ret;

        pSubKey = (ORegKey*)hSubKey;
        if (pSubKey->getValueInfo(valueName, &valueType, &valueSize))
        {
            if (_ret = pKey->closeKey(hSubKey))
                return _ret;
            else
                return REG_INVALID_VALUE;
        }

        *pValueType = valueType;
        *pValueSize = valueSize;

        return pKey->closeKey(hSubKey);
    }


    if (pKey->getValueInfo(valueName, &valueType, &valueSize))
    {
        return REG_INVALID_VALUE;
    }

    *pValueType = valueType;
    *pValueSize = valueSize;

    return REG_NO_ERROR;
}

//*********************************************************************
//  getValueInfo
//
RegError REGISTRY_CALLTYPE getValue(RegKeyHandle hKey,
                                    rtl_uString* keyName,
                                    RegValue pValue)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            pValue = NULL;
            return REG_INVALID_KEY;
        }
    } else
    {
        pValue = NULL;
        return REG_INVALID_KEY;
    }

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
        {
            pValue = NULL;
            return _ret1;
        }

        pSubKey = (ORegKey*)hSubKey;
        if (_ret1 = pSubKey->getValue(valueName, pValue))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
            {
                pValue = NULL;
                return _ret2;
            } else
            {
                pValue = NULL;
                return _ret1;
            }
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->getValue(valueName, pValue);
}

//*********************************************************************
//  getLongValueList
//
RegError REGISTRY_CALLTYPE getLongListValue(RegKeyHandle hKey,
                                            rtl_uString* keyName,
                                            sal_Int32** pValueList,
                                            sal_uInt32* pLen)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            pValueList = NULL;
            *pLen = 0;
            return REG_INVALID_KEY;
        }
    } else
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_INVALID_KEY;
    }

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
        {
            pValueList = NULL;
            *pLen = 0;
            return _ret1;
        }

        pSubKey = (ORegKey*)hSubKey;

        if (_ret1 = pSubKey->getLongListValue(valueName, pValueList, pLen))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret2;
            } else
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret1;
            }
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->getLongListValue(valueName, pValueList, pLen);
}

//*********************************************************************
//  getStringValueList
//
RegError REGISTRY_CALLTYPE getStringListValue(RegKeyHandle hKey,
                                              rtl_uString* keyName,
                                              sal_Char*** pValueList,
                                              sal_uInt32* pLen)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            pValueList = NULL;
            *pLen = 0;
            return REG_INVALID_KEY;
        }
    } else
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_INVALID_KEY;
    }

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if ((_ret1 = pKey->openKey(keyName, &hSubKey)))
        {
            pValueList = NULL;
            *pLen = 0;
            return _ret1;
        }

        pSubKey = (ORegKey*)hSubKey;

        if (_ret1 = pSubKey->getStringListValue(valueName, pValueList, pLen))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret2;
            } else
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret1;
            }
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->getStringListValue(valueName, pValueList, pLen);
}

//*********************************************************************
//  getUnicodeListValue
//
RegError REGISTRY_CALLTYPE getUnicodeListValue(RegKeyHandle hKey,
                                               rtl_uString* keyName,
                                               sal_Unicode*** pValueList,
                                               sal_uInt32* pLen)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
        {
            pValueList = NULL;
            *pLen = 0;
            return REG_INVALID_KEY;
        }
    } else
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_INVALID_KEY;
    }

    OUString valueName( RTL_CONSTASCII_USTRINGPARAM("value") );
    if (keyName->length)
    {
        RegKeyHandle hSubKey;
        ORegKey* pSubKey;
        RegError _ret1, _ret2;
        if (_ret1 = pKey->openKey(keyName, &hSubKey))
        {
            pValueList = NULL;
            *pLen = 0;
            return _ret1;
        }

        pSubKey = (ORegKey*)hSubKey;

        if (_ret1 = pSubKey->getUnicodeListValue(valueName, pValueList, pLen))
        {
            if (_ret2 = pKey->closeKey(hSubKey))
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret2;
            } else
            {
                pValueList = NULL;
                *pLen = 0;
                return _ret1;
            }
        }

        return pKey->closeKey(hSubKey);
    }

    return pKey->getUnicodeListValue(valueName, pValueList, pLen);
}

//*********************************************************************
//  freeValueList
//
RegError REGISTRY_CALLTYPE freeValueList(RegValueType valueType,
                                              RegValue pValueList,
                                              sal_uInt32 len)
{
    switch (valueType)
    {
        case 5:
            {
                sal_Int32* pVList = (sal_Int32*)pValueList;
                rtl_freeMemory(pValueList);
            }
            break;
        case 6:
            {
                sal_Char** pVList = (sal_Char**)pValueList;
                for (int i=0; i < len; i++)
                {
                    rtl_freeMemory(pVList[i]);
                }

                rtl_freeMemory(pVList);
            }
            break;
        case 7:
            {
                sal_Unicode** pVList = (sal_Unicode**)pValueList;
                for (int i=0; i < len; i++)
                {
                    rtl_freeMemory(pVList[i]);
                }

                rtl_freeMemory(pVList);
            }
            break;
        default:
            return REG_INVALID_VALUE;
    }

    pValueList = NULL;
    return REG_NO_ERROR;
}


//*********************************************************************
//  createLink
//
RegError REGISTRY_CALLTYPE createLink(RegKeyHandle hKey,
                                           rtl_uString* linkName,
                                      rtl_uString* linkTarget)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    return pKey->createLink(linkName, linkTarget);
}

//*********************************************************************
//  deleteLink
//
RegError REGISTRY_CALLTYPE deleteLink(RegKeyHandle hKey,
                                        rtl_uString* linkName)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    if (pKey->isReadOnly())
        return REG_REGISTRY_READONLY;

    return pKey->deleteLink(linkName);
}

//*********************************************************************
//  getKeyType
//
RegError REGISTRY_CALLTYPE getKeyType(RegKeyHandle hKey,
                                      rtl_uString* keyName,
                                         RegKeyType* pKeyType)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    return pKey->getKeyType(keyName, pKeyType);
}

//*********************************************************************
//  getLinkTarget
//
RegError REGISTRY_CALLTYPE getLinkTarget(RegKeyHandle hKey,
                                         rtl_uString* linkName,
                                           rtl_uString** pLinkTarget)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    OUString linkTarget;
    RegError ret = pKey->getLinkTarget(linkName, linkTarget);
    if (!ret)
        rtl_uString_assign(pLinkTarget, linkTarget.pData);
    return ret;
}


//*********************************************************************
//  getName
//
RegError REGISTRY_CALLTYPE getResolvedKeyName(RegKeyHandle hKey,
                                              rtl_uString* keyName,
                                              sal_Bool firstLinkOnly,
                                                rtl_uString** pResolvedName)
{
    ORegKey     *pKey;
    RegError    _ret = REG_NO_ERROR;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    OUString resolvedName;
    _ret = pKey->getResolvedKeyName(keyName, resolvedName, firstLinkOnly);
    if (!_ret)
        rtl_uString_assign(pResolvedName, resolvedName.pData);
    return _ret;
}


//*********************************************************************
//  getKeyNames
//
RegError REGISTRY_CALLTYPE getKeyNames(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                         rtl_uString*** pSubKeyNames,
                                         sal_uInt32* pnSubKeys)
{
    ORegKey*    pKey;

    if (hKey)
    {
        pKey = (ORegKey*)hKey;

        if (pKey->isDeleted())
            return REG_INVALID_KEY;
    } else
        return REG_INVALID_KEY;

    return pKey->getKeyNames(keyName, pSubKeyNames, pnSubKeys);
}

//*********************************************************************
//  freeKeyNames
//
RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString** pKeyNames,
                                          sal_uInt32 nKeys)
{
    for (int i=0; i <nKeys; i++)
    {
        rtl_uString_release(pKeyNames[i]);
    }

    rtl_freeMemory(pKeyNames);

    return REG_NO_ERROR;
}

//*********************************************************************
//  C API
//

//*********************************************************************
//  reg_createKey
//
RegError REGISTRY_CALLTYPE reg_createKey(RegKeyHandle hKey,
                                         rtl_uString* keyName,
                                         RegKeyHandle* phNewKey)
{
    if (!hKey)
         return REG_INVALID_KEY;

    return createKey(hKey, keyName, phNewKey);
}

//*********************************************************************
//  reg_openKey
//
RegError REGISTRY_CALLTYPE reg_openKey(RegKeyHandle hKey,
                                       rtl_uString* keyName,
                                       RegKeyHandle* phOpenKey)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return openKey(hKey, keyName, phOpenKey);
}

//*********************************************************************
//  reg_openSubKeys
//
RegError REGISTRY_CALLTYPE reg_openSubKeys(RegKeyHandle hKey,
                                           rtl_uString* keyName,
                                           RegKeyHandle** pphSubKeys,
                                           sal_uInt32* pnSubKeys)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return openSubKeys(hKey, keyName, pphSubKeys, pnSubKeys);
}

//*********************************************************************
//  reg_closeSubKeys
//
RegError REGISTRY_CALLTYPE reg_closeSubKeys(RegKeyHandle* pphSubKeys,
                                            sal_uInt32 nSubKeys)
{
    if (!pphSubKeys)
        return REG_INVALID_KEY;

    return closeSubKeys(pphSubKeys, nSubKeys);
}

//*********************************************************************
//  reg_deleteKey
//
RegError REGISTRY_CALLTYPE reg_deleteKey(RegKeyHandle hKey,
                                         rtl_uString* keyName)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return deleteKey(hKey, keyName);
}

//*********************************************************************
//  reg_closeKey
//
RegError REGISTRY_CALLTYPE reg_closeKey(RegKeyHandle hKey)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return closeKey(hKey);
}


//*********************************************************************
//  reg_getKeyName
//
const RegError REGISTRY_CALLTYPE reg_getKeyName(RegKeyHandle hKey, rtl_uString** pKeyName)
{
    if (hKey)
    {
        rtl_uString_assign( pKeyName, ((ORegKey*)hKey)->getName().pData );
        return REG_NO_ERROR;
    } else
    {
        rtl_uString_new( pKeyName );
        return REG_INVALID_KEY;
    }
}

//*********************************************************************
//  reg_setValue
//
RegError REGISTRY_CALLTYPE reg_setValue(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        RegValueType valueType,
                                        RegValue pData,
                                        sal_uInt32 valueSize)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return setValue(hKey, keyName, valueType, pData, valueSize);
}

//*********************************************************************
//  reg_setLongListValue
//
RegError REGISTRY_CALLTYPE reg_setLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32* pValueList,
                                                      sal_uInt32 len)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return setLongListValue(hKey, keyName, pValueList, len);
}

//*********************************************************************
//  reg_setStringListValue
//
RegError REGISTRY_CALLTYPE reg_setStringListValue(RegKeyHandle hKey,
                                                          rtl_uString* keyName,
                                                          sal_Char** pValueList,
                                                          sal_uInt32 len)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return setStringListValue(hKey, keyName, pValueList, len);
}

//*********************************************************************
//  reg_setUnicodeListValue
//
RegError REGISTRY_CALLTYPE reg_setUnicodeListValue(RegKeyHandle hKey,
                                                            rtl_uString* keyName,
                                                            sal_Unicode** pValueList,
                                                            sal_uInt32 len)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return setUnicodeListValue(hKey, keyName, pValueList, len);
}

//*********************************************************************
//  reg_getValueInfo
//
RegError REGISTRY_CALLTYPE reg_getValueInfo(RegKeyHandle hKey,
                                            rtl_uString* keyName,
                                            RegValueType* pValueType,
                                            sal_uInt32* pValueSize)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getValueInfo(hKey, keyName, pValueType, pValueSize);
}

//*********************************************************************
//  reg_getValueInfo
//
RegError REGISTRY_CALLTYPE reg_getValue(RegKeyHandle hKey,
                                        rtl_uString* keyName,
                                        RegValue pData)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getValue(hKey, keyName, pData);
}

//*********************************************************************
//  reg_getLongListValue
//
RegError REGISTRY_CALLTYPE reg_getLongListValue(RegKeyHandle hKey,
                                                      rtl_uString* keyName,
                                                      sal_Int32** pValueList,
                                                      sal_uInt32* pLen)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getLongListValue(hKey, keyName, pValueList, pLen);
}

//*********************************************************************
//  reg_getStringListValue
//
RegError REGISTRY_CALLTYPE reg_getStringListValue(RegKeyHandle hKey,
                                                       rtl_uString* keyName,
                                                       sal_Char*** pValueList,
                                                       sal_uInt32* pLen)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getStringListValue(hKey, keyName, pValueList, pLen);
}

//*********************************************************************
//  reg_getUnicodeListValue
//
RegError REGISTRY_CALLTYPE reg_getUnicodeListValue(RegKeyHandle hKey,
                                                         rtl_uString* keyName,
                                                         sal_Unicode*** pValueList,
                                                         sal_uInt32* pLen)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getUnicodeListValue(hKey, keyName, pValueList, pLen);
}

//*********************************************************************
//  reg_freeValueList
//
RegError REGISTRY_CALLTYPE reg_freeValueList(RegValueType valueType,
                                                  RegValue pValueList,
                                                  sal_uInt32 len)
{
    if (pValueList)
        return freeValueList(valueType, pValueList, len);
    else
        return REG_INVALID_VALUE;
}

//*********************************************************************
//  reg_createLink
//
RegError REGISTRY_CALLTYPE reg_createLink(RegKeyHandle hKey,
                                               rtl_uString* linkName,
                                          rtl_uString* linkTarget)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return createLink(hKey, linkName, linkTarget);
}

//*********************************************************************
//  reg_deleteLink
//
RegError REGISTRY_CALLTYPE reg_deleteLink(RegKeyHandle hKey,
                                            rtl_uString* linkName)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return deleteLink(hKey, linkName);
}

//*********************************************************************
//  reg_getKeyType
//
RegError REGISTRY_CALLTYPE reg_getKeyType(RegKeyHandle hKey,
                                          rtl_uString* keyName,
                                             RegKeyType* pKeyType)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getKeyType(hKey, keyName, pKeyType);
}

//*********************************************************************
//  reg_getLinkTarget
//
RegError REGISTRY_CALLTYPE reg_getLinkTarget(RegKeyHandle hKey,
                                             rtl_uString* linkName,
                                               rtl_uString** pLinkTarget)
{
    if (!hKey)
        return REG_INVALID_KEY;

    return getLinkTarget(hKey, linkName, pLinkTarget);
}


//*********************************************************************
//  reg_getResolvedKeyName
//
RegError REGISTRY_CALLTYPE reg_getResolvedKeyName(RegKeyHandle hKey,
                                                    rtl_uString* keyName,
                                                    sal_Bool firstLinkOnly,
                                                      rtl_uString** pResolvedName)
{
    ORegKey *pKey;

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        return REG_INVALID_KEY;

    return getResolvedKeyName(hKey, keyName, firstLinkOnly, pResolvedName);
}


