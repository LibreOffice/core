/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_IMPORTCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_IMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>

class ScXMLImport;

/**
 * This class exists only to provide GetScImport() to its derived classes.
 */
class ScXMLImportContext : public SvXMLImportContext
{
public:
    ScXMLImportContext(
        ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName);

    ScXMLImportContext( SvXMLImport& rImport );

    virtual void SAL_CALL startFastElement (sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList) override;

    virtual void SAL_CALL characters(const OUString & aChars) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;


protected:
    ScXMLImport& GetScImport();
    const ScXMLImport& GetScImport() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

#endif
