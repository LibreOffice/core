/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astunion.cxx,v $
 * $Revision: 1.10 $
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
#include <idlc/astunion.hxx>
#include <idlc/astbasetype.hxx>
#include <idlc/errorhandler.hxx>

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

AstUnion::AstUnion(const ::rtl::OString& name, AstType* pDiscType, AstScope* pScope)
    : AstStruct(NT_union, name, NULL, pScope)
    , m_pDiscriminantType(pDiscType)
    , m_discExprType(ET_long)
{
    AstBaseType* pBaseType;

    if ( !pDiscType )
    {
        m_pDiscriminantType = NULL;
        m_discExprType = ET_none;
        return;
    }
    /*
     * If the discriminator type is a predefined type
     * then install the equivalent coercion target type in
     * the pd_udisc_type field.
     */
    if ( pDiscType->getNodeType() == NT_predefined )
    {
        pBaseType = (AstBaseType*)pDiscType;
        if ( !pBaseType )
        {
            m_pDiscriminantType = NULL;
            m_discExprType = ET_none;
            return;
        }
        m_pDiscriminantType = pDiscType;
        switch (pBaseType->getExprType())
        {
            case ET_long:
            case ET_ulong:
            case ET_short:
            case ET_ushort:
            case ET_char:
            case ET_boolean:
                m_discExprType = pBaseType->getExprType();
                break;
            default:
                m_discExprType = ET_none;
                m_pDiscriminantType = NULL;
                break;
        }
    } else
        if (pDiscType->getNodeType() == NT_enum)
        {
            m_discExprType = ET_any;
            m_pDiscriminantType = pDiscType;
        } else
        {
            m_discExprType = ET_none;
            m_pDiscriminantType = NULL;
        }

    if ( !m_pDiscriminantType )
        idlc()->error()->error2(EIDL_DISC_TYPE, this, pDiscType);
}

AstUnion::~AstUnion()
{
}

AstDeclaration* AstUnion::addDeclaration(AstDeclaration* pDecl)
{
    if ( pDecl->getNodeType() == NT_union_branch )
    {
        AstUnionBranch* pBranch = (AstUnionBranch*)pDecl;
        if ( lookupBranch(pBranch) )
        {
            idlc()->error()->error2(EIDL_MULTIPLE_BRANCH, this, pDecl);
            return NULL;
        }
    }

    return AstScope::addDeclaration(pDecl);
}

AstUnionBranch* AstUnion::lookupBranch(AstUnionBranch* pBranch)
{
    AstUnionLabel* pLabel = NULL;

    if ( pBranch )
        pLabel = pBranch->getLabel();

    if ( pLabel )
    {
        if (pLabel->getLabelKind() == UL_default)
            return lookupDefault();
        if (m_discExprType == ET_any)
            /* CONVENTION: indicates enum discr */
            return lookupEnum(pBranch);
        return lookupLabel(pBranch);
    }
    return NULL;
}

AstUnionBranch* AstUnion::lookupDefault(sal_Bool bReportError)
{
    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstUnionBranch      *pBranch = NULL;
    AstDeclaration      *pDecl = NULL;

    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getNodeType() == NT_union_branch )
        {
            pBranch = (AstUnionBranch*)pDecl;
            if (pBranch == NULL)
            {
                ++iter;
                continue;
            }
            if ( pBranch->getLabel() != NULL &&
                 pBranch->getLabel()->getLabelKind() == UL_default)
            {
                if ( bReportError )
                    idlc()->error()->error2(EIDL_MULTIPLE_BRANCH, this, pBranch);
                return pBranch;
            }
        }
        ++iter;
    }
    return NULL;
}

AstUnionBranch* AstUnion::lookupLabel(AstUnionBranch* pBranch)
{
    AstUnionLabel* pLabel = pBranch->getLabel();

    if ( !pLabel->getLabelValue() )
        return pBranch;
//  pLabel->getLabelValue()->setExprValue(pLabel->getLabelValue()->coerce(m_discExprType, sal_False));
    AstExprValue* pLabelValue = pLabel->getLabelValue()->coerce(
        m_discExprType, sal_False);
    if ( !pLabelValue )
    {
        idlc()->error()->evalError(pLabel->getLabelValue());
        return pBranch;
    } else
    {
        pLabel->getLabelValue()->setExprValue(pLabelValue);
    }

    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstUnionBranch* pB = NULL;
    AstDeclaration* pDecl = NULL;

    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getNodeType() == NT_union_branch )
        {
            pB = (AstUnionBranch*)pDecl;
            if ( !pB )
            {
                ++iter;
                continue;
            }
            if ( pB->getLabel() != NULL &&
                 pB->getLabel()->getLabelKind() == UL_label &&
                 pB->getLabel()->getLabelValue()->compare(pLabel->getLabelValue()) )
            {
                idlc()->error()->error2(EIDL_MULTIPLE_BRANCH, this, pBranch);
                return pBranch;
            }
        }
        ++iter;
    }
    return NULL;
}

