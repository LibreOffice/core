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
#ifndef INCLUDED_XMLOFF_XMLCHARCONTEXT_HXX
#define INCLUDED_XMLOFF_XMLCHARCONTEXT_HXX

#include <xmloff/dllapi.h>
#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

class XMLOFF_DLLPUBLIC XMLCharContext : public SvXMLImportContext
{
    XMLCharContext(const XMLCharContext&) = delete;
    void operator =(const XMLCharContext&) = delete;
    sal_Int16 const   m_nControl;
protected:
    sal_uInt16  m_nCount;
    sal_Unicode m_c;
public:


    XMLCharContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            bool bCount );
    XMLCharContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual ~XMLCharContext() override;

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement() override;

    virtual void InsertControlCharacter(sal_Int16   _nControl);
    virtual void InsertString(const OUString& _sString);
};

#endif // INCLUDED_XMLOFF_XMLCHARCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
