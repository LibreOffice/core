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
    OUString                           maLocalName;
    std::unique_ptr<SvXMLNamespaceMap> m_pRewindMap;
    oslInterlockedCount                m_nRefCount;
    sal_uInt16                         mnPrefix;
    bool                               mbPrefixAndLocalNameFilledIn;

    SAL_DLLPRIVATE std::unique_ptr<SvXMLNamespaceMap> TakeRewindMap() { return std::move(m_pRewindMap); }
    SAL_DLLPRIVATE void PutRewindMap(std::unique_ptr<SvXMLNamespaceMap> p) { m_pRewindMap = std::move(p); }

protected:

    SvXMLImport& GetImport() { return mrImport; }
    const SvXMLImport& GetImport() const { return mrImport; }

public:

    bool IsPrefixFilledIn() const { return mnPrefix != 0; }
    sal_uInt16 GetPrefix() const { assert(mbPrefixAndLocalNameFilledIn && "those fields not filled, probably fast-parser context"); return mnPrefix; }
    const OUString& GetLocalName() const { assert(mbPrefixAndLocalNameFilledIn && "those fields not filled, probably fast-parser context"); return maLocalName; }

    /** A contexts constructor does anything that is required if an element
     * starts. Namespace processing has been done already.
     * Note that virtual methods cannot be used inside constructors. Use
     * StartElement instead if this is required. */
    SvXMLImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                        const OUString& rLName );

    SvXMLImportContext( SvXMLImport& rImport );

    /** A contexts destructor does anything that is required if an element
     * ends. By default, nothing is done.
     * Note that virtual methods cannot be used inside destructors. Use
     * EndElement instead if this is required. */
    virtual ~SvXMLImportContext();

    /** Create a children element context. By default, the import's
     * CreateContext method is called to create a new default context. */
    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    /** StartElement is called after a context has been constructed and
     * before an elements context is parsed. It may be used for actions that
     * require virtual methods. The default is to do nothing. */
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList );

    /** EndElement is called before a context will be destructed, but
     * after an elements context has been parsed. It may be used for actions
     * that require virtual methods. The default is to do nothing. */
    virtual void EndElement();

    /** This method is called for all characters that are contained in the
     * current element. The default is to ignore them. */
    virtual void Characters( const OUString& rChars );

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;

    virtual void SAL_CALL startUnknownElement(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

    virtual void SAL_CALL endFastElement(sal_Int32 Element) override;

    virtual void SAL_CALL endUnknownElement(const OUString & Namespace, const OUString & Name) override;

    virtual css::uno::Reference< XFastContextHandler >  SAL_CALL createFastChildContext(sal_Int32 Element,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& Attribs) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(
        const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

    virtual void SAL_CALL characters(const OUString & aChars) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) final override;
    virtual void SAL_CALL acquire() throw () final override
    { osl_atomic_increment(&m_nRefCount); }
    virtual void SAL_CALL release() throw () final override
    { if (osl_atomic_decrement(&m_nRefCount) == 0) delete this; }

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) final override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) final override;
};

#endif // INCLUDED_XMLOFF_XMLICTXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
