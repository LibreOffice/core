/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _IDLC_ASTATTRIBUTE_HXX_
#define _IDLC_ASTATTRIBUTE_HXX_

#include <idlc/astdeclaration.hxx>
#include "idlc/astscope.hxx"

#include "registry/types.h"
#include "rtl/ustring.hxx"

namespace typereg { class Writer; }

class AstAttribute: public AstDeclaration, public AstScope {
public:
    AstAttribute(
        sal_uInt32 flags, AstType const * type, OString const & name,
        AstScope * scope):
        AstDeclaration(NT_attribute, name, scope),
        AstScope(NT_attribute), m_flags(flags), m_pType(type)
    {}

    AstAttribute(NodeType nodeType, sal_uInt32 flags, AstType const * pType, const OString& name, AstScope* pScope)
        : AstDeclaration(nodeType, name, pScope), AstScope(nodeType)
        , m_flags(flags)
        , m_pType(pType)
        {}
    virtual ~AstAttribute() {}

    void setExceptions(
        OUString const * getDoc, DeclList const * getExc,
        OUString const * setDoc, DeclList const * setExc)
    {
        if (getDoc != 0) {
            m_getDocumentation = *getDoc;
        }
        if (getExc != 0) {
            m_getExceptions = *getExc;
        }
        if (setDoc != 0) {
            m_setDocumentation = *setDoc;
        }
        if (setExc != 0) {
            m_setExceptions = *setExc;
        }
    }

    DeclList::size_type getGetExceptionCount() const
    { return m_getExceptions.size(); }

    DeclList::size_type getSetExceptionCount() const
    { return m_setExceptions.size(); }

    AstType const * getType() const
        { return m_pType; }
    sal_Bool isReadonly() const
        { return ((m_flags & AF_READONLY) == AF_READONLY); }
    sal_Bool isOptional() const
        { return ((m_flags & AF_OPTIONAL) == AF_OPTIONAL); }
    sal_Bool isAttribute() const
        { return ((m_flags & AF_ATTRIBUTE) == AF_ATTRIBUTE); }
    sal_Bool isProperty() const
        { return ((m_flags & AF_PROPERTY) == AF_PROPERTY); }
    sal_Bool isBound() const
        { return ((m_flags & AF_BOUND) == AF_BOUND); }
    sal_Bool isMayBeVoid() const
        { return ((m_flags & AF_MAYBEVOID) == AF_MAYBEVOID); }
    sal_Bool isConstrained() const
        { return ((m_flags & AF_CONSTRAINED) == AF_CONSTRAINED); }
    sal_Bool isTransient() const
        { return ((m_flags & AF_TRANSIENT) == AF_TRANSIENT); }
    sal_Bool isMayBeAmbiguous() const
        { return ((m_flags & AF_MAYBEAMBIGUOUS) == AF_MAYBEAMBIGUOUS); }
    sal_Bool isMayBeDefault() const
        { return ((m_flags & AF_MAYBEDEFAULT) == AF_MAYBEDEFAULT); }
    sal_Bool isRemoveable() const
        { return ((m_flags & AF_REMOVEABLE) == AF_REMOVEABLE); }

    sal_Bool dumpBlob(
        typereg::Writer & rBlob, sal_uInt16 index, sal_uInt16 * methodIndex);

private:
    void dumpExceptions(
        typereg::Writer & writer, OUString const & documentation,
        DeclList const & exceptions, RTMethodMode flags,
        sal_uInt16 * methodIndex);

    const sal_uInt32    m_flags;
    AstType const * m_pType;
    OUString m_getDocumentation;
    DeclList m_getExceptions;
    OUString m_setDocumentation;
    DeclList m_setExceptions;
};

#endif // _IDLC_ASTATTRIBUTE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
