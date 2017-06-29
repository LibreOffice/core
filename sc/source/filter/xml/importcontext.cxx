/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "importcontext.hxx"
#include "xmlimprt.hxx"

ScXMLImportContext::ScXMLImportContext(ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName) :
    SvXMLImportContext(rImport, nPrefix, rLocalName)
{
}

ScXMLImportContext::ScXMLImportContext(SvXMLImport& rImport ) :
    SvXMLImportContext( rImport )
{
}

ScXMLImport& ScXMLImportContext::GetScImport()
{
    return static_cast<ScXMLImport&>(GetImport());
}

const ScXMLImport& ScXMLImportContext::GetScImport() const
{
    return static_cast<const ScXMLImport&>(GetImport());
}

void SAL_CALL ScXMLImportContext::startFastElement(sal_Int32 /*nElement*/, const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/)
{
}

void SAL_CALL ScXMLImportContext::endFastElement(sal_Int32 /*nElement*/)
{
}

void SAL_CALL ScXMLImportContext::characters(const OUString &)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL ScXMLImportContext::createFastChildContext(
    sal_Int32/* nElement */, const css::uno::Reference< css::xml::sax::XFastAttributeList >&/* xAttrList */ )
{
    return new SvXMLImportContext( GetImport() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
