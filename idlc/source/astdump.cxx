/*************************************************************************
 *
 *  $RCSfile: astdump.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:42:47 $
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

#ifndef _IDLC_ASTMODULE_HXX_
#include <idlc/astmodule.hxx>
#endif
#ifndef _IDLC_ASTTYPEDEF_HXX_
#include <idlc/asttypedef.hxx>
#endif
#ifndef _IDLC_ASTSERVICE_HXX_
#include <idlc/astservice.hxx>
#endif
#ifndef _IDLC_ASTCONSTANT_HXX_
#include <idlc/astconstant.hxx>
#endif
#ifndef _IDLC_ASTATTRIBUTE_HXX_
#include <idlc/astattribute.hxx>
#endif
#ifndef _IDLC_ASTINTERFACEMEMBER_HXX_
#include <idlc/astinterfacemember.hxx>
#endif
#ifndef _IDLC_ASTSERVICEEMEMBER_HXX_
#include <idlc/astservicemember.hxx>
#endif
#ifndef _IDLC_ASTOBSERVES_HXX_
#include <idlc/astobserves.hxx>
#endif
#ifndef _IDLC_ASTNEEDS_HXX_
#include <idlc/astneeds.hxx>
#endif
#ifndef _IDLC_ASTSEQUENCE_HXX_
#include <idlc/astsequence.hxx>
#endif
#include "idlc/astoperation.hxx"

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

sal_Bool AstModule::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if ( getNodeType() == NT_root )
    {
        localKey = rKey;
    }else
    {
        if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
        {
            fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return sal_False;
        }
    }

    sal_uInt16          nConst = getNodeCount(NT_const);

    if ( nConst > 0 )
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(),
            OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8), typeClass,
            m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0,
            nConst, 0, 0);

        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_const &&
                 pDecl->isInMainfile() )
            {
                ((AstConstant*)pDecl)->dumpBlob(
                    aBlob, index++,
                    getNodeType() == NT_module && pDecl->isPublished());
            }
            ++iter;
        }

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return sal_False;
        }
    } else
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        rtl::OUString fileName;
        if (getNodeType() == NT_constants) {
            fileName = OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8);
        }
        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), fileName, typeClass, m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0, 0, 0,
            0);

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if ( getNodeType() != NT_root )
        {
            if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                    (RegValue)pBlob, aBlobSize))
            {
                fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                        idlc()->getOptions()->getProgramName().getStr(),
                        getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
                return sal_False;
            }
        }
    }
    return AstDeclaration::dump(rKey);
}

sal_Bool AstTypeDef::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    typereg::Writer aBlob(
        m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
        getDocumentation(),
        OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8),
        RT_TYPE_TYPEDEF, m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 1, 0, 0, 0);
    aBlob.setSuperTypeName(
        0,
        OStringToOUString(
            getBaseType()->getRelativName(), RTL_TEXTENCODING_UTF8));

    sal_uInt32 aBlobSize;
    void const * pBlob = aBlob.getBlob(&aBlobSize);

    if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY, (RegValue)pBlob, aBlobSize))
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    return sal_True;
}

sal_Bool AstService::dump(RegistryKey& rKey)
{
    typereg_Version version = m_bPublished
        ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0;
    rtl::OString superName;
    sal_uInt16 constructors = 0;
    sal_uInt16 properties = 0;
    sal_uInt16 references = 0;
    {for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
          ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_interface:
        case NT_typedef:
            version = TYPEREG_VERSION_1;
            OSL_ASSERT(superName.getLength() == 0);
            superName = (*i)->getRelativName();
            break;

        case NT_operation:
            OSL_ASSERT(getNodeType() == NT_service);
            ++constructors;
            break;

        case NT_property:
            OSL_ASSERT(getNodeType() == NT_service);
            ++properties;
            break;

        case NT_service_member:
            if (getNodeType() == NT_singleton) {
                OSL_ASSERT(superName.getLength() == 0);
                if (!idlc()->getOptions()->isValid("-C")) {
                    return true;
                }
                superName = static_cast< AstServiceMember * >(*i)->
                    getRealService()->getRelativName();
                break;
            }
        case NT_interface_member:
        case NT_observes:
        case NT_needs:
            OSL_ASSERT(getNodeType() == NT_service);
            ++references;
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }}
    OSL_ASSERT(constructors == 0 || !m_defaultConstructor);
    if (m_defaultConstructor) {
        constructors = 1;
    }
    RegistryKey localKey;
    if (rKey.createKey(
            rtl::OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8),
            localKey)) {
        fprintf(
            stderr, "%s: warning, could not create key '%s' in '%s'\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getFullName().getStr(),
            rtl::OUStringToOString(
                rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    typereg::Writer writer(
        version, getDocumentation(),
        rtl::OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8),
        getNodeType() == NT_singleton ? RT_TYPE_SINGLETON : RT_TYPE_SERVICE,
        m_bPublished,
        rtl::OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
        superName.getLength() == 0 ? 0 : 1, properties, constructors,
        references);
    if (superName.getLength() != 0) {
        writer.setSuperTypeName(
            0, rtl::OStringToOUString(superName, RTL_TEXTENCODING_UTF8));
    }
    sal_uInt16 constructorIndex = 0;
    sal_uInt16 propertyIndex = 0;
    sal_uInt16 referenceIndex = 0;
    {for (DeclList::const_iterator i(getIteratorBegin()); i != getIteratorEnd();
          ++i)
    {
        switch ((*i)->getNodeType()) {
        case NT_operation:
            static_cast< AstOperation * >(*i)->dumpBlob(
                writer, constructorIndex++);
            break;

        case NT_property:
            static_cast< AstAttribute * >(*i)->dumpBlob(
                writer, propertyIndex++, 0);
            break;

        case NT_interface_member:
            {
                AstInterfaceMember * decl = static_cast< AstInterfaceMember *>(
                    *i);
                writer.setReferenceData(
                    referenceIndex++, decl->getDocumentation(), RT_REF_SUPPORTS,
                    (decl->isOptional()
                     ? RT_ACCESS_OPTIONAL : RT_ACCESS_INVALID),
                    rtl::OStringToOUString(
                        decl->getRealInterface()->getRelativName(),
                        RTL_TEXTENCODING_UTF8));
                break;
            }

        case NT_service_member:
            if (getNodeType() == NT_service) {
                AstServiceMember * decl = static_cast< AstServiceMember * >(*i);
                writer.setReferenceData(
                    referenceIndex++, decl->getDocumentation(), RT_REF_EXPORTS,
                    (decl->isOptional()
                     ? RT_ACCESS_OPTIONAL : RT_ACCESS_INVALID),
                    rtl::OStringToOUString(
                        decl->getRealService()->getRelativName(),
                        RTL_TEXTENCODING_UTF8));
            }
            break;

        case NT_observes:
            {
                AstObserves * decl = static_cast< AstObserves * >(*i);
                writer.setReferenceData(
                    referenceIndex++, decl->getDocumentation(), RT_REF_OBSERVES,
                    RT_ACCESS_INVALID,
                    rtl::OStringToOUString(
                        decl->getRealInterface()->getRelativName(),
                        RTL_TEXTENCODING_UTF8));
                break;
            }

        case NT_needs:
            {
                AstNeeds * decl = static_cast< AstNeeds * >(*i);
                writer.setReferenceData(
                    referenceIndex++, decl->getDocumentation(), RT_REF_NEEDS,
                    RT_ACCESS_INVALID,
                    rtl::OStringToOUString(
                        decl->getRealService()->getRelativName(),
                        RTL_TEXTENCODING_UTF8));
                break;
            }

        default:
            OSL_ASSERT(
                (*i)->getNodeType() == NT_interface
                || (*i)->getNodeType() == NT_typedef);
            break;
        }
    }}
    if (m_defaultConstructor) {
        writer.setMethodData(
            constructorIndex++, rtl::OUString(), RT_MODE_TWOWAY,
            rtl::OUString(), rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")),
            0, 0);
    }
    sal_uInt32 size;
    void const * blob = writer.getBlob(&size);
    if (localKey.setValue(
            rtl::OUString(), RG_VALUETYPE_BINARY, const_cast< void * >(blob),
            size))
    {
        fprintf(
            stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
            idlc()->getOptions()->getProgramName().getStr(),
            getFullName().getStr(),
            rtl::OUStringToOString(
                localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    return true;
}

sal_Bool AstAttribute::dumpBlob(
    typereg::Writer & rBlob, sal_uInt16 index, sal_uInt16 * methodIndex)
{
    RTFieldAccess accessMode = RT_ACCESS_INVALID;

    if (isReadonly())
    {
        accessMode |= RT_ACCESS_READONLY;
    } else
    {
        accessMode |= RT_ACCESS_READWRITE;
    }
    if (isOptional())
    {
        accessMode |= RT_ACCESS_OPTIONAL;
    }
    if (isBound())
    {
        accessMode |= RT_ACCESS_BOUND;
    }
    if (isMayBeVoid())
    {
        accessMode |= RT_ACCESS_MAYBEVOID;
    }
    if (isConstrained())
    {
        accessMode |= RT_ACCESS_CONSTRAINED;
    }
    if (isTransient())
    {
        accessMode |= RT_ACCESS_TRANSIENT;
    }
    if (isMayBeAmbiguous())
    {
        accessMode |= RT_ACCESS_MAYBEAMBIGUOUS;
    }
    if (isMayBeDefault())
    {
        accessMode |= RT_ACCESS_MAYBEDEFAULT;
    }
    if (isRemoveable())
    {
        accessMode |= RT_ACCESS_REMOVEABLE;
    }

    rtl::OUString name(
        OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8));
    rBlob.setFieldData(
        index, getDocumentation(), OUString(), accessMode, name,
        OStringToOUString(getType()->getRelativName(), RTL_TEXTENCODING_UTF8),
        RTConstValue());
    dumpExceptions(
        rBlob, m_getDocumentation, m_getExceptions, RT_MODE_ATTRIBUTE_GET,
        methodIndex);
    dumpExceptions(
        rBlob, m_setDocumentation, m_setExceptions, RT_MODE_ATTRIBUTE_SET,
        methodIndex);

    return sal_True;
}

void AstAttribute::dumpExceptions(
    typereg::Writer & writer, rtl::OUString const & documentation,
    DeclList const & exceptions, RTMethodMode flags, sal_uInt16 * methodIndex)
{
    if (!exceptions.empty()) {
        OSL_ASSERT(methodIndex != 0);
        sal_uInt16 idx = (*methodIndex)++;
        // exceptions.size() <= SAL_MAX_UINT16 already checked in
        // AstInterface::dump:
        writer.setMethodData(
            idx, documentation, flags,
            OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")), 0,
            static_cast< sal_uInt16 >(exceptions.size()));
        sal_uInt16 exceptionIndex = 0;
        for (DeclList::const_iterator i(exceptions.begin());
             i != exceptions.end(); ++i)
        {
            writer.setMethodExceptionTypeName(
                idx, exceptionIndex++,
                rtl::OStringToOUString(
                    (*i)->getRelativName(), RTL_TEXTENCODING_UTF8));
        }
    }
}

const sal_Char* AstSequence::getRelativName() const
{
    if ( !m_pRelativName )
    {
        m_pRelativName = new OString("[]");
        AstDeclaration const * pType = resolveTypedefs( m_pMemberType );
        *m_pRelativName += pType->getRelativName();
    }

    return m_pRelativName->getStr();
}
