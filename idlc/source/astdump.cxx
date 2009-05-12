/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astdump.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
#include <idlc/astmodule.hxx>
#include <idlc/asttypedef.hxx>
#include <idlc/astservice.hxx>
#include <idlc/astconstant.hxx>
#include <idlc/astattribute.hxx>
#include <idlc/astinterfacemember.hxx>
#ifndef _IDLC_ASTSERVICEEMEMBER_HXX_
#include <idlc/astservicemember.hxx>
#endif
#include <idlc/astobserves.hxx>
#include <idlc/astneeds.hxx>
#include <idlc/astsequence.hxx>
#include "idlc/astoperation.hxx"

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

sal_Bool AstModule::dump(RegistryKey& rKey)
{
    OUString emptyStr;
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
            getDocumentation(), emptyStr, typeClass,
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

        if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
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

        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), emptyStr, typeClass, m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0, 0, 0,
            0);

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if ( getNodeType() != NT_root )
        {
            if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
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
    OUString emptyStr;
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
        getDocumentation(), emptyStr, RT_TYPE_TYPEDEF, m_bPublished,
        OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 1, 0, 0, 0);
    aBlob.setSuperTypeName(
        0,
        OStringToOUString(
            getBaseType()->getRelativName(), RTL_TEXTENCODING_UTF8));

    sal_uInt32 aBlobSize;
    void const * pBlob = aBlob.getBlob(&aBlobSize);

    if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY, (RegValue)pBlob, aBlobSize))
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
    OUString emptyStr;
    typereg_Version version = m_bPublished
        ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0;
    OString superName;
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
                superName = ((AstServiceMember *)(*i))->
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
        version, getDocumentation(), emptyStr,
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
//           static_cast< AstOperation * >(*i)->dumpBlob(
            ((AstOperation *)(*i))->dumpBlob(
                writer, constructorIndex++);
            break;

        case NT_property:
//            static_cast< AstAttribute * >(*i)->dumpBlob(
            ((AstAttribute *)(*i))->dumpBlob(
                writer, propertyIndex++, 0);
            break;

        case NT_interface_member:
            {
//               AstInterfaceMember * decl = static_cast< AstInterfaceMember *>(*i);
                AstInterfaceMember * decl = (AstInterfaceMember *)(*i);
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
//              AstServiceMember * decl = static_cast< AstServiceMember * >(*i);
                AstServiceMember * decl = (AstServiceMember *)(*i);
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
//              AstObserves * decl = static_cast< AstObserves * >(*i);
                AstObserves * decl = (AstObserves *)(*i);
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
//              AstNeeds * decl = static_cast< AstNeeds * >(*i);
                AstNeeds * decl = (AstNeeds *)(*i);
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
            constructorIndex++, emptyStr, RT_MODE_TWOWAY,
            emptyStr, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")),
            0, 0);
    }
    sal_uInt32 size;
    void const * blob = writer.getBlob(&size);
    if (localKey.setValue(
            emptyStr, RG_VALUETYPE_BINARY, const_cast< void * >(blob),
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

    OUString name(OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8));
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
