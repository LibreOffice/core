/*************************************************************************
 *
 *  $RCSfile: regimpl.hxx,v $
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

#ifndef _REGIMPL_HXX_
#define _REGIMPL_HXX_

#include    <registry/registry.h>

#ifndef _REGMAP_HXX_
#include    "regmap.hxx"
#endif

#ifndef _OSL_MUTEX_HXX_
#include    <osl/mutex.hxx>
#endif

#ifndef _STORE_STORE_HXX_
#include    <store/store.hxx>
#endif

#define REG_PAGESIZE 512

#define REG_MODE_CREATE     store_AccessCreate
#define REG_MODE_OPEN       store_AccessReadWrite
#define REG_MODE_OPENREAD   store_AccessReadOnly

#define KEY_MODE_CREATE     store_AccessCreate
#define KEY_MODE_OPEN       store_AccessReadWrite
#define KEY_MODE_OPENREAD   store_AccessReadOnly


#define VALUE_MODE_CREATE   store_AccessCreate
#define VALUE_MODE_OPEN     store_AccessReadWrite
#define VALUE_MODE_OPENREAD store_AccessReadOnly

#define VALUE_PREFIX        "$VL_"
#define VALUE_LINKTARGET    "$VL_LINK_TARGET"

// 5 Bytes = 1 (Byte fuer den Typ) + 4 (Bytes fuer die Groesse der Daten)
#define VALUE_HEADERSIZE    5
#define VALUE_TYPEOFFSET    1
#define VALUE_HEADEROFFSET  5

#define REG_CREATE      0x0004  // allow write accesses

#define REG_GUARD(mutex) \
    Guard< Mutex > aGuard( mutex );

using namespace rtl;
using namespace osl;
using namespace store;

enum RESOLVE
{
    // alle Links werden aufgeloest
    RESOLVE_FULL,
    // alles bis zum letzten TeilStueck wird aufgeloest
    RESOLVE_PART,
    // kein Link wird aufgeloest
    RESOLVE_NOTHING
};

class ORegManager;
class RegistryTypeReader;

class ORegistry
{
public:
    ORegistry();

    sal_uInt32  acquire()
        { return ++m_refCount; }

    sal_uInt32  release()
        { return --m_refCount; }

    RegError    initRegistry(const OUString& name,
                             RegAccessMode accessMode);

    RegError    closeRegistry();

    RegError    destroyRegistry(const OUString& name);

    RegError    createKey(RegKeyHandle hKey,
                          const OUString& keyName,
                          RegKeyHandle* phNewKey);

    RegError    openKey(RegKeyHandle hKey,
                        const OUString& keyName,
                        RegKeyHandle* phOpenKey,
                        RESOLVE eResolve=RESOLVE_FULL);

    RegError    closeKey(RegKeyHandle hKey);

    RegError    deleteKey(RegKeyHandle hKey, const OUString& keyName);

    RegError    loadKey(RegKeyHandle hKey,
                        const OUString& regFileName,
                        sal_Bool bWarings=sal_False,
                        sal_Bool bReport=sal_False);

    RegError    saveKey(RegKeyHandle hKey,
                        const OUString& regFileName,
                        sal_Bool bWarings=sal_False,
                        sal_Bool bReport=sal_False);

    RegError    dumpRegistry(RegKeyHandle hKey) const;

    RegError    createLink(RegKeyHandle hKey,
                           const OUString& linkName,
                           const OUString& linkTarget);

       RegError     deleteLink(RegKeyHandle hKey,
                           const OUString& linkName);

public:
    virtual ~ORegistry();

    sal_Bool            isKeyOpen(const OUString&   keyName) const;
    sal_Bool            isReadOnly() const
        { return m_readOnly; }

    sal_Bool            isOpen() const
        { return m_isOpen; }

    ORegKey*    getRootKey();

    const OStoreFile&   getStoreFile()
        { return m_file; }

    const OUString&     getName() const
        { return m_name; }

    RegError getResolvedKeyName(RegKeyHandle hKey,
                                 const OUString& keyName,
                                 OUString& resolvedName,
                                 sal_Bool firstLinkOnly=sal_False);

    friend class ORegKey;
protected:
    RegError    eraseKey(ORegKey* pKey,
                         const OUString& keyName,
                         RESOLVE eResolve=RESOLVE_FULL);

    RegError    deleteSubkeysAndValues(ORegKey* pKey,
                                       RESOLVE eResolve=RESOLVE_FULL);

    sal_uInt32      countSubKeys(ORegKey* pKey);

    RegError    loadAndSaveValue(ORegKey* pTargetKey,
                                 ORegKey* pSourceKey,
                                 const OUString& valueName,
                                 sal_uInt32 nCut,
                                 sal_Bool bWarnings=sal_False,
                                 sal_Bool bReport=sal_False);

    RegError    checkBlop(OStoreStream& rValue,
                          ORegKey* pTargetKey,
                          const OUString& sTargetPath,
                          const OUString& valueName,
                          sal_uInt32 srcValueSize,
                          sal_uInt8* pSrcBuffer,
                          sal_Bool bReport=sal_False);

    RegError    mergeModuleValue(OStoreStream& rTargetValue,
                                 ORegKey* pTargetKey,
                                 const OUString& sTargetPath,
                                 const OUString& valueName,
                                 RegistryTypeReader& reader,
                                 RegistryTypeReader& reader2);

    RegError    loadAndSaveKeys(ORegKey* pTargetKey,
                                ORegKey* pSourceKey,
                                const OUString& keyName,
                                sal_uInt32 nCut,
                                sal_Bool bWarnings=sal_False,
                                sal_Bool bReport=sal_False);

    RegError    dumpValue(const OUString& sPath,
                          const OUString& sName,
                          sal_Int16 nSpace) const;

    RegError    dumpKey(const OUString& sPath,
                        const OUString& sName,
                        sal_Int16 nSpace) const;

    OUString resolveLinks(ORegKey* pKey, const OUString& path, sal_Bool firstLinkOnly=sal_False);
    ORegKey* resolveLink(ORegKey* pKey, OUString& resolvedPath, const OUString& name);

    sal_Bool insertRecursionLink(ORegKey* pLink);
    sal_Bool resetRecursionLinks();

protected:

    sal_uInt32      m_refCount;
    Mutex           m_mutex;
    sal_Bool        m_readOnly;
    sal_Bool        m_isOpen;
    OUString        m_name;
    OStoreFile      m_file;
    KeyMap          m_openKeyTable;
    LinkList        m_recursionList;

    const OUString  ROOT;
};

#endif

