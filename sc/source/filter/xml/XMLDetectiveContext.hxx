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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLDETECTIVECONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLDETECTIVECONTEXT_HXX

#include <xmloff/xmlimp.hxx>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "detfunc.hxx"
#include "detdata.hxx"
#include "xmlimprt.hxx"

#include <list>

struct ScMyImpDetectiveObj
{
    ScRange                     aSourceRange;
    ScDetectiveObjType          eObjType;
    bool                        bHasError;

                                ScMyImpDetectiveObj();
};

typedef ::std::vector< ScMyImpDetectiveObj > ScMyImpDetectiveObjVec;

struct ScMyImpDetectiveOp
{
    ScAddress                   aPosition;
    ScDetOpType                 eOpType;
    sal_Int32                   nIndex;

    ScMyImpDetectiveOp()
        : eOpType(SCDETOP_ADDSUCC)
        , nIndex(-1)
    {
    }

    bool operator<(const ScMyImpDetectiveOp& rDetOp) const;
};

typedef ::std::list< ScMyImpDetectiveOp > ScMyImpDetectiveOpList;

class ScMyImpDetectiveOpArray
{
private:
    ScMyImpDetectiveOpList      aDetectiveOpList;

public:
    inline                      ScMyImpDetectiveOpArray() :
                                    aDetectiveOpList()  {}

    inline void                 AddDetectiveOp( const ScMyImpDetectiveOp& rDetOp )
                                    { aDetectiveOpList.push_back( rDetOp ); }

    void                        Sort();
    bool                        GetFirstOp( ScMyImpDetectiveOp& rDetOp );
};

class ScXMLDetectiveContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*     pDetectiveObjVec;

    const ScXMLImport&          GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport&                GetScImport()       { return static_cast<ScXMLImport&>(GetImport()); }

public:
                                ScXMLDetectiveContext(
                                    ScXMLImport& rImport,
                                    sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList
                                    ) override;
    virtual void                EndElement() override;
};

class ScXMLDetectiveHighlightedContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*     pDetectiveObjVec;
    ScMyImpDetectiveObj         aDetectiveObj;
    bool                        bValid;

    const ScXMLImport&          GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport&                GetScImport()       { return static_cast<ScXMLImport&>(GetImport()); }

public:
                                ScXMLDetectiveHighlightedContext(
                                    ScXMLImport& rImport,
                                    sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveHighlightedContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList
                                    ) override;
    virtual void                EndElement() override;
};

class ScXMLDetectiveOperationContext : public SvXMLImportContext
{
private:
    ScMyImpDetectiveOp          aDetectiveOp;
    bool                        bHasType;

    const ScXMLImport&          GetScImport() const { return static_cast<const ScXMLImport&>(GetImport()); }
    ScXMLImport&                GetScImport()       { return static_cast<ScXMLImport&>(GetImport()); }

public:
                                ScXMLDetectiveOperationContext(
                                    ScXMLImport& rImport,
                                    sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList
                                    );
    virtual                     ~ScXMLDetectiveOperationContext();

    virtual SvXMLImportContext* CreateChildContext(
                                    sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList
                                    ) override;
    virtual void                EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
