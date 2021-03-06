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

#pragma once

#include <detfunc.hxx>
#include <detdata.hxx>
#include "importcontext.hxx"

#include <list>

namespace sax_fastparser { class FastAttributeList; }

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
    ScMyImpDetectiveOpArray() :
                                    aDetectiveOpList()  {}

    void                 AddDetectiveOp( const ScMyImpDetectiveOp& rDetOp )
                                    { aDetectiveOpList.push_back( rDetOp ); }

    void                        Sort();
    bool                        GetFirstOp( ScMyImpDetectiveOp& rDetOp );
};

class ScXMLDetectiveContext : public ScXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*     pDetectiveObjVec;

public:
                                ScXMLDetectiveContext(
                                    ScXMLImport& rImport,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
                                createFastChildContext(
                                    sal_Int32 nElement,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList
                                    ) override;
};

class ScXMLDetectiveHighlightedContext : public ScXMLImportContext
{
private:
    ScMyImpDetectiveObjVec*     pDetectiveObjVec;
    ScMyImpDetectiveObj         aDetectiveObj;
    bool                        bValid;

public:
                                ScXMLDetectiveHighlightedContext(
                                    ScXMLImport& rImport,
                                    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                    ScMyImpDetectiveObjVec* pNewDetectiveObjVec
                                    );
    virtual                     ~ScXMLDetectiveHighlightedContext() override;

    virtual void SAL_CALL       endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDetectiveOperationContext : public ScXMLImportContext
{
private:
    ScMyImpDetectiveOp          aDetectiveOp;
    bool                        bHasType;

public:
                                ScXMLDetectiveOperationContext(
                                    ScXMLImport& rImport,
                                    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList
                                    );
    virtual                     ~ScXMLDetectiveOperationContext() override;

    virtual void SAL_CALL       endFastElement( sal_Int32 nElement ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
