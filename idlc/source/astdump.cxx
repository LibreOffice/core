/*************************************************************************
 *
 *  $RCSfile: astdump.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
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

using namespace ::rtl;

sal_Bool AstModule::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
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
                    getFullName().getStr(), rKey.getRegistryName().getStr());
            return sal_False;
        }
    }

    const sal_uInt8*    pBlob = NULL;
    sal_uInt32          aBlobSize = 0;
    sal_uInt16          nConst = getNodeCount(NT_const);

    if ( nConst > 0 )
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        RegistryTypeWriter aBlob(pLoader->getApi(),
                                 typeClass,
                                 OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                                 OUString(), nConst, 0, 0);

        aBlob.setDoku( getDocumentation() );
        aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

        DeclList::iterator iter = getIteratorBegin();
        DeclList::iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_const &&
                 pDecl->isInMainfile() )
            {
                ((AstConstant*)pDecl)->dumpBlob(aBlob, index++);
            }
            iter++;
        }

        pBlob = aBlob.getBlop();
        aBlobSize = aBlob.getBlopSize();

        if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName(), localKey.getRegistryName());
            return sal_False;
        }
    } else
    {
        RTTypeClass typeClass = RT_TYPE_MODULE;
        if ( getNodeType() == NT_constants )
            typeClass = RT_TYPE_CONSTANTS;

        RegistryTypeWriter aBlob(pLoader->getApi(),
                                 typeClass,
                                 OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                                 OUString(), 0, 0, 0);

        aBlob.setDoku( getDocumentation() );
        if ( getNodeType() == NT_constants )
            aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

        pBlob = aBlob.getBlop();
        aBlobSize = aBlob.getBlopSize();

        if ( getNodeType() != NT_root )
        {
            if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                    (RegValue)pBlob, aBlobSize))
            {
                fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                        idlc()->getOptions()->getProgramName().getStr(),
                        getFullName(), localKey.getRegistryName());
                return sal_False;
            }
        }
    }
    return AstDeclaration::dump(rKey, pLoader);
}

sal_Bool AstTypeDef::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), rKey.getRegistryName().getStr());
        return sal_False;
    }

    RegistryTypeWriter aBlob(pLoader->getApi(), RT_TYPE_TYPEDEF,
                             OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                             OStringToOUString(getBaseType()->getRelativName(), RTL_TEXTENCODING_UTF8),
                             0, 0, 0);

    aBlob.setDoku( getDocumentation() );
    aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

    const sal_uInt8*    pBlob = aBlob.getBlop();
    sal_uInt32          aBlobSize = aBlob.getBlopSize();

    if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY, (RegValue)pBlob, aBlobSize))
    {
        fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName(), localKey.getRegistryName());
        return sal_False;
    }

    return sal_True;
}

sal_Bool AstService::dump(RegistryKey& rKey, RegistryTypeWriterLoader* pLoader)
{
    if ( !idlc()->getOptions()->isValid("-C") )
        return sal_True;

    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), rKey.getRegistryName().getStr());
        return sal_False;
    }

    sal_uInt16 nProperties = 0;
    sal_uInt16 nIfaceMember = 0;
    sal_uInt16 nServMember = 0;
    sal_uInt16 nNeeds = 0;
    sal_uInt16 nObserves = 0;
    if ( nMembers() > 0 )
    {
        DeclList::iterator iter = getIteratorBegin();
        DeclList::iterator end = getIteratorEnd();
        while ( iter != end )
        {
            switch ( (*iter)->getNodeType() )
            {
                case NT_property:
                    nProperties++;
                    break;
                case NT_interface_member:
                    nIfaceMember++;
                    break;
                case NT_service_member:
                    nServMember++;
                    break;
                case NT_observes:
                    nObserves++;
                    break;
                case NT_needs:
                    nNeeds++;
                    break;
            }
            iter++;
        }
    }
    sal_uInt16 nReferences = nIfaceMember + nServMember + nObserves + nNeeds;

    RegistryTypeWriter aBlob(pLoader->getApi(), RT_TYPE_SERVICE,
                             OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8),
                             OUString(), nProperties, 0, nReferences);

    aBlob.setDoku( getDocumentation() );
    aBlob.setFileName( OStringToOUString(getFileName(), RTL_TEXTENCODING_UTF8));

    if ( nProperties || nReferences )
    {
        DeclList::iterator iter = getIteratorBegin();
        DeclList::iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16  propertyIndex = 0;
        sal_uInt16  referenceIndex = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            switch ( pDecl->getNodeType() )
            {
            case NT_property:
                ((AstAttribute*)pDecl)->dumpBlob(aBlob, propertyIndex++);
                break;
            case NT_interface_member:
            {
                AstInterfaceMember* pIfaceMember = (AstInterfaceMember*)pDecl;
                sal_uInt16 access = (pIfaceMember->isOptional() ? RT_ACCESS_OPTIONAL : RT_ACCESS_INVALID);
                aBlob.setReferenceData(referenceIndex++,
                           OStringToOUString( pIfaceMember->getRealInterface()->getRelativName(), RTL_TEXTENCODING_UTF8),
                           RT_REF_SUPPORTS, pIfaceMember->getDocumentation(), access);
            }
                break;
            case NT_service_member:
            {
                AstServiceMember* pServMember = (AstServiceMember*)pDecl;
                aBlob.setReferenceData(referenceIndex++,
                           OStringToOUString( pServMember->getRealService()->getRelativName(), RTL_TEXTENCODING_UTF8),
                           RT_REF_EXPORTS, pServMember->getDocumentation());
            }
                break;
            case NT_observes:
            {
                AstObserves* pObserves = (AstObserves*)pDecl;
                aBlob.setReferenceData(referenceIndex++,
                           OStringToOUString( pObserves->getRealInterface()->getRelativName(), RTL_TEXTENCODING_UTF8),
                           RT_REF_OBSERVES, pObserves->getDocumentation());
            }
                break;
            case NT_needs:
            {
                AstNeeds* pNeeds = (AstNeeds*)pDecl;
                aBlob.setReferenceData(referenceIndex++,
                           OStringToOUString( pNeeds->getRealService()->getRelativName(), RTL_TEXTENCODING_UTF8),
                           RT_REF_NEEDS, pNeeds->getDocumentation());
            }
                break;
            }
            iter++;
        }

        const sal_uInt8* pBlob = aBlob.getBlop();
        sal_uInt32       aBlobSize = aBlob.getBlopSize();

        if (localKey.setValue(OUString(), RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName(), localKey.getRegistryName());
            return sal_False;
        }
    }

    return sal_True;
}

sal_Bool AstAttribute::dumpBlob(RegistryTypeWriter& rBlob, sal_uInt16 index)
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

    rBlob.setFieldData(index, OStringToOUString(getLocalName(), RTL_TEXTENCODING_UTF8),
                       OStringToOUString(getType()->getRelativName(), RTL_TEXTENCODING_UTF8),
                       getDocumentation(), OUString(), accessMode);

    return sal_True;
}

AstType* SAL_CALL resolveTypeDef(AstType* pType)
{
    if ( pType->getNodeType() == NT_typedef )
    {
        return resolveTypeDef(((AstTypeDef*)pType)->getBaseType());
    }
    return pType;
}

const sal_Char* AstSequence::getRelativName()
{
    if ( !m_pRelativName )
    {
        m_pRelativName = new OString("[]");
        AstType* pType = resolveTypeDef( m_pMemberType );
        *m_pRelativName += pType->getRelativName();
    }

    return m_pRelativName->getStr();
}
