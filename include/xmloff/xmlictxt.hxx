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

#ifndef INCLUDED_XMLOFF_XMLICTXT_HXX
#define INCLUDED_XMLOFF_XMLICTXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/xml/sax/XFastContextHandler.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <rtl/ustring.hxx>
#include <xmloff/namespacemap.hxx>
#include <memory>

namespace com::sun::star::xml::sax { class XAttributeList; }

class SvXMLImport;

class SvXMLImportContext;

typedef rtl::Reference<SvXMLImportContext> SvXMLImportContextRef;

/**
This class deliberately does not support XWeak, to improve performance when loading
large documents.
*/
class XMLOFF_DLLPUBLIC SvXMLImportContext : public css::xml::sax::XFastContextHandler,
                                            public css::lang::XTypeProvider

{
    friend class SvXMLImport;

    SvXMLImport&                       mrImport;
    oslInterlockedCount                m_nRefCount;
    std::unique_ptr<SvXMLNamespaceMap> m_pRewindMap;

    SAL_DLLPRIVATE std::unique_ptr<SvXMLNamespaceMap> TakeRewindMap() { return std::move(m_pRewindMap); }
    SAL_DLLPRIVATE void PutRewindMap(std::unique_ptr<SvXMLNamespaceMap> p) { m_pRewindMap = std::move(p); }

protected:

    SvXMLImport& GetImport() { return mrImport; }
    const SvXMLImport& GetImport() const { return mrImport; }

public:

    /** A contexts constructor does anything that is required if an element
     * starts. Namespace processing has been done already.
     * Note that virtual methods cannot be used inside constructors. Use
     * StartElement instead if this is required. */
    SvXMLImportContext( SvXMLImport& rImport );

    /** A contexts destructor does anything that is required if an element
     * ends. By default, nothing is done.
     * Note that virtual methods cannot be used inside destructors. Use
     * EndElement instead if this is required. */
    virtual ~SvXMLImportContext();

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;

    virtual void SAL_CALL startUnknownElement(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

    /** endFastElement is called before a context will be destructed, but
     * after an elements context has been parsed. It may be used for actions
     * that require virtual methods. The default is to do nothing. */
    virtual void SAL_CALL endFastElement(sal_Int32 Element) override;

    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name) override;

    virtual css::uno::Reference< XFastContextHandler >  SAL_CALL createFastChildContext(sal_Int32 Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(
        const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

    /** This method is called for all characters that are contained in the
     * current element. The default is to ignore them. */
    virtual void SAL_CALL characters(const OUString & aChars) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) final override;
    virtual void SAL_CALL acquire() noexcept final override
    { osl_atomic_increment(&m_nRefCount); }
    virtual void SAL_CALL release() noexcept final override
    { if (osl_atomic_decrement(&m_nRefCount) == 0) delete this; }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) final override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) final override;
};

#define XMLOFF_WARN_UNKNOWN(area, rIter) \
    SAL_WARN(area, "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(rIter.getToken()) << " value=" << rIter.toString());

#define XMLOFF_WARN_UNKNOWN_ATTR(area, token, value) \
    SAL_WARN(area, "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(token) << "=" << value);

#define XMLOFF_WARN_UNKNOWN_ELEMENT(area, token) \
    SAL_WARN(area, "unknown element " << SvXMLImport::getPrefixAndNameFromToken(token));

#endif // INCLUDED_XMLOFF_XMLICTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
