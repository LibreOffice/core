/*************************************************************************
 *
 *  $RCSfile: astinterface.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 11:58:23 $
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
#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif
#ifndef _IDLC_ASTATTRIBUTE_HXX_
#include <idlc/astattribute.hxx>
#endif
#ifndef _IDLC_ASTOPERATION_HXX_
#include <idlc/astoperation.hxx>
#endif

using namespace ::rtl;

AstInterface::AstInterface(const ::rtl::OString& name,
                           AstDeclaration* pInherits,
                           AstScope* pScope)
    : AstType(NT_interface, name, pScope)
    , AstScope(NT_interface)
    , m_bIsDefined(false)
    , m_bForwarded(sal_False)
    , m_bForwardedInSameFile(sal_False)
    , m_bSingleInheritance(pInherits != 0)
{
    if ( pInherits )
        m_inheritedInterfaces.push_back(pInherits);
}

AstInterface::~AstInterface()
{
}

bool AstInterface::addInheritedInterface(AstInterface * pInherits)
{
    for (DeclList::iterator i(m_inheritedInterfaces.begin());
         i != m_inheritedInterfaces.end(); ++i)
    {
        if ((*i)->getScopedName() == pInherits->getScopedName()) {
            return false;
        }
    }
    m_inheritedInterfaces.push_back(pInherits);
    return true;
}

void AstInterface::forwardDefined(AstInterface const & def)
{
    setImported(def.isImported());
    setInMainfile(def.isInMainfile());
    setLineNumber(def.getLineNumber());
    setFileName(def.getFileName());
    setDocumentation(def.getDocumentation());
    m_inheritedInterfaces = def.m_inheritedInterfaces;
    m_bIsDefined = true;
}

sal_Bool AstInterface::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
{
    if ( !isDefined() )
        return sal_True;

    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    if (m_inheritedInterfaces.size() > SAL_MAX_UINT16) {
        //TODO
    }
    sal_uInt16 nBaseTypes = static_cast< sal_uInt16 >(
        m_inheritedInterfaces.size());
    sal_uInt16 nAttributes = getNodeCount(NT_attribute);
    sal_uInt16 nMethods = getNodeCount(NT_operation);

    RegistryTypeWriter aBlob(pLoader->getApi(), RT_TYPE_INTERFACE,
                             OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                             nBaseTypes, nAttributes, nMethods, 0);

    aBlob.setDoku( getDocumentation() );
    aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

    RTUik aUik;
    aUik.m_Data1 = 0;
    aUik.m_Data2 = 0;
    aUik.m_Data3 = 0;
    aUik.m_Data4 = 0;
    aUik.m_Data5 = 0;
    aBlob.setUik(aUik);

    sal_uInt16 superTypeIndex = 0;
    for (DeclList::iterator i = m_inheritedInterfaces.begin();
         i != m_inheritedInterfaces.end(); ++i)
    {
        aBlob.setMISuperTypeData(
            superTypeIndex++,
            OStringToOUString((*i)->getRelativName(), RTL_TEXTENCODING_UTF8));
    }

    DeclList::iterator iter = getIteratorBegin();
    DeclList::iterator end = getIteratorEnd();
    AstDeclaration* pDecl = NULL;
    sal_uInt16  attrIndex = 0;
    sal_uInt16  methodIndex = 0;
    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getNodeType() == NT_attribute )
        {
            ((AstAttribute*)pDecl)->dumpBlob(aBlob, attrIndex++);
        } else
        if ( pDecl->getNodeType() == NT_operation )
        {
            ((AstOperation*)pDecl)->dumpBlob(aBlob, methodIndex++);
        }
        ++iter;
    }

    const sal_uInt8*    pBlob = aBlob.getBlop();
    sal_uInt32          aBlobSize = aBlob.getBlopSize();

    if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY, (RegValue)pBlob, aBlobSize))
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    return AstDeclaration::dump(rKey, pLoader);
}
