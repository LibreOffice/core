/*************************************************************************
 *
 *  $RCSfile: regimpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-14 09:36:00 $
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

#include    <string.h>
#include    <stdio.h>
#ifdef MAC
#include    <unistd.h>
#define strdup(str) strcpy((sal_Char*)malloc(strlen(str)+1),str)
#endif
#ifdef UNX
#include    <unistd.h>
#endif

#include    "regimpl.hxx"

#ifndef __REGISTRY_REFLREAD_HXX__
#include    <registry/reflread.hxx>
#endif

#ifndef __REGISTRY_REFLWRIT_HXX__
#include    <registry/reflwrit.hxx>
#endif

#ifndef __REFLCNST_HXX__
#include    "reflcnst.hxx"
#endif

#ifndef _KEYIMPL_HXX_
#include    "keyimpl.hxx"
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _RTL_MEMORY_H_
#include    <rtl/memory.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include    <rtl/ustrbuf.hxx>
#endif

#if defined ( GCC ) && ( defined ( SCO ) || defined ( OS2 ) )
NAMESPACE_VOS(ORealDynamicLoader)* NAMESPACE_VOS(ODynamicLoader)<RegistryTypeReader_Api>::m_pLoader = NULL;
#endif


extern "C" RegistryTypeReader_Api* TYPEREG_CALLTYPE initRegistryTypeReader_Api();
extern "C" RegistryTypeWriter_Api* TYPEREG_CALLTYPE initRegistryTypeWriter_Api();


//*********************************************************************
//  dumpType()
//
static sal_Bool dumpType(RegistryTypeReader& reader, const OString& sIndent)
{
    sal_uInt16 i, j;
    sal_Bool ret = sal_True;
    const sal_Char* indent = sIndent;

    if (reader.isValid())
    {
        fprintf(stdout, "minor version: %d\n", reader.getMinorVersion());
        fprintf(stdout, "%smajor version: %d\n", indent, reader.getMajorVersion());
        fprintf(stdout, "%s" ,indent);
        switch (reader.getTypeClass())
        {
            case RT_TYPE_INVALID:
                fprintf(stdout, "type: 'invalid'\n");
                break;
            case RT_TYPE_INTERFACE:
                {
                    fprintf(stdout, "type: 'interface'\n");
                    RTUik uik;

                    reader.getUik(uik);

                    fprintf(stdout, "%suik: { 0x%.8x-0x%.4x-0x%.4x-0x%.8x-0x%.8x }\n",
                            indent, uik.m_Data1, uik.m_Data2, uik.m_Data3, uik.m_Data4, uik.m_Data5);
                }
                break;
            case RT_TYPE_MODULE:
                fprintf(stdout, "type: 'module'\n");
                break;
            case RT_TYPE_STRUCT:
                fprintf(stdout, "type: 'struct'\n");
                break;
            case RT_TYPE_ENUM:
                fprintf(stdout, "type: 'enum'\n");
                break;
            case RT_TYPE_EXCEPTION:
                fprintf(stdout, "type: 'exception'\n");
                break;
            case RT_TYPE_TYPEDEF:
                fprintf(stdout, "type: 'typedef'\n");
                break;
            case RT_TYPE_SERVICE:
                fprintf(stdout, "type: 'service'\n");
                break;
            case RT_TYPE_CONSTANTS:
                fprintf(stdout, "type: 'constants'\n");
                break;
            case RT_TYPE_UNION:
                fprintf(stdout, "type: 'union'\n");
                break;
            default:
                fprintf(stdout, "type: <unknown>\n");
                break;
        }

        fprintf(stdout, "%sname: '%s'\n", indent, OUStringToOString(reader.getTypeName(), RTL_TEXTENCODING_UTF8).getStr());
        if (reader.getTypeClass() == RT_TYPE_UNION )
        {
            fprintf(stdout, "%sdiscriminant type: '%s'\n", indent, OUStringToOString(reader.getSuperTypeName(), RTL_TEXTENCODING_UTF8).getStr());
        } else
        {
            fprintf(stdout, "%ssuper name: '%s'\n", indent, OUStringToOString(reader.getSuperTypeName(), RTL_TEXTENCODING_UTF8).getStr());
        }
        fprintf(stdout, "%sDoku: \"%s\"\n", indent, OUStringToOString(reader.getDoku(), RTL_TEXTENCODING_UTF8).getStr());
        fprintf(stdout, "%sIDL source file: \"%s\"\n", indent, OUStringToOString(reader.getFileName(), RTL_TEXTENCODING_UTF8).getStr());
        fprintf(stdout, "%snumber of fields: %d\n", indent, reader.getFieldCount());
        sal_uInt16 fieldAccess = RT_ACCESS_INVALID;
        for (i = 0; i < reader.getFieldCount(); i++)
        {
            fprintf(stdout, "%sfield #%d:\n%s  name='%s'\n%s  type='%s'\n", indent,
                    i, indent, OUStringToOString(reader.getFieldName(i), RTL_TEXTENCODING_UTF8).getStr(),
                    indent, OUStringToOString(reader.getFieldType(i), RTL_TEXTENCODING_UTF8).getStr());

            fieldAccess = reader.getFieldAccess(i);
            if ( fieldAccess == RT_ACCESS_INVALID )
            {
                fprintf(stdout, "%s  access=INVALID\n", indent);
            }
            if ( (fieldAccess  & RT_ACCESS_READONLY) == RT_ACCESS_READONLY )
            {
                fprintf(stdout, "%s  access=READONLY\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_READWRITE) == RT_ACCESS_READWRITE )
            {
                fprintf(stdout, "%s  access=READWRITE\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_OPTIONAL) == RT_ACCESS_OPTIONAL )
            {
                fprintf(stdout, "%s  access=OPTIONAL\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_MAYBEVOID) == RT_ACCESS_MAYBEVOID )
            {
                fprintf(stdout, "%s  access=MAYBEVOID\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_BOUND) == RT_ACCESS_BOUND )
            {
                fprintf(stdout, "%s  access=BOUND\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_CONSTRAINED) == RT_ACCESS_CONSTRAINED )
            {
                fprintf(stdout, "%s  access=CONSTRAINED\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_TRANSIENT) == RT_ACCESS_TRANSIENT )
            {
                fprintf(stdout, "%s  access=TRANSIENT\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_MAYBEAMBIGUOUS) == RT_ACCESS_MAYBEAMBIGUOUS )
            {
                fprintf(stdout, "%s  access=MAYBEAMBIGUOUS\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_MAYBEDEFAULT) == RT_ACCESS_MAYBEDEFAULT )
            {
                fprintf(stdout, "%s  access=MAYBEDEFAULT\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_REMOVEABLE) == RT_ACCESS_REMOVEABLE )
            {
                fprintf(stdout, "%s  access=REMOVEABLE\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_DEFAULT) == RT_ACCESS_DEFAULT )
            {
                fprintf(stdout, "%s  access=DEFAULT\n", indent);
            }
            if ( (fieldAccess & RT_ACCESS_CONST) == RT_ACCESS_CONST )
            {
                   fprintf(stdout, "%s  access=CONST\n", indent);
            }

            RTConstValue constVal = reader.getFieldConstValue(i);

            if ( constVal.m_type != RT_TYPE_NONE )
            {
                fprintf(stdout, "%s  value = ", indent);

                switch (constVal.m_type)
                {
                    case RT_TYPE_BOOL:
                        if (constVal.m_value.aBool)
                            fprintf(stdout, "TRUE");
                        else
                            fprintf(stdout, "FALSE");
                        break;
                    case RT_TYPE_BYTE:
                        fprintf(stdout, "%d", (int)constVal.m_value.aByte);
                        break;
                    case RT_TYPE_INT16:
                        fprintf(stdout, "%d", constVal.m_value.aShort);
                        break;
                    case RT_TYPE_UINT16:
                        fprintf(stdout, "%u", constVal.m_value.aUShort);
                        break;
                    case RT_TYPE_INT32:
                        fprintf(stdout, "%d", constVal.m_value.aLong);
                        break;
                    case RT_TYPE_UINT32:
                        fprintf(stdout, "%u", constVal.m_value.aULong);
                        break;
                    case RT_TYPE_INT64:
                        fprintf(stdout, "%d", constVal.m_value.aHyper);
                        break;
                    case RT_TYPE_UINT64:
                        fprintf(stdout, "%u", constVal.m_value.aUHyper);
                        break;
                    case RT_TYPE_FLOAT:
                        fprintf(stdout, "%f", constVal.m_value.aFloat);
                        break;
                    case RT_TYPE_DOUBLE:
                        fprintf(stdout, "%f", constVal.m_value.aDouble);
                        break;
                    case RT_TYPE_STRING:
                        fprintf(stdout, "%s", OUStringToOString(constVal.m_value.aString, RTL_TEXTENCODING_UTF8).getStr());
                        break;
                    default:
                        break;
                }
            }

            fprintf(stdout, "\n%s  Doku: \"%s\"", indent, OUStringToOString(reader.getFieldDoku(i), RTL_TEXTENCODING_UTF8).getStr());
            fprintf(stdout, "\n%s  IDL source file: \"%s\"\n", indent, OUStringToOString(reader.getFieldFileName(i), RTL_TEXTENCODING_UTF8).getStr());
        }

        fprintf(stdout, "%snumber of methods: %d\n", indent, reader.getMethodCount());
        for (i = 0; i < reader.getMethodCount(); i++)
        {
            fprintf(stdout, "%smethod #%d: ", indent, i);

            switch (reader.getMethodMode(i))
            {
                case RT_MODE_INVALID:
                    fprintf(stdout, "<invalid mode> ");
                    break;
                case RT_MODE_ONEWAY:
                    fprintf(stdout, "[oneway] ");
                    break;
                case RT_MODE_ONEWAY_CONST:
                    fprintf(stdout, "[oneway, const] ");
                    break;
                case RT_MODE_TWOWAY:
                    break;
                case RT_MODE_TWOWAY_CONST:
                    fprintf(stdout, "[const] ");
                    break;
                default:
                    fprintf(stdout, "<unknown mode> ");
                    break;
            }

            fprintf(stdout, "%s %s(",
                    OUStringToOString(reader.getMethodReturnType(i), RTL_TEXTENCODING_UTF8).getStr(),
                    OUStringToOString(reader.getMethodName(i), RTL_TEXTENCODING_UTF8).getStr());

            for (j = 0; j < reader.getMethodParamCount(i); j++)
            {
                switch (reader.getMethodParamMode(i, j))
                {
                    case RT_PARAM_INVALID:
                        fprintf(stdout, "<invalid mode> ");
                        break;
                    case RT_PARAM_IN:
                        fprintf(stdout, "[in] ");
                        break;
                    case RT_PARAM_OUT:
                        fprintf(stdout, "[out] ");
                        break;
                    case RT_PARAM_INOUT:
                        fprintf(stdout, "[inout] ");
                        break;
                    default:
                        fprintf(stdout, "<unknown mode> ");
                        break;
                }

                fprintf(stdout, "%s %s",
                        OUStringToOString(reader.getMethodParamType(i, j), RTL_TEXTENCODING_UTF8).getStr(),
                        OUStringToOString(reader.getMethodParamName(i, j), RTL_TEXTENCODING_UTF8).getStr());

                if (j != reader.getMethodParamCount(i) - 1)
                    fprintf(stdout, ", ");
            }

            fprintf(stdout, ") ");

            if (reader.getMethodExcCount(i))
            {
                fprintf(stdout, "raises ");
                for (j = 0; j < reader.getMethodExcCount(i); j++)
                {
                    fprintf(stdout, "%s",
                            OUStringToOString(reader.getMethodExcType(i, j), RTL_TEXTENCODING_UTF8).getStr());
                    if (j != reader.getMethodExcCount(i) - 1)
                        fprintf(stdout, ", ");
                }
            }

            fprintf(stdout, "\n%s  Doku: \"%s\"\n", indent,
                    OUStringToOString(reader.getMethodDoku(i), RTL_TEXTENCODING_UTF8).getStr());
        }

        fprintf(stdout, "%snumber of references: %d\n", indent, reader.getReferenceCount());
        for (i = 0; i < reader.getReferenceCount(); i++)
        {
            fprintf(stdout, "%sreference #%d:\n%s  name='%s'\n", indent, i, indent,
                    OUStringToOString(reader.getReferenceName(i), RTL_TEXTENCODING_UTF8).getStr());
            switch (reader.getReferenceType(i))
            {
                case RT_REF_INVALID:
                    fprintf(stdout, "%s  type=INVALID\n", indent);
                    break;
                case RT_REF_SUPPORTS:
                    {
                        fprintf(stdout, "%s  type=supported interface\n", indent);
                        if (reader.getReferenceAccess(i) == RT_ACCESS_OPTIONAL)
                        {
                            fprintf(stdout, "%s  access=optional\n", indent);
                        }
                    }
                    break;
                case RT_REF_OBSERVES:
                    fprintf(stdout, "%s  type=observed interface\n", indent);
                    break;
                case RT_REF_EXPORTS:
                    fprintf(stdout, "%s  type=exported service\n", indent);
                    break;
                case RT_REF_NEEDS:
                    fprintf(stdout, "%s  type=needed service\n", indent);
                    break;
                default:
                    fprintf(stdout, "%s  type=<unknown>\n", indent);
                    break;
            }

            fprintf(stdout, "%s  Doku: \"%s\"\n", indent,
                    OUStringToOString(reader.getReferenceDoku(i), RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    else
    {
        ret = sal_False;
    }

    return ret;
}


//*********************************************************************
//  ORegistry()
//
ORegistry::ORegistry()
    : m_refCount(1)
    , m_readOnly(sal_False)
    , m_isOpen(sal_False)
    , ROOT( RTL_CONSTASCII_USTRINGPARAM("/") )
{
}

//*********************************************************************
//  ~ORegistry()
//
ORegistry::~ORegistry()
{
    if (m_openKeyTable.count(ROOT) > 0)
    {
        m_openKeyTable[ROOT]->release();
        delete(m_openKeyTable[ROOT]);
    }

    if (m_file.isValid())
        m_file.close();
}


//*********************************************************************
//  initRegistry
//
RegError ORegistry::initRegistry(const OUString& regName, RegAccessMode accessMode)
{
    OStoreFile      rRegFile;
    storeAccessMode sAccessMode = REG_MODE_OPEN;
    storeError      errCode;

    if (accessMode & REG_CREATE)
    {
        sAccessMode = REG_MODE_CREATE;
    } else
    if (accessMode & REG_READONLY)
    {
        sAccessMode = REG_MODE_OPENREAD;
        m_readOnly = sal_True;
    }

    if (errCode = rRegFile.create(regName, sAccessMode, REG_PAGESIZE))
    {
        switch (errCode)
        {
            case  store_E_NotExists:
                return REG_REGISTRY_NOT_EXISTS;
                break;
            case store_E_LockingViolation:
                return REG_CANNOT_OPEN_FOR_READWRITE;
                break;
            default:
                return REG_INVALID_REGISTRY;
                break;
        }
    } else
    {
        OStoreDirectory rStoreDir;
        storeError      _err = rStoreDir.create(rRegFile, OUString(), OUString(), sAccessMode);

        if ( _err == store_E_None )
        {
            m_file = rRegFile;
            m_name = regName;
            m_isOpen = sal_True;


            m_openKeyTable[ROOT] = new ORegKey(ROOT, rStoreDir, this);
            return REG_NO_ERROR;
        } else
            return REG_INVALID_REGISTRY;
    }
}


//*********************************************************************
//  closeRegistry
//
RegError ORegistry::closeRegistry()
{
    REG_GUARD(m_mutex);

    if (m_file.isValid())
    {
        closeKey(m_openKeyTable[ROOT]);
        m_file.close();
        m_isOpen = sal_False;
        return REG_NO_ERROR;
    } else
    {
        return REG_REGISTRY_NOT_EXISTS;
    }
}


//*********************************************************************
//  destroyRegistry
//
RegError ORegistry::destroyRegistry(const OUString& regName)
{
    REG_GUARD(m_mutex);

    if (regName.getLength())
    {
        ORegistry* pReg = new ORegistry();

        if (!pReg->initRegistry(regName, REG_READWRITE))
        {
            delete pReg;

            OString name( OUStringToOString(regName, RTL_TEXTENCODING_UTF8) );
            if (unlink(name) != 0)
            {
                return REG_DESTROY_REGISTRY_FAILED;
            }
        } else
        {
            return REG_DESTROY_REGISTRY_FAILED;
        }
    } else
    {
        if (m_refCount != 1 || isReadOnly())
        {
            return REG_DESTROY_REGISTRY_FAILED;
        }

        if (m_file.isValid())
        {
            closeKey(m_openKeyTable[ROOT]);
            m_file.close();
            m_isOpen = sal_False;

            OString name( OUStringToOString(m_name, RTL_TEXTENCODING_UTF8) );
            if (unlink(name) != 0)
            {
                return REG_DESTROY_REGISTRY_FAILED;
            }
        } else
        {
            return REG_REGISTRY_NOT_EXISTS;
        }
    }

    return REG_NO_ERROR;
}

//*********************************************************************
//  createKey
//
RegError ORegistry::createKey(RegKeyHandle hKey, const OUString& keyName,
                              RegKeyHandle* phNewKey)
{
    ORegKey*    pKey;

    *phNewKey = NULL;

    if ( !keyName.getLength() )
        return REG_INVALID_KEYNAME;

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

    OUString sFullKeyName = resolveLinks(pKey, keyName);

    if ( !sFullKeyName.getLength() )
        return REG_DETECT_RECURSION;

    if (m_openKeyTable.count(sFullKeyName) > 0)
    {
        *phNewKey = m_openKeyTable[sFullKeyName];
        ((ORegKey*)*phNewKey)->acquire();
        ((ORegKey*)*phNewKey)->setDeleted(sal_False);
        return REG_NO_ERROR;
    }

    OStoreDirectory rStoreDir;
    OUStringBuffer  sFullPath(sFullKeyName.getLength());
    sal_Int32       tokenCount = sFullKeyName.getTokenCount('/');
    OUString        token;

    sFullPath.append((sal_Unicode)'/');

    for (sal_Int32 i=0; i < tokenCount; i++)
    {
        token = sFullKeyName.getToken(i, '/');
        if (token.getLength())
        {
            if (rStoreDir.create(pKey->getStoreFile(), sFullPath.getStr(), token, KEY_MODE_CREATE))
            {
                return REG_CREATE_KEY_FAILED;
            }

            sFullPath.append(token);
            sFullPath.append((sal_Unicode)'/');
        }
    }

    pKey = new ORegKey(sFullKeyName, rStoreDir, this);
    *phNewKey = pKey;
    m_openKeyTable[sFullKeyName] = pKey;

    return REG_NO_ERROR;
}


//*********************************************************************
//  openKey
//
RegError ORegistry::openKey(RegKeyHandle hKey, const OUString& keyName,
                            RegKeyHandle* phOpenKey, RESOLVE eResolve)
{
    ORegKey*        pKey;
    storeAccessMode accessMode = KEY_MODE_OPEN;

    *phOpenKey = NULL;

    if ( !keyName.getLength() )
    {
        return REG_INVALID_KEYNAME;
    }

    if ( isReadOnly() )
    {
        accessMode = KEY_MODE_OPENREAD;
    }

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

    OUString    sFullKeyName;
    OUString    sFullPath;
    OUString    sRelativKey;

    switch (eResolve)
    {
        case RESOLVE_FULL:
            {
                sFullKeyName = resolveLinks(pKey, keyName);

                if ( !sFullKeyName.getLength() )
                    return REG_DETECT_RECURSION;

                sal_Int32 lastIndex = sFullKeyName.lastIndexOf('/');
                sRelativKey = sFullKeyName.copy(lastIndex + 1);
                   sFullPath = sFullKeyName.copy(0, lastIndex + 1);
            }
            break;
        case RESOLVE_PART:
            {
                sal_Int32 lastIndex = keyName.lastIndexOf('/');
                if ( lastIndex >= 0 )
                {
                    OUString sRelativ(keyName.copy(lastIndex));
                    OUString tmpKey(keyName.copy(0, lastIndex + 1));
                    sFullKeyName = resolveLinks(pKey, tmpKey);

                    sFullPath = sFullKeyName;
                    sFullPath += ROOT;
                    sFullKeyName += sRelativ;
                    sRelativKey = sRelativ.copy(1);
                } else
                {
                    sFullKeyName = pKey->getName();
                    sFullPath = sFullKeyName;

                    sRelativKey = keyName;

                    if ( sFullKeyName.getLength() > 1 )
                        sFullKeyName += ROOT;

                    sFullKeyName += keyName;

                    if ( sFullPath.getLength() > 1 )
                        sFullPath += ROOT;
                }
            }
            break;
        case RESOLVE_NOTHING:
            {
                sFullKeyName = pKey->getName();
                sFullPath = sFullKeyName;

                if (sFullKeyName.getLength() > 1)
                    sFullKeyName += ROOT;

                sal_Int32 lastIndex = keyName.lastIndexOf('/');
                if ( lastIndex >= 0 && lastIndex < keyName.getLength() )
                {
                    OUString sRelativ(keyName.copy(lastIndex+1));
                    sRelativKey = sRelativ;
                    sFullKeyName += keyName.copy(1);

                    sFullPath = sFullKeyName.copy(0, keyName.lastIndexOf('/') + 1);
                } else
                {

                    sRelativKey += keyName;
                    sFullKeyName += keyName;

                    if ( sFullPath.getLength() > 1 )
                        sFullPath += ROOT;
                }
            }
            break;
    }

    if (m_openKeyTable.count(sFullKeyName) > 0)
    {
        m_openKeyTable[sFullKeyName]->acquire();
        *phOpenKey = m_openKeyTable[sFullKeyName];
        return REG_NO_ERROR;
    }

    OStoreDirectory rStoreDir;
    storeError      _err = rStoreDir.create(pKey->getStoreFile(), sFullPath, sRelativKey, accessMode);

    if (_err == store_E_NotExists)
        return REG_KEY_NOT_EXISTS;
    else
    if (_err == store_E_WrongFormat)
        return REG_INVALID_KEY;

    pKey = new ORegKey(sFullKeyName, rStoreDir, this);

    *phOpenKey = pKey;
    m_openKeyTable[sFullKeyName] = pKey;

    return REG_NO_ERROR;
}


//*********************************************************************
//  closeKey
//
RegError ORegistry::closeKey(RegKeyHandle hKey)
{
    ORegKey* pKey = (ORegKey*)hKey;

    REG_GUARD(m_mutex);

    if (m_openKeyTable.count(pKey->getName()) > 0)
    {
        if (pKey->getRefCount() == 1)
        {
            m_openKeyTable.erase(pKey->getName());
            delete(pKey);
            hKey = NULL;
        } else
        {
            pKey->release();
        }

        return REG_NO_ERROR;
    } else
    {
        return REG_KEY_NOT_OPEN;
    }
}


//*********************************************************************
//  deleteKey
//
RegError ORegistry::deleteKey(RegKeyHandle hKey, const OUString& keyName)
{
    ORegKey*    pKey;
    RegError    _ret = REG_NO_ERROR;

    if ( !keyName.getLength() )
    {
        return REG_INVALID_KEYNAME;
    }

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

    OUString sFullKeyName = resolveLinks(pKey, keyName);

    if ( !sFullKeyName.getLength() )
        return REG_DETECT_RECURSION;

    pKey = m_openKeyTable[ROOT];
    _ret = eraseKey(pKey, sFullKeyName, RESOLVE_NOTHING);

    return _ret;
}

RegError ORegistry::eraseKey(ORegKey* pKey, const OUString& keyName, RESOLVE eResolve)
{
    RegError _ret = REG_NO_ERROR;

    if ( !keyName.getLength() )
    {
        return REG_INVALID_KEYNAME;
    }

    OUString     sFullKeyName(pKey->getName());
    OUString     sFullPath(sFullKeyName);
    OUString     sRelativKey;
    sal_Int32    lastIndex = keyName.lastIndexOf('/');

    if ( lastIndex >= 0 )
    {
        sRelativKey += keyName.copy(lastIndex + 1);

        if (sFullKeyName.getLength() > 1)
            sFullKeyName += keyName;
        else
            sFullKeyName += (keyName+1);

        sFullPath = sFullKeyName.copy(0, keyName.lastIndexOf('/') + 1);
    } else
    {
        if (sFullKeyName.getLength() > 1)
            sFullKeyName += ROOT;

        sRelativKey += keyName;
        sFullKeyName += keyName;

        if (sFullPath.getLength() > 1)
            sFullPath += ROOT;
    }

    RegKeyHandle    hOldKey;
    if (_ret = pKey->openKey(keyName, &hOldKey, eResolve))
    {
        return _ret;
    }

    if (_ret = deleteSubkeysAndValues((ORegKey*)hOldKey, eResolve))
    {
        pKey->closeKey(hOldKey);
        return _ret;
    }

    OUString tmpName(sRelativKey);
    tmpName += ROOT;

    OStoreFile sFile(pKey->getStoreFile());

    if ( sFile.isValid() && sFile.remove(sFullPath, tmpName) )
    {
        return REG_DELETE_KEY_FAILED;
    }

    // set flag deleted !!!
    ((ORegKey*)hOldKey)->setDeleted(sal_True);

    if (_ret = pKey->closeKey(hOldKey))
    {
        return _ret;
    }

    return REG_NO_ERROR;
}


//*********************************************************************
//  deleteSubKeys
//
RegError ORegistry::deleteSubkeysAndValues(ORegKey* pKey, RESOLVE eResolve)
{
    OStoreDirectory::iterator   iter;
    OUString                    keyName;
    RegError                    _ret = REG_NO_ERROR;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while ( _err == store_E_None )
    {
        keyName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            if ((_ret = eraseKey(pKey, keyName, eResolve)))
                return _ret;
        } else
        {
            OUString sFullPath(pKey->getName());

            if (sFullPath.getLength() > 1)
                sFullPath += ROOT;

            if ( ((OStoreFile&)pKey->getStoreFile()).remove(sFullPath, keyName) )
            {
                return REG_DELETE_VALUE_FAILED;
            }
        }

        _err = rStoreDir.next(iter);
    }

    return REG_NO_ERROR;
}


//*********************************************************************
//  loadKey
//
RegError ORegistry::loadKey(RegKeyHandle hKey, const OUString& regFileName,
                            sal_Bool bWarnings, sal_Bool bReport)
{
    RegError        _ret = REG_NO_ERROR;
    ORegistry*      pReg;
    ORegKey         *pKey, *pRootKey;

    pReg = new ORegistry();
    if (_ret = pReg->initRegistry(regFileName, REG_READONLY))
    {
        return _ret;
    }

    pKey = (ORegKey*)hKey;
    pRootKey = pReg->getRootKey();

    REG_GUARD(m_mutex);

    OStoreDirectory::iterator   iter;
    OUString                    keyName;
    OStoreDirectory             rStoreDir(pRootKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while ( _err == store_E_None )
    {
        keyName = iter.m_pszName;

        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            _ret = loadAndSaveKeys(pKey, pRootKey, keyName, 0, bWarnings, bReport);
        } else
        {
            _ret = loadAndSaveValue(pKey, pRootKey, keyName, 0, bWarnings, bReport);
        }

        if (_ret == REG_MERGE_ERROR ||
            (_ret == REG_MERGE_CONFLICT && bWarnings))
        {
            rStoreDir = OStoreDirectory();
            pRootKey->release();
            delete(pReg);
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    rStoreDir = OStoreDirectory();
    pRootKey->release();
    delete(pReg);
    return _ret;
}


//*********************************************************************
//  loadKey
//
RegError ORegistry::saveKey(RegKeyHandle hKey, const OUString& regFileName,
                            sal_Bool bWarnings, sal_Bool bReport)
{
    RegError        _ret = REG_NO_ERROR;
    ORegistry*      pReg;
    ORegKey         *pKey, *pRootKey;

    pReg = new ORegistry();
    if (_ret = pReg->initRegistry(regFileName, REG_CREATE))
    {
        return _ret;
    }

    pKey = (ORegKey*)hKey;
    pRootKey = pReg->getRootKey();

    REG_GUARD(m_mutex);

    OStoreDirectory::iterator   iter;
    OUString                    keyName;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while ( _err == store_E_None )
    {
        keyName = iter.m_pszName;

        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            _ret = loadAndSaveKeys(pRootKey, pKey, keyName,
                                   pKey->getName().getLength(),
                                   bWarnings, bReport);
        } else
        {
            _ret = loadAndSaveValue(pRootKey, pKey, keyName,
                                    pKey->getName().getLength(),
                                    bWarnings, bReport);
        }

        if (_ret)
        {
            pRootKey->release();
            delete(pReg);
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    pRootKey->release();
    delete(pReg);
    return REG_NO_ERROR;
}


//*********************************************************************
//  isKeyOpen()
//
sal_Bool ORegistry::isKeyOpen(const OUString& keyName) const
{
    return(m_openKeyTable.count(keyName) > 0);
}



//*********************************************************************
//  countSubKeys()
//
sal_uInt32 ORegistry::countSubKeys(ORegKey* pKey)
{
    REG_GUARD(m_mutex);

    OStoreDirectory::iterator   iter;
    sal_uInt32                  count = 0;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    while ( _err == store_E_None)
    {
        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            count++;
        }

        _err = rStoreDir.next(iter);
    }

    return count;
}


//*********************************************************************
//  loadValue()
//
RegError ORegistry::loadAndSaveValue(ORegKey* pTargetKey,
                                     ORegKey* pSourceKey,
                                     const OUString& valueName,
                                     sal_uInt32 nCut,
                                     sal_Bool bWarnings,
                                     sal_Bool bReport)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt32      nSize;
    storeAccessMode sourceAccess = VALUE_MODE_OPEN;
    OUString        sTargetPath(pTargetKey->getName());
    OUString        sSourcePath(pSourceKey->getName());

    if (pSourceKey->isReadOnly())
    {
        sourceAccess = VALUE_MODE_OPENREAD;
    }

    if (nCut)
    {
        sTargetPath = sSourcePath.copy(nCut);
    } else
    {
        if (sTargetPath.getLength() > 1)
        {
            if (sSourcePath.getLength() > 1)
                sTargetPath += sSourcePath;
        } else
            sTargetPath = sSourcePath;
    }

    if (sTargetPath.getLength() > 1) sTargetPath += ROOT;
    if (sSourcePath.getLength() > 1) sSourcePath += ROOT;

    if (rValue.create(pSourceKey->getStoreFile(), sSourcePath, valueName, sourceAccess))
    {
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  rwBytes;
    if (rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, rwBytes))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (rwBytes != VALUE_HEADERSIZE)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    RegError _ret = REG_NO_ERROR;
    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;
    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);
    rtl_freeMemory(pBuffer);

    nSize = VALUE_HEADERSIZE + valueSize;
    pBuffer = (sal_uInt8*)rtl_allocateMemory(nSize);

    if (rValue.readAt(0, pBuffer, nSize, rwBytes))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (rwBytes != nSize)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    OStoreFile  rTargetFile(pTargetKey->getStoreFile());

    if (!rValue.create(rTargetFile, sTargetPath, valueName, VALUE_MODE_OPEN))
    {
        if (valueType == RG_VALUETYPE_BINARY)
        {
            if (_ret = checkBlop(rValue, pTargetKey, sTargetPath,
                                 valueName, valueSize, pBuffer+VALUE_HEADEROFFSET,
                                 bReport))
            {
                if (_ret == REG_MERGE_ERROR ||
                    (_ret == REG_MERGE_CONFLICT && bWarnings))
                {
                    rtl_freeMemory(pBuffer);
                    return _ret;
                }
            } else
            {
                rtl_freeMemory(pBuffer);
                return _ret;
            }
        }
    }

    // write
    if (rValue.create(rTargetFile, sTargetPath, valueName, VALUE_MODE_CREATE))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (rValue.writeAt(0, pBuffer, nSize, rwBytes))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    if (rwBytes != nSize)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    rtl_freeMemory(pBuffer);
    return _ret;
}


//*********************************************************************
//  checkblop()
//
RegError ORegistry::checkBlop(OStoreStream& rValue,
                              ORegKey* pTargetKey,
                              const OUString& sTargetPath,
                              const OUString& valueName,
                              sal_uInt32 srcValueSize,
                              sal_uInt8* pSrcBuffer,
                              sal_Bool bReport)
{
    RegistryTypeReader_Api* pReaderApi;

    pReaderApi = initRegistryTypeReader_Api();

    RegistryTypeReader reader(pReaderApi, pSrcBuffer, srcValueSize, sal_False);

    if (reader.getTypeClass() == RT_TYPE_INVALID)
    {
        return REG_INVALID_VALUE;
    }

    sal_uInt8*      pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);
    RegValueType    valueType;
    sal_uInt32      valueSize;
    sal_uInt32      rwBytes;
    OString         targetPath( OUStringToOString(sTargetPath, RTL_TEXTENCODING_UTF8) );

    if (!rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, rwBytes) &&
        (rwBytes == VALUE_HEADERSIZE))
    {
        sal_uInt8 type = *((sal_uInt8*)pBuffer);
        valueType = (RegValueType)type;
        readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);
        rtl_freeMemory(pBuffer);

        if (valueType == RG_VALUETYPE_BINARY)
        {
            pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, rwBytes) &&
                (rwBytes == valueSize))
            {
                RegistryTypeReader reader2(pReaderApi, pBuffer, valueSize, sal_False);

                if ((reader.getTypeClass() != reader2.getTypeClass())
                    || reader2.getTypeClass() == RT_TYPE_INVALID)
                {
                    rtl_freeMemory(pBuffer);

                    if (bReport)
                    {
                        fprintf(stdout, "ERROR: values of blop from key \"%s\" has different types.\n",
                                targetPath.getStr());
                    }
                    return REG_MERGE_ERROR;
                }

                if (reader.getTypeClass() == RT_TYPE_MODULE)
                {
                    if (reader.getFieldCount() > 0 &&
                        reader2.getFieldCount() > 0)
                    {
                        mergeModuleValue(rValue, pTargetKey, sTargetPath,
                                         valueName, reader, reader2);

                        rtl_freeMemory(pBuffer);
                        return REG_NO_ERROR;
                    } else
                    if (reader2.getFieldCount() > 0)
                    {
                        rtl_freeMemory(pBuffer);
                        return REG_NO_ERROR;
                    } else
                    {
                        rtl_freeMemory(pBuffer);
                        return REG_MERGE_CONFLICT;
                    }
                } else
                {
                    rtl_freeMemory(pBuffer);

                    if (bReport)
                    {
                        fprintf(stdout, "WARNING: value of key \"%s\" already exists.\n",
                                targetPath.getStr());
                    }
                    return REG_MERGE_CONFLICT;
                }
            } else
            {
                rtl_freeMemory(pBuffer);
                if (bReport)
                {
                    fprintf(stdout, "ERROR: values of key \"%s\" contains bad data.\n",
                            targetPath.getStr());
                }
                return REG_MERGE_ERROR;
            }
        } else
        {
            rtl_freeMemory(pBuffer);
            if (bReport)
            {
                fprintf(stdout, "ERROR: values of key \"%s\" has different types.\n",
                        targetPath.getStr());
            }
            return REG_MERGE_ERROR;
        }
    } else
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
}

static sal_uInt32 checkTypeReaders(RegistryTypeReader& reader1,
                                   RegistryTypeReader& reader2,
                                   StringSet& nameSet)
{
    sal_uInt32 count=0;
    sal_uInt16 i;
    for (i=0 ; i < reader1.getFieldCount(); i++)
    {
        nameSet.insert(reader1.getFieldName(i));
        count++;
    }
    for (i=0 ; i < reader2.getFieldCount(); i++)
    {
        if (nameSet.find(reader2.getFieldName(i)) == nameSet.end())
        {
            nameSet.insert(reader2.getFieldName(i));
            count++;
        }
    }
    return count;
}

//*********************************************************************
//  mergeModuleValue()
//
RegError ORegistry::mergeModuleValue(OStoreStream& rTargetValue,
                                     ORegKey* pTargetKey,
                                     const OUString& sTargetPath,
                                     const OUString& valueName,
                                     RegistryTypeReader& reader,
                                     RegistryTypeReader& reader2)
{
    sal_uInt16                  index = 0;
    RegistryTypeWriter_Api*     pWriterApi;

    StringSet nameSet;
    sal_uInt32 count = checkTypeReaders(reader, reader2, nameSet);

    if (count != reader.getFieldCount())
    {
        pWriterApi = initRegistryTypeWriter_Api();

        RegistryTypeWriter writer(pWriterApi,
                                  reader.getTypeClass(),
                                  reader.getTypeName(),
                                  reader.getSuperTypeName(),
                                  (sal_uInt16)count,
                                  0,
                                  0);

        sal_uInt16 i;
        for (i=0 ; i < reader.getFieldCount(); i++)
        {
            writer.setFieldData(index,
                               reader.getFieldName(i),
                               reader.getFieldType(i),
                               reader.getFieldDoku(i),
                               reader.getFieldFileName(i),
                               reader.getFieldAccess(i),
                               reader.getFieldConstValue(i));
            index++;
        }
        for (i=0 ; i < reader2.getFieldCount(); i++)
        {
            if (nameSet.find(reader2.getFieldName(i)) == nameSet.end())
            {
                writer.setFieldData(index,
                                   reader2.getFieldName(i),
                                   reader2.getFieldType(i),
                                   reader2.getFieldDoku(i),
                                   reader2.getFieldFileName(i),
                                   reader2.getFieldAccess(i),
                                   reader2.getFieldConstValue(i));
                index++;
            }
        }

        const sal_uInt8*    pBlop = writer.getBlop();
        sal_uInt32          aBlopSize = writer.getBlopSize();

        sal_uInt8   type = (sal_uInt8)RG_VALUETYPE_BINARY;
        sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE + aBlopSize);

        rtl_copyMemory(pBuffer, &type, 1);
        writeUINT32(pBuffer+VALUE_TYPEOFFSET, aBlopSize);
        rtl_copyMemory(pBuffer+VALUE_HEADEROFFSET, pBlop, aBlopSize);

        sal_uInt32  rwBytes;
        if (rTargetValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+aBlopSize, rwBytes))
        {
            rtl_freeMemory(pBuffer);
            return REG_INVALID_VALUE;
        }

        if (rwBytes != VALUE_HEADERSIZE+aBlopSize)
        {
            rtl_freeMemory(pBuffer);
            return REG_INVALID_VALUE;
        }

        rtl_freeMemory(pBuffer);
    }
    return REG_NO_ERROR;
}

//*********************************************************************
//  loadKeys()
//
RegError ORegistry::loadAndSaveKeys(ORegKey* pTargetKey,
                                    ORegKey* pSourceKey,
                                    const OUString& keyName,
                                    sal_uInt32 nCut,
                                    sal_Bool bWarnings,
                                    sal_Bool bReport)
{
    ORegKey*    pTmpKey;
    RegError    _ret = REG_NO_ERROR;
    OUString    sRelPath(pSourceKey->getName().copy(nCut));
    OUString    sFullPath;

    if(pTargetKey->getName().getLength() > 1)
        sFullPath += pTargetKey->getName();
    sFullPath += sRelPath;
    if (sRelPath.getLength() > 1 || sFullPath.getLength() == 0)
        sFullPath += ROOT;

    OUString        sFullKeyName = sFullPath;
    OStoreDirectory rStoreDir;

    sFullKeyName += keyName;

    if (rStoreDir.create(pTargetKey->getStoreFile(), sFullPath, keyName, KEY_MODE_CREATE))
    {
        return REG_CREATE_KEY_FAILED;
    }

    if (m_openKeyTable.count(sFullKeyName) > 0)
    {
        m_openKeyTable[sFullKeyName]->setDeleted(sal_False);
    }


    if (_ret = pSourceKey->openKey(keyName, (RegKeyHandle*)&pTmpKey, RESOLVE_NOTHING))
    {
        return _ret;
    }

    OStoreDirectory::iterator   iter;
    OUString                    sName;
    OStoreDirectory             rTmpStoreDir(pTmpKey->getStoreDir());
    storeError                  _err = rTmpStoreDir.first(iter);

    while ( _err == store_E_None)
    {
        sName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = loadAndSaveKeys(pTargetKey, pTmpKey,
                                   sName, nCut, bWarnings, bReport);
        } else
        {
            _ret = loadAndSaveValue(pTargetKey, pTmpKey,
                                    sName, nCut, bWarnings, bReport);
        }

        if (_ret == REG_MERGE_ERROR ||
            (_ret == REG_MERGE_CONFLICT && bWarnings))
        {
            pSourceKey->closeKey(pTmpKey);
            return _ret;
        }

        _err = rTmpStoreDir.next(iter);
    }

    pSourceKey->closeKey(pTmpKey);
    return _ret;
}


//*********************************************************************
//  getRootKey()
//
ORegKey* ORegistry::getRootKey()
{
    m_openKeyTable[ROOT]->acquire();
    return m_openKeyTable[ROOT];
}


//*********************************************************************
//  getResolvedKeyName()
//
RegError ORegistry::getResolvedKeyName(RegKeyHandle hKey,
                                       const OUString& keyName,
                                       OUString& resolvedName,
                                       sal_Bool firstLinkOnly)
{
    ORegKey*    pKey;

    if ( !keyName.getLength() )
        return REG_INVALID_KEYNAME;

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

       resolvedName = resolveLinks(pKey, keyName, firstLinkOnly);

    if ( resolvedName.getLength() )
        return REG_NO_ERROR;
    else
        return REG_DETECT_RECURSION;
}

//*********************************************************************
//  dumpRegistry()
//
RegError ORegistry::dumpRegistry(RegKeyHandle hKey) const
{
    ORegKey                     *pKey = (ORegKey*)hKey;
    OUString                    sName;
    RegError                    _ret = REG_NO_ERROR;
    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    OString regName( OUStringToOString( getName(), RTL_TEXTENCODING_UTF8 ) );
    OString keyName( OUStringToOString( pKey->getName(), RTL_TEXTENCODING_UTF8 ) );
    fprintf(stdout, "Registry \"%s\":\n\n%s\n", regName.getStr(), keyName.getStr());

    while ( _err == store_E_None )
    {
        sName = iter.m_pszName;

        if (iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            _ret = dumpKey(pKey->getName(), sName, 1);
        } else
        {
            _ret = dumpValue(pKey->getName(), sName, 1);
        }

        if (_ret)
        {
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    return REG_NO_ERROR;
}


//*********************************************************************
//  dumpValue()
//
RegError ORegistry::dumpValue(const OUString& sPath, const OUString& sName, sal_Int16 nSpc) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    sal_uInt32      valueSize;
    RegValueType    valueType;
    OUString        sFullPath(sPath);
    OString         sIndent;
    storeAccessMode accessMode = VALUE_MODE_OPEN;
    sal_Bool        bLinkValue = sal_False;

    if (isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    for (int i= 0; i < nSpc; i++) sIndent += " ";

    if (sFullPath.getLength() > 1)
    {
        sFullPath += ROOT;
    }
    if (rValue.create(m_file, sFullPath, sName, accessMode))
    {
        return REG_VALUE_NOT_EXISTS;
    }

    OUString tmpName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    tmpName += OUString( RTL_CONSTASCII_USTRINGPARAM("LINK_TARGET") );
    if (sName == tmpName)
    {
        bLinkValue = sal_True;
    }
    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  rwBytes;
    if (rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, rwBytes))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (rwBytes != (VALUE_HEADERSIZE))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt8 type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;
    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
    if (rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, rwBytes))
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (rwBytes != valueSize)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    const sal_Char* indent = sIndent.getStr();
    switch (valueType)
    {
        case 0:
            fprintf(stdout, "%sValue: Type = VALUETYPE_NOT_DEFINED\n", indent);
            break;
        case 1:
            {
                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_LONG\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Data = ", indent);

                sal_Int32 value;
                readINT32(pBuffer, value);
                fprintf(stdout, "%d\n", value);
            }
            break;
        case 2:
            {
                sal_Char* value = (sal_Char*)rtl_allocateMemory(valueSize);
                readUtf8(pBuffer, value, valueSize);

                if (bLinkValue)
                {
                    fprintf(stdout, "%sKEY: Type = RG_LINKTYPE\n", indent);
                    fprintf(stdout, "%s     LinkTarget = \"%s\"\n", indent, value);
                } else
                {
                    fprintf(stdout, "%sValue: Type = RG_VALUETYPE_STRING\n", indent);
                    fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                    fprintf(stdout, "%s       Data = \"%s\"\n", indent, value);
                }

                rtl_freeMemory(value);
            }
            break;
        case 3:
            {
                sal_uInt32 size = (valueSize / 2) * sizeof(sal_Unicode);
                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_UNICODE\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Data = ", indent);

                sal_Unicode* value = new sal_Unicode[size];
                readString(pBuffer, value, size);

                OString uStr = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
                fprintf(stdout, "L\"%s\"\n", uStr.getStr());
                delete[] value;
            }
            break;
        case 4:
            {
                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_BINARY\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Data = ", indent);

                RegistryTypeReader_Api* pReaderApi;

                pReaderApi = initRegistryTypeReader_Api();

                RegistryTypeReader reader(pReaderApi, pBuffer, valueSize, sal_False);

                sIndent += "              ";
                if (!dumpType(reader, sIndent))
                {
                    fprintf(stdout, "<unknown>\n");
                }

            }
            break;
        case 5:
            {
                sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays
                sal_uInt32 len = 0;

                readUINT32(pBuffer, len);

                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_LONGLIST\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Len  = %d\n", indent, len);
                fprintf(stdout, "%s       Data = ", indent);

                sal_Int32 longValue;
                for (sal_uInt32 i=0; i < len; i++)
                {
                    readINT32(pBuffer+offset, longValue);

                    if (offset > 4)
                        fprintf(stdout, "%s              ", indent);

                    fprintf(stdout, "%d = %d\n", i, longValue);
                    offset += 4; // 4 Bytes fuer sal_Int32
                }
            }
            break;
        case 6:
            {
                sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays
                sal_uInt32 sLen = 0;
                sal_uInt32 len = 0;

                readUINT32(pBuffer, len);

                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_STRINGLIST\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Len  = %d\n", indent, len);
                fprintf(stdout, "%s       Data = ", indent);

                sal_Char *pValue;
                for (sal_uInt32 i=0; i < len; i++)
                {
                    readUINT32(pBuffer+offset, sLen);

                    offset += 4; // 4 Bytes (sal_uInt32) fuer die Groesse des strings in Bytes

                    pValue = (sal_Char*)rtl_allocateMemory(sLen);
                    readUtf8(pBuffer+offset, pValue, sLen);

                    if (offset > 8)
                        fprintf(stdout, "%s              ", indent);

                    fprintf(stdout, "%d = \"%s\"\n", i, pValue);
                    offset += sLen;
                }
            }
            break;
        case 7:
            {
                sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays
                sal_uInt32 sLen = 0;
                sal_uInt32 len = 0;

                readUINT32(pBuffer, len);

                fprintf(stdout, "%sValue: Type = RG_VALUETYPE_UNICODELIST\n", indent);
                fprintf(stdout, "%s       Size = %d\n", indent, valueSize);
                fprintf(stdout, "%s       Len  = %d\n", indent, len);
                fprintf(stdout, "%s       Data = ", indent);

                sal_Unicode *pValue;
                OString uStr;
                for (sal_uInt32 i=0; i < len; i++)
                {
                    readUINT32(pBuffer+offset, sLen);

                    offset += 4; // 4 Bytes (sal_uInt32) fuer die Groesse des strings in Bytes

                    pValue = (sal_Unicode*)rtl_allocateMemory((sLen / 2) * sizeof(sal_Unicode));
                    readString(pBuffer+offset, pValue, sLen);

                    if (offset > 8)
                        fprintf(stdout, "%s              ", indent);

                    uStr = OUStringToOString(pValue, RTL_TEXTENCODING_UTF8);
                    fprintf(stdout, "%d = L\"%s\"\n", i, uStr.getStr());

                    offset += sLen;

                    rtl_freeMemory(pValue);
                }
            }
            break;
    }

    fprintf(stdout, "\n");

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  dumpKey()
//
RegError ORegistry::dumpKey(const OUString& sPath, const OUString& sName, sal_Int16 nSpace) const
{
    OStoreDirectory     rStoreDir;
    OUString            sFullPath(sPath);
    OString             sIndent;
    storeAccessMode     accessMode = KEY_MODE_OPEN;
    RegError            _ret = REG_NO_ERROR;

    if (isReadOnly())
    {
        accessMode = KEY_MODE_OPENREAD;
    }

    for (int i= 0; i < nSpace; i++) sIndent += " ";

    if (sFullPath.getLength() > 1)
        sFullPath += ROOT;

    storeError _err = rStoreDir.create(m_file, sFullPath, sName, accessMode);

    if (_err == store_E_NotExists)
        return REG_KEY_NOT_EXISTS;
    else
    if (_err == store_E_WrongFormat)
        return REG_INVALID_KEY;

    fprintf(stdout, "%s/ %s\n", sIndent.getStr(), OUStringToOString(sName, RTL_TEXTENCODING_UTF8).getStr());

    OUString sSubPath(sFullPath);
    OUString sSubName;
    sSubPath += sName;

    OStoreDirectory::iterator   iter;

    _err = rStoreDir.first(iter);

    while ( _err == store_E_None)
    {
        sSubName = iter.m_pszName;

        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            _ret = dumpKey(sSubPath, sSubName, nSpace+2);
        } else
        {
            _ret = dumpValue(sSubPath, sSubName, nSpace+2);
        }

        if (_ret)
        {
            return _ret;
        }

        _err = rStoreDir.next(iter);
    }

    return REG_NO_ERROR;
}

//*********************************************************************
//  createLink()
//
RegError ORegistry::createLink(RegKeyHandle hKey,
                               const OUString& linkName,
                               const OUString& linkTarget)
{
    ORegKey*    pKey;

    if ( !linkName.getLength() )
    {
        return REG_INVALID_LINKNAME;
    }

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

    OUString sFullLinkName = resolveLinks(pKey, linkName, sal_True);

    if (sFullLinkName.getLength() == 0)
        return REG_DETECT_RECURSION;

    OStoreDirectory rStoreDir;
    OUString        sFullPath(ROOT);

    sal_uInt32  tokenCount = sFullLinkName.getTokenCount('/');
    sal_uInt32  actToken = 0;
    OUString    token;

    token = sFullLinkName.getToken(actToken, '/');

    while ( actToken < tokenCount && token.getLength() > 0 )
    {
        if (rStoreDir.create(pKey->getStoreFile(), sFullPath, token, KEY_MODE_CREATE))
        {
            return REG_CREATE_KEY_FAILED;
        }

        sFullPath += token;
        sFullPath += ROOT;

        token = sFullLinkName.getToken(++actToken, '/');
    }

    pKey = new ORegKey(sFullLinkName, linkTarget, rStoreDir, this);
    delete pKey;

    return REG_NO_ERROR;
}

//*********************************************************************
//  deleteLink()
//
RegError ORegistry::deleteLink(RegKeyHandle hKey, const OUString& linkName)
{
    ORegKey*    pKey;

    if ( !linkName.getLength() )
    {
        return REG_INVALID_LINKNAME;
    }

    REG_GUARD(m_mutex);

    if (hKey)
        pKey = (ORegKey*)hKey;
    else
        pKey = m_openKeyTable[ROOT];

    OUString    tmpPath(linkName);
    OUString    tmpName;
    OUString    resolvedPath;
    sal_Int32   lastIndex = tmpPath.lastIndexOf('/');

    if ( lastIndex > 0 && tmpPath.getStr()[0] == '/')
    {
        tmpName = tmpPath.copy(lastIndex);

        OUString linkPath = tmpPath.copy(0, lastIndex);

        resolvedPath = resolveLinks(pKey, linkPath);

        if ( !resolvedPath.getLength() )
        {
            return REG_DETECT_RECURSION;
        }

        resolvedPath += tmpName;
    } else
    {
        resolvedPath = pKey->getName();

        if (lastIndex != 0 && resolvedPath.getLength() > 1)
            resolvedPath += ROOT;

        resolvedPath += linkName;
    }

    pKey = m_openKeyTable[ROOT];

    RegKeyType  keyType;
    RegError    ret = REG_NO_ERROR;
    if (ret = pKey->getKeyType(resolvedPath, &keyType))
        return ret;

    if (keyType != RG_LINKTYPE)
        return REG_INVALID_LINK;

    return eraseKey(pKey, resolvedPath, RESOLVE_PART);
}

//*********************************************************************
//  resolveLinks()
//
OUString ORegistry::resolveLinks(ORegKey* pKey, const OUString& path, sal_Bool firstLinkOnly)
{
    OUString    resolvedPath(pKey->getName());
    sal_uInt32  tokenCount = path.getTokenCount('/');
    sal_uInt32  actToken = 0;
    OUString    token;
    ORegKey*    pLink = NULL;

    if (resolvedPath.getLength() > 1)
        resolvedPath += ROOT;

    if ( path.getStr()[0] == '/' )
        token = path.getToken(++actToken, '/');
    else
        token = path.getToken(actToken, '/');

    while ( actToken < tokenCount && token.getLength() > 0 )
    {
        pLink = resolveLink(pKey, resolvedPath, token);

        if (pLink)
        {
            OUString    tmpName;
            sal_Int32   lastIndex;

            while(pLink)
            {
                if (!insertRecursionLink(pLink))
                {
                    resetRecursionLinks();
                    delete pLink;
                    return OUString();
                }


                lastIndex = resolvedPath.lastIndexOf('/');
                tmpName = resolvedPath.copy(lastIndex + 1);
                resolvedPath = resolvedPath.copy(0, lastIndex + 1);

                pLink = resolveLink(pKey, resolvedPath, tmpName);
            }

            resetRecursionLinks();
        }

        token = path.getToken(++actToken, '/');
        if ( token.getLength() )
            resolvedPath += ROOT;
    }

    return resolvedPath;
}

//*********************************************************************
//  resolveLink()
//
ORegKey* ORegistry::resolveLink(ORegKey* pKey, OUString& resolvedPath, const OUString& name)
{
    OStoreDirectory rStoreDir;
    ORegKey*        pTmpKey = NULL;

    if ( !rStoreDir.create(pKey->getStoreFile(), resolvedPath,
                          name, KEY_MODE_OPENREAD) )
    {
        resolvedPath += name;
        pTmpKey = new ORegKey(resolvedPath, rStoreDir, pKey->getRegistry());
        RegKeyType  keyType;
        sal_Bool    bIsLink=sal_False;
        if (!pTmpKey->getKeyType(OUString(), &keyType) && (keyType == RG_LINKTYPE))
        {
            resolvedPath = pTmpKey->getLinkTarget();
            return pTmpKey;
        }

        delete pTmpKey;
        return NULL;
    } else
    {
        resolvedPath += name;
        return NULL;
    }
}

sal_Bool ORegistry::insertRecursionLink(ORegKey* pLink)
{
    if (m_recursionList.empty())
    {
        m_recursionList.push_back(pLink);
    } else
    {
        LinkList::iterator iter = m_recursionList.begin();

        while (iter != m_recursionList.end())
        {
            if ((*iter)->getName() == pLink->getName())
                return sal_False;

            iter++;
        }
        m_recursionList.push_back(pLink);
    }

    return sal_True;
}

sal_Bool ORegistry::resetRecursionLinks()
{
    LinkList::iterator iter = m_recursionList.begin();

    while (iter != m_recursionList.end())
    {
        delete *iter;
        iter++;
    }

    m_recursionList.erase(m_recursionList.begin(), m_recursionList.end());

    return sal_True;
}


