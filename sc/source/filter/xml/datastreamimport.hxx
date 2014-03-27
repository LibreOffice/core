/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_XML_DATASTREAMIMPORT_HXX
#define SC_XML_DATASTREAMIMPORT_HXX

#include "importcontext.hxx"

#include <importfilterdata.hxx>
#include <address.hxx>

class ScXMLDataStreamContext : public ScXMLImportContext
{
    OUString maURL;
    ScRange maRange;
    bool mbRefreshOnEmpty;
    sc::ImportPostProcessData::DataStream::InsertPos meInsertPos;

public:
    ScXMLDataStreamContext(
        ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual ~ScXMLDataStreamContext();

    virtual void EndElement() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
