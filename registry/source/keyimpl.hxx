/*************************************************************************
 *
 *  $RCSfile: keyimpl.hxx,v $
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

#ifndef _KEYIMPL_HXX_
#define _KEYIMPL_HXX_

#ifndef _REGISTRY_REGISTRY_H_
#include    <registry/registry.h>
#endif

#ifndef _REGIMPL_HXX_
#include    "regimpl.hxx"
#endif

#ifndef _RTL_STRING_HXX_
#include    <rtl/string.hxx>
#endif

using namespace rtl;
using namespace store;

class ORegKey
{
public:

    ORegKey(const OUString& keyName, const OStoreDirectory& rStoreDir, ORegistry* pReg);
    ORegKey(const OUString& keyName, const OUString& linkName,
            const OStoreDirectory& rStoreDir, ORegistry* pReg);

      sal_uInt32    acquire()
        { return ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    createKey(const OUString& keyName, RegKeyHandle* phNewKey);

    RegError    openKey(const OUString& keyName, RegKeyHandle* phOpenKey,
                        RESOLVE eResolve=RESOLVE_FULL);

    RegError    openSubKeys(const OUString& keyName,
                            RegKeyHandle** phOpenSubKeys,
                            sal_uInt32* pnSubKeys);

    RegError    getKeyNames(const OUString& keyName,
                            rtl_uString*** pSubKeyNames,
                            sal_uInt32* pnSubKeys);

    RegError    closeSubKeys(RegKeyHandle* phSubKeys, sal_uInt32 len);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(const OUString& keyName);

    RegError    getValueInfo(const OUString& valueName,
                             RegValueType* pValueTye,
                             sal_uInt32* pValueSize) const;

    RegError    setValue(const OUString& valueName,
                         RegValueType vType,
                         RegValue value,
                         sal_uInt32 vSize);

    RegError    setLongListValue(const OUString& valueName,
                                  sal_Int32* pValueList,
                                 sal_uInt32 len);

    RegError    setStringListValue(const OUString& valueName,
                                   sal_Char** pValueList,
                                  sal_uInt32 len);

    RegError    setUnicodeListValue(const OUString& valueName,
                                   sal_Unicode** pValueList,
                                  sal_uInt32 len);

    RegError    getValue(const OUString& valueName, RegValue value) const;

    RegError    getLongListValue(const OUString& valueName,
                                  sal_Int32** pValueList,
                                 sal_uInt32* pLen) const;

    RegError    getStringListValue(const OUString& valueName,
                                    sal_Char*** pValueList,
                                   sal_uInt32* pLen) const;

    RegError    getUnicodeListValue(const OUString& valueName,
                                     sal_Unicode*** pValueList,
                                    sal_uInt32* pLen) const;

    RegError    createLink(const OUString& linkName, const OUString& linkTarget);

    RegError    deleteLink(const OUString& linkName);

    RegError    getKeyType(const OUString& name,
                           RegKeyType* pKeyType) const;

    RegError    getLinkTarget(const OUString& linkName,
                              OUString& pLinkTarget) const;

    RegError    getResolvedKeyName(const OUString& keyName,
                                   OUString& resolvedName,
                                   sal_Bool firstLinkOnly=sal_False);

public:
    ORegKey();
    virtual ~ORegKey();


    sal_Bool        isDeleted() const
                    { return m_bDeleted; }

    void        setDeleted(sal_Bool bKeyDeleted)
                    { m_bDeleted = bKeyDeleted; }

    sal_Bool        isReadOnly() const
                    { return m_pRegistry->isReadOnly(); }

    sal_uInt32      countSubKeys();

    sal_Bool        isLink() const
                    { return m_isLink; }

    const OUString& getLinkTarget() const
                    { return m_link; }

    ORegistry* getRegistry() const
                    { return m_pRegistry; }

    const OStoreFile& getStoreFile() const
                    { return m_storeFile; }

    OStoreDirectory getStoreDir();

    const OUString& getName() const
                    { return m_name; }

    sal_uInt32 getRefCount() const
                    { return m_refCount; }

    friend class ORegistry;
protected:
    sal_Bool                checkLink();

    sal_uInt32              m_refCount;
    OUString                m_name;
    OUString                m_link;
    sal_Bool                m_bDeleted;
    sal_Bool                m_isLink;
    ORegistry*              m_pRegistry;
    OStoreFile              m_storeFile;
};



#endif