AstUnionBranch* AstUnion::lookupEnum(AstUnionBranch* pBranch)
{
    AstDeclaration const * pType = resolveTypedefs(m_pDiscriminantType);
    if ( pType->getNodeType() != NT_enum )
        return NULL;

    AstUnionLabel* pLabel = pBranch->getLabel();
    AstExpression* pExpr = pLabel->getLabelValue();
    if ( !pExpr )
        return pBranch;

    /*
     * Expecting a symbol label
     */
    if ( pExpr->getCombOperator() != EC_symbol)
    {
        idlc()->error()->enumValExpected(this);
        return pBranch;
    }

    /*
     * See if the symbol defines a constant in the discriminator enum
     */
    AstEnum* pEnum = (AstEnum*)pType;
    AstDeclaration* pDecl = pEnum->lookupByName(*pExpr->getSymbolicName());
    if ( pDecl == NULL || pDecl->getScope() != pEnum)
    {
        idlc()->error()->enumValLookupFailure(this, pEnum, *pExpr->getSymbolicName());
        return pBranch;
    }


    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstUnionBranch* pB = NULL;
    pDecl = NULL;

    while ( iter != end )
    {
        pDecl = *iter;
        if ( pDecl->getNodeType() == NT_union_branch )
        {
            pB = (AstUnionBranch*)pDecl;
            if ( !pB )
            {
                ++iter;
                continue;
            }
            if ( pB->getLabel() != NULL &&
                 pB->getLabel()->getLabelKind() == UL_label &&
                 pB->getLabel()->getLabelValue()->compare(pLabel->getLabelValue()) )
            {
                idlc()->error()->error2(EIDL_MULTIPLE_BRANCH, this, pBranch);
                return pBranch;
            }
        }
        ++iter;
    }
    return NULL;
}

sal_Bool AstUnion::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    sal_uInt16 nMember = getNodeCount(NT_union_branch);

    OUString emptyStr;
    typereg::Writer aBlob(
        TYPEREG_VERSION_0, getDocumentation(), emptyStr, RT_TYPE_UNION,
        false, OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 1,
        nMember, 0, 0);
    aBlob.setSuperTypeName(
        0,
        OStringToOUString(
            getDiscrimantType()->getScopedName(), RTL_TEXTENCODING_UTF8));

    if ( nMember > 0 )
    {
        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        AstUnionBranch* pBranch = NULL;
        AstUnionBranch* pDefault = lookupDefault(sal_False);
        AstUnionLabel*  pLabel = NULL;
        AstExprValue*   pExprValue = NULL;
        RTConstValue    aConst;
        RTFieldAccess   access = RT_ACCESS_READWRITE;
        OUString    docu;
        sal_uInt16  index = 0;
        if ( pDefault )
            index = 1;

        sal_Int64   disc = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_union_branch )
            {
                pBranch = (AstUnionBranch*)pDecl;
                if (pBranch == pDefault)
                {
                    ++iter;
                    continue;
                }

                pLabel = pBranch->getLabel();
                pExprValue = pLabel->getLabelValue()->coerce(ET_hyper, sal_False);
                aConst.m_type = RT_TYPE_INT64;
                aConst.m_value.aHyper = pExprValue->u.hval;
                if ( aConst.m_value.aHyper > disc )
                    disc = aConst.m_value.aHyper;

                aBlob.setFieldData(
                    index++, pBranch->getDocumentation(), emptyStr, RT_ACCESS_READWRITE,
                    OStringToOUString(
                        pBranch->getLocalName(), RTL_TEXTENCODING_UTF8),
                    OStringToOUString(
                        pBranch->getType()->getRelativName(),
                        RTL_TEXTENCODING_UTF8),
                    aConst);
            }
            ++iter;
        }

        if ( pDefault )
        {
            access = RT_ACCESS_DEFAULT;
            aConst.m_type = RT_TYPE_INT64;
            aConst.m_value.aHyper = disc + 1;
            aBlob.setFieldData(
                0, pDefault->getDocumentation(), emptyStr, RT_ACCESS_DEFAULT,
                OStringToOUString(
                    pDefault->getLocalName(), RTL_TEXTENCODING_UTF8),
                OStringToOUString(
                    pDefault->getType()->getRelativName(),
                    RTL_TEXTENCODING_UTF8),
                aConst);
        }
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

    return sal_True;
}

AstUnionBranch::AstUnionBranch(AstUnionLabel* pLabel, AstType const * pType, const ::rtl::OString& name, AstScope* pScope)
    : AstMember(NT_union_branch, pType, name, pScope)
    , m_pLabel(pLabel)
{
}

AstUnionBranch::~AstUnionBranch()
{
    if ( m_pLabel )
        delete m_pLabel;
}

AstUnionLabel::AstUnionLabel()
    : m_label(UL_default)
    , m_pLabelValue(NULL)
{
}

AstUnionLabel::AstUnionLabel(UnionLabel labelKind, AstExpression* pExpr)
    : m_label(labelKind)
    , m_pLabelValue(pExpr)
{
    if ( m_pLabelValue )
        m_pLabelValue->evaluate(EK_const);
}

AstUnionLabel::~AstUnionLabel()
{
    if ( m_pLabelValue )
        delete m_pLabelValue;
}

