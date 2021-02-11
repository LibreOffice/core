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

#include <memory>
#include "XMLTrackedChangesContext.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include <formulacell.hxx>
#include <textuno.hxx>
#include <editutil.hxx>
#include <document.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <svl/sharedstringpool.hxx>
#include <comphelper/base64.hxx>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

namespace {

class ScXMLChangeInfoContext : public ScXMLImportContext
{
    ScMyActionInfo                      aInfo;
    OUStringBuffer                      sAuthorBuffer{32};
    OUStringBuffer                      sDateTimeBuffer{32};
    OUStringBuffer                      sCommentBuffer{64};
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    sal_uInt32                          nParagraphCount;

public:
    ScXMLChangeInfoContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLBigRangeContext : public ScXMLImportContext
{
public:
    ScXMLBigRangeContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScBigRange& rBigRange);
};

class ScXMLCellContentDeletionContext : public ScXMLImportContext
{
    OUString                       sFormulaAddress;
    OUString                       sFormula;
    OUString                       sFormulaNmsp;
    OUString                       sInputString;
    ScBigRange                          aBigRange;
    double                              fValue;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScCellValue maCell;
    sal_uInt32                          nID;
    sal_Int32                           nMatrixCols;
    sal_Int32                           nMatrixRows;
    formula::FormulaGrammar::Grammar    eGrammar;
    sal_uInt16                          nType;
    ScMatrixMode                        nMatrixFlag;

public:
    ScXMLCellContentDeletionContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLDependenceContext : public ScXMLImportContext
{
public:
    ScXMLDependenceContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
};

class ScXMLDependingsContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLDependingsContext( ScXMLImport& rImport,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLChangeDeletionContext : public ScXMLImportContext
{
public:
    ScXMLChangeDeletionContext( ScXMLImport& rImport,
                                const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

};

class ScXMLDeletionsContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLDeletionsContext( ScXMLImport& rImport,
                           ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLChangeCellContext;

class ScXMLChangeTextPContext : public ScXMLImportContext
{
    css::uno::Reference< css::xml::sax::XFastAttributeList> mxAttrList;
    sal_Int32                   mnElement;
    OUStringBuffer              sText;
    ScXMLChangeCellContext*     pChangeCellContext;
    rtl::Reference<SvXMLImportContext>
                                pTextPContext;

public:

    ScXMLChangeTextPContext( ScXMLImport& rImport, sal_Int32 nElement,
                       const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLChangeCellContext* pChangeCellContext);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class ScXMLChangeCellContext : public ScXMLImportContext
{
    ScCellValue& mrOldCell;

    OUString           sText;
    OUString&          rInputString;
    rtl::Reference<ScEditEngineTextObj> mpEditTextObj;
    double                  fValue;
    sal_uInt16&             rType;
    bool                bEmpty;
    bool                bFirstParagraph;
    bool                bString;
    bool                bFormula;

public:
    ScXMLChangeCellContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScCellValue& rOldCell, OUString& sAddress,
                                      OUString& rFormula, OUString& rFormulaNmsp,
                                      formula::FormulaGrammar::Grammar& rGrammar,
                                      OUString& rInputString, double& fValue, sal_uInt16& nType,
                                      ScMatrixMode& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    void CreateTextPContext(bool bIsNewParagraph);
    bool IsEditCell() const { return mpEditTextObj.is(); }
    void SetText(const OUString& sTempText) { sText = sTempText; }

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLPreviousContext : public ScXMLImportContext
{
    OUString                       sFormulaAddress;
    OUString                       sFormula;
    OUString                       sFormulaNmsp;
    OUString                       sInputString;
    double                              fValue;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScCellValue maOldCell;
    sal_uInt32                          nID;
    sal_Int32                           nMatrixCols;
    sal_Int32                           nMatrixRows;
    formula::FormulaGrammar::Grammar    eGrammar;
    sal_uInt16                          nType;
    ScMatrixMode                        nMatrixFlag;

public:
    ScXMLPreviousContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLContentChangeContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScBigRange                          aBigRange;

public:
    ScXMLContentChangeContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLInsertionContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLInsertionContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLInsertionCutOffContext : public ScXMLImportContext
{
public:
    ScXMLInsertionCutOffContext( ScXMLImport& rImport,
                                      const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
};

class ScXMLMovementCutOffContext : public ScXMLImportContext
{
public:
    ScXMLMovementCutOffContext( ScXMLImport& rImport,
                                      const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
};

class ScXMLCutOffsContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLCutOffsContext( ScXMLImport& rImport,
                         ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

class ScXMLDeletionContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLDeletionContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLMovementContext : public ScXMLImportContext
{
    ScBigRange                          aSourceRange;
    ScBigRange                          aTargetRange;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLMovementContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

class ScXMLRejectionContext : public ScXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

public:
    ScXMLRejectionContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

}

ScXMLTrackedChangesContext::ScXMLTrackedChangesContext( ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    rImport.LockSolarMutex();

    if ( !rAttrList.is() )
        return;

    auto aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_PROTECTION_KEY ) ) );
    if (aIter != rAttrList->end())
    {
        if( !aIter.isEmpty() )
        {
            uno::Sequence<sal_Int8> aPass;
            ::comphelper::Base64::decode( aPass, aIter.toString() );
            pChangeTrackingImportHelper->SetProtection(aPass);
        }
    }
}

ScXMLTrackedChangesContext::~ScXMLTrackedChangesContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLTrackedChangesContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( TABLE, XML_CELL_CONTENT_CHANGE ):
        pContext = new ScXMLContentChangeContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_INSERTION ):
        pContext = new ScXMLInsertionContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETION ):
        pContext = new ScXMLDeletionContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_MOVEMENT ):
        pContext = new ScXMLMovementContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_REJECTION ):
        pContext = new ScXMLRejectionContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    }

    return pContext;
}

ScXMLChangeInfoContext::ScXMLChangeInfoContext( ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    aInfo(),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nParagraphCount(0)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            sal_Int32 nToken = aIter.getToken();
            if ( nToken == XML_ELEMENT( OFFICE, XML_CHG_AUTHOR ) )
                sAuthorBuffer = aIter.toString();
            else if ( nToken == XML_ELEMENT( OFFICE, XML_CHG_DATE_TIME ) )
                sDateTimeBuffer = aIter.toString();
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > ScXMLChangeInfoContext::createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    SvXMLImportContext *pContext(nullptr);

    if( nElement == XML_ELEMENT(DC, XML_CREATOR ) )
        pContext = new ScXMLContentContext(GetScImport(), sAuthorBuffer);
    else if( nElement == XML_ELEMENT(DC, XML_DATE ) )
        pContext = new ScXMLContentContext(GetScImport(), sDateTimeBuffer);
    else if (nElement == XML_ELEMENT(TEXT, XML_P))
    {
        if(nParagraphCount)
            sCommentBuffer.append('\n');
        ++nParagraphCount;
        pContext = new ScXMLContentContext( GetScImport(), sCommentBuffer);
    }

    return pContext;
}

void SAL_CALL ScXMLChangeInfoContext::endFastElement( sal_Int32 /*nElement*/ )
{
    aInfo.sUser = sAuthorBuffer.makeStringAndClear();
    ::sax::Converter::parseDateTime(aInfo.aDateTime,
            sDateTimeBuffer.makeStringAndClear());
    aInfo.sComment = sCommentBuffer.makeStringAndClear();
    pChangeTrackingImportHelper->SetActionInfo(aInfo);
}

ScXMLBigRangeContext::ScXMLBigRangeContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScBigRange& rBigRange ) :
    ScXMLImportContext( rImport )
{
    bool bColumn(false);
    bool bRow(false);
    bool bTable(false);
    sal_Int32 nColumn(0);
    sal_Int32 nRow(0);
    sal_Int32 nTable(0);
    sal_Int32 nStartColumn(0);
    sal_Int32 nEndColumn(0);
    sal_Int32 nStartRow(0);
    sal_Int32 nEndRow(0);
    sal_Int32 nStartTable(0);
    sal_Int32 nEndTable(0);
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_COLUMN ):
                nColumn = aIter.toInt32();
                bColumn = true;
                break;
            case XML_ELEMENT( TABLE, XML_ROW ):
                nRow = aIter.toInt32();
                bRow = true;
                break;
            case XML_ELEMENT( TABLE, XML_TABLE ):
                nTable = aIter.toInt32();
                bTable = true;
                break;
            case XML_ELEMENT( TABLE, XML_START_COLUMN ):
                nStartColumn = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_END_COLUMN ):
                nEndColumn = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_START_ROW ):
                nStartRow = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_END_ROW ):
                nEndRow = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_START_TABLE ):
                nStartTable = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_END_TABLE ):
                nEndTable = aIter.toInt32();
                break;
            }
        }
    }

    if (bColumn)
        nStartColumn = nEndColumn = nColumn;
    if (bRow)
        nStartRow = nEndRow = nRow;
    if (bTable)
        nStartTable = nEndTable = nTable;
    rBigRange.Set(nStartColumn, nStartRow, nStartTable,
        nEndColumn, nEndRow, nEndTable);
}

ScXMLCellContentDeletionContext::ScXMLCellContentDeletionContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper) :
    ScXMLImportContext( rImport ),
    fValue(0.0),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nID(0),
    nMatrixCols(0),
    nMatrixRows(0),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nType(css::util::NumberFormat::ALL),
    nMatrixFlag(ScMatrixMode::NONE)
{
    if ( rAttrList.is() )
    {
        auto aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_ID ) ) );
        if (aIter != rAttrList->end())
            nID = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
    }
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLCellContentDeletionContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( TABLE, XML_CHANGE_TRACK_TABLE_CELL ):
        pContext = new ScXMLChangeCellContext(GetScImport(), pAttribList,
                maCell, sFormulaAddress, sFormula, sFormulaNmsp, eGrammar, sInputString, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows );
        break;
    case XML_ELEMENT( TABLE, XML_CELL_ADDRESS ):
        OSL_ENSURE(!nID, "a action with a ID should not contain a BigRange");
        pContext = new ScXMLBigRangeContext(GetScImport(), pAttribList, aBigRange);
        break;
    }

    return pContext;
}

void SAL_CALL ScXMLCellContentDeletionContext::endFastElement( sal_Int32 /*nElement*/ )
{
    std::unique_ptr<ScMyCellInfo> pCellInfo(new ScMyCellInfo(maCell, sFormulaAddress, sFormula, eGrammar, sInputString, fValue, nType,
            nMatrixFlag, nMatrixCols, nMatrixRows));
    if (nID)
        pChangeTrackingImportHelper->AddDeleted(nID, std::move(pCellInfo));
    else
        pChangeTrackingImportHelper->AddGenerated(std::move(pCellInfo), aBigRange);
}

ScXMLDependenceContext::ScXMLDependenceContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                            ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport )
{
    sal_uInt32 nID(0);
    if ( rAttrList.is() )
    {
        auto aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_ID ) ) );
        if (aIter != rAttrList->end())
            nID = ScXMLChangeTrackingImportHelper::GetIDFromString(aIter.toView());
    }
    pChangeTrackingImportHelper->AddDependence(nID);
}

ScXMLDependingsContext::ScXMLDependingsContext(  ScXMLImport& rImport,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDependingsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    // #i80033# read both old (dependence) and new (dependency) elements
    if (nElement == XML_ELEMENT( TABLE, XML_DEPENDENCE ) ||
        nElement == XML_ELEMENT( TABLE, XML_DEPENDENCY ))
    {
        pContext = new ScXMLDependenceContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
    }

    return pContext;
}

ScXMLChangeDeletionContext::ScXMLChangeDeletionContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport )
{
    sal_uInt32 nID(0);
    if ( rAttrList.is() )
    {
        auto aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_ID ) ) );
        if (aIter != rAttrList->end())
            nID = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
    }
    pChangeTrackingImportHelper->AddDeleted(nID);
}

ScXMLDeletionsContext::ScXMLDeletionsContext(  ScXMLImport& rImport,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDeletionsContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( TABLE, XML_CHANGE_DELETION ):
        pContext = new ScXMLChangeDeletionContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_CELL_CONTENT_DELETION ):
        pContext = new ScXMLCellContentDeletionContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    }

    return pContext;
}

ScXMLChangeTextPContext::ScXMLChangeTextPContext( ScXMLImport& rImport,
                                      sal_Int32 nElement,
                                      const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                                      ScXMLChangeCellContext* pTempChangeCellContext) :
    ScXMLImportContext( rImport ),
    mxAttrList(xAttrList),
    mnElement(nElement),
    sText(),
    pChangeCellContext(pTempChangeCellContext)
{
    // here are no attributes
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLChangeTextPContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    uno::Reference< xml::sax::XFastContextHandler > xContext;
    if (IsTokenInNamespace(mnElement, XML_NAMESPACE_TEXT)
        && (nElement & TOKEN_MASK) == XML_S
        && !pTextPContext)
    {
        sal_Int32 nRepeat(0);
        for( auto& aIter : sax_fastparser::castToFastAttributeList(mxAttrList) )
        {
            if (aIter.getToken() == XML_ELEMENT(TEXT, XML_C))
                nRepeat = aIter.toInt32();
            else
                XMLOFF_WARN_UNKNOWN("sc", aIter);
        }
        if (nRepeat)
            for (sal_Int32 j = 0; j < nRepeat; ++j)
                sText.append(' ');
        else
            sText.append(' ');
    }
    else
    {
        if (!pChangeCellContext->IsEditCell())
            pChangeCellContext->CreateTextPContext(false);
        bool bWasContext (true);
        if (!pTextPContext)
        {
            bWasContext = false;
            pTextPContext = GetScImport().GetTextImport()->CreateTextChildContext(
                                    GetScImport(), mnElement, mxAttrList);
        }
        if (pTextPContext)
        {
            if (!bWasContext)
                pTextPContext->characters(sText.makeStringAndClear());
            xContext = pTextPContext->createFastChildContext(nElement, xAttrList);
        }
    }

    return xContext;
}

void ScXMLChangeTextPContext::characters( const OUString& rChars )
{
    if (!pTextPContext)
        sText.append(rChars);
    else
        pTextPContext->characters(rChars);
}

void ScXMLChangeTextPContext::endFastElement(sal_Int32 /*nElement*/)
{
    if (!pTextPContext)
        pChangeCellContext->SetText(sText.makeStringAndClear());
}

ScXMLChangeCellContext::ScXMLChangeCellContext( ScXMLImport& rImport,
                                                const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                                ScCellValue& rOldCell, OUString& rAddress,
                                                OUString& rFormula, OUString& rFormulaNmsp,
                                                formula::FormulaGrammar::Grammar& rGrammar,
                                                OUString& rTempInputString, double& rDateTimeValue, sal_uInt16& nType,
                                                ScMatrixMode& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows )
    : ScXMLImportContext( rImport )
    , mrOldCell(rOldCell)
    , rInputString(rTempInputString)
    , fValue(0.0)
    , rType(nType)
    , bEmpty(true)
    , bFirstParagraph(true)
    , bString(true)
    , bFormula(false)
{
    bool bIsMatrix(false);
    bool bIsCoveredMatrix(false);
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_FORMULA ):
                bEmpty = false;
                GetScImport().ExtractFormulaNamespaceGrammar( rFormula, rFormulaNmsp, rGrammar, aIter.toString() );
                bFormula = true;
                break;
            case XML_ELEMENT( TABLE, XML_CELL_ADDRESS ):
                rAddress = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_MATRIX_COVERED ):
                bIsCoveredMatrix = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_ELEMENT( TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED ):
                bIsMatrix = true;
                nMatrixCols = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED ):
                bIsMatrix = true;
                nMatrixRows = aIter.toInt32();
                break;
            case XML_ELEMENT( OFFICE, XML_VALUE_TYPE ):
                if (IsXMLToken(aIter, XML_FLOAT))
                    bString = false;
                else if (IsXMLToken(aIter, XML_DATE))
                {
                    rType = css::util::NumberFormat::DATE;
                    bString = false;
                }
                else if (IsXMLToken(aIter, XML_TIME))
                {
                    rType = css::util::NumberFormat::TIME;
                    bString = false;
                }
                break;
            case XML_ELEMENT( OFFICE, XML_VALUE ):
                fValue = aIter.toDouble();
                bEmpty = false;
                break;
            case XML_ELEMENT( OFFICE, XML_DATE_VALUE ):
                bEmpty = false;
                if (GetScImport().GetMM100UnitConverter().setNullDate(GetScImport().GetModel()))
                    GetScImport().GetMM100UnitConverter().convertDateTime(rDateTimeValue, aIter.toString());
                fValue = rDateTimeValue;
                break;
            case XML_ELEMENT( OFFICE, XML_TIME_VALUE ):
                bEmpty = false;
                ::sax::Converter::convertDuration(rDateTimeValue, aIter.toString());
                fValue = rDateTimeValue;
            }
        }
    }

    if (bIsCoveredMatrix)
        nMatrixFlag = ScMatrixMode::Reference;
    else if (bIsMatrix && nMatrixRows && nMatrixCols)
        nMatrixFlag = ScMatrixMode::Formula;
}

uno::Reference< xml::sax::XFastContextHandler > ScXMLChangeCellContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nElement == XML_ELEMENT(TEXT, XML_P))
    {
        bEmpty = false;
        if (bFirstParagraph)
        {
            pContext = new ScXMLChangeTextPContext(GetScImport(), nElement, xAttrList, this);
            bFirstParagraph = false;
        }
        else
        {
            if (!mpEditTextObj.is())
                CreateTextPContext(true);
            pContext = GetScImport().GetTextImport()->CreateTextChildContext(
                GetScImport(), nElement, xAttrList);
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("sc", nElement);

    return pContext;
}

void ScXMLChangeCellContext::CreateTextPContext(bool bIsNewParagraph)
{
    if (!GetScImport().GetDocument())
        return;

    mpEditTextObj = new ScEditEngineTextObj();
    mpEditTextObj->GetEditEngine()->SetEditTextObjectPool(GetScImport().GetDocument()->GetEditPool());
    uno::Reference <text::XText> xText(mpEditTextObj);
    if (xText.is())
    {
        uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor());
        if (bIsNewParagraph)
        {
            xText->setString(sText);
            xTextCursor->gotoEnd(false);
            xText->insertControlCharacter(xTextCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
        }
        GetScImport().GetTextImport()->SetCursor(xTextCursor);
    }
}

void SAL_CALL ScXMLChangeCellContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!bEmpty)
    {
        ScDocument* pDoc = GetScImport().GetDocument();
        if (mpEditTextObj.is())
        {
            if (GetImport().GetTextImport()->GetCursor().is())
            {
                //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
                if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, true ) )
                {
                    GetImport().GetTextImport()->GetText()->insertString(
                        GetImport().GetTextImport()->GetCursorAsRange(), "",
                        true );
                }
            }

            // The cell will own the text object instance.
            mrOldCell.meType = CELLTYPE_EDIT;
            mrOldCell.mpEditText = mpEditTextObj->CreateTextObject().release();
            GetScImport().GetTextImport()->ResetCursor();
            mpEditTextObj.clear();
        }
        else
        {
            if (!bFormula)
            {
                if (!sText.isEmpty() && bString)
                {
                    mrOldCell.meType = CELLTYPE_STRING;
                    mrOldCell.mpString = new svl::SharedString(pDoc->GetSharedStringPool().intern(sText));
                }
                else
                {
                    mrOldCell.meType = CELLTYPE_VALUE;
                    mrOldCell.mfValue = fValue;
                }
                if (rType == css::util::NumberFormat::DATE || rType == css::util::NumberFormat::TIME)
                    rInputString = sText;
            }
        }
    }
    else
        mrOldCell.clear();
}

ScXMLPreviousContext::ScXMLPreviousContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    fValue(0.0),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nID(0),
    nMatrixCols(0),
    nMatrixRows(0),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nType(css::util::NumberFormat::ALL),
    nMatrixFlag(ScMatrixMode::NONE)
{
    if ( rAttrList.is() )
    {
        auto aIter( rAttrList->find( XML_ELEMENT( TABLE, XML_ID ) ) );
        if (aIter != rAttrList->end())
            nID = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
    }
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLPreviousContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    if ( nElement == XML_ELEMENT( TABLE, XML_CHANGE_TRACK_TABLE_CELL ) )
        pContext = new ScXMLChangeCellContext(GetScImport(), pAttribList,
            maOldCell, sFormulaAddress, sFormula, sFormulaNmsp, eGrammar, sInputString, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows);

    return pContext;
}

void SAL_CALL ScXMLPreviousContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->SetPreviousChange(nID, new ScMyCellInfo(maOldCell, sFormulaAddress, sFormula, eGrammar, sInputString,
        fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows));
}

ScXMLContentChangeContext::ScXMLContentChangeContext(  ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_ID ):
                nActionNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_ACCEPTANCE_STATE ):
                if (IsXMLToken( aIter, XML_ACCEPTED ))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken( aIter, XML_REJECTED ))
                    nActionState = SC_CAS_REJECTED;
                break;
            case XML_ELEMENT( TABLE, XML_REJECTING_CHANGE_ID ):
                nRejectingNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_CONTENT);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLContentChangeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( OFFICE, XML_CHANGE_INFO ):
        pContext = new ScXMLChangeInfoContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_CELL_ADDRESS ):
        pContext = new ScXMLBigRangeContext(GetScImport(), pAttribList, aBigRange);
        break;
    case XML_ELEMENT( TABLE, XML_DEPENDENCIES ):
        pContext = new ScXMLDependingsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETIONS ):
        pContext = new ScXMLDeletionsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_PREVIOUS ):
        pContext = new ScXMLPreviousContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    }

    return pContext;
}

void SAL_CALL ScXMLContentChangeContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->SetBigRange(aBigRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionContext::ScXMLInsertionContext( ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nCount(1);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_INSERT_COLS);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_ID ):
                nActionNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_ACCEPTANCE_STATE ):
                if (IsXMLToken( aIter, XML_ACCEPTED ))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken( aIter, XML_REJECTED ))
                    nActionState = SC_CAS_REJECTED;
                break;
            case XML_ELEMENT( TABLE, XML_REJECTING_CHANGE_ID ):
                nRejectingNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_TYPE ):
                if (IsXMLToken( aIter, XML_ROW ))
                    nActionType = SC_CAT_INSERT_ROWS;
                else if (IsXMLToken( aIter, XML_TABLE ))
                    nActionType = SC_CAT_INSERT_TABS;
                break;
            case XML_ELEMENT( TABLE, XML_POSITION ):
                nPosition = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_TABLE ):
                nTable = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_COUNT ):
                nCount = aIter.toInt32();
                break;
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, nCount, nTable);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLInsertionContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( OFFICE, XML_CHANGE_INFO ):
        pContext = new ScXMLChangeInfoContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DEPENDENCIES ):
        pContext = new ScXMLDependingsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETIONS ):
        pContext = new ScXMLDeletionsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    }

    return pContext;
}

void SAL_CALL ScXMLInsertionContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionCutOffContext::ScXMLInsertionCutOffContext( ScXMLImport& rImport,
                                              const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport )
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TABLE, XML_ID):
                nID = ScXMLChangeTrackingImportHelper::GetIDFromString(aIter.toView());
                break;
            case XML_ELEMENT(TABLE, XML_POSITION):
                ::sax::Converter::convertNumber(nPosition, aIter.toView());
                break;
        }
    }
    pChangeTrackingImportHelper->SetInsertionCutOff(nID, nPosition);
}


ScXMLMovementCutOffContext::ScXMLMovementCutOffContext( ScXMLImport& rImport,
                                              const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport )
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    sal_Int32 nStartPosition(0);
    sal_Int32 nEndPosition(0);
    bool bPosition(false);
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TABLE, XML_ID):
                nID = ScXMLChangeTrackingImportHelper::GetIDFromString(aIter.toView());
                break;
            case XML_ELEMENT(TABLE, XML_POSITION):
                bPosition = true;
                ::sax::Converter::convertNumber(nPosition, aIter.toView());
                break;
            case XML_ELEMENT(TABLE, XML_START_POSITION):
                ::sax::Converter::convertNumber(nStartPosition, aIter.toView());
                break;
            case XML_ELEMENT(TABLE, XML_END_POSITION):
                ::sax::Converter::convertNumber(nEndPosition, aIter.toView());
                break;
        }
    }
    if (bPosition)
        nStartPosition = nEndPosition = nPosition;
    pChangeTrackingImportHelper->AddMoveCutOff(nID, nStartPosition, nEndPosition);
}

ScXMLCutOffsContext::ScXMLCutOffsContext( ScXMLImport& rImport,
                                          ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

css::uno::Reference< css::xml::sax::XFastContextHandler > ScXMLCutOffsContext::createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nElement == XML_ELEMENT(TABLE, XML_INSERTION_CUT_OFF))
        pContext = new ScXMLInsertionCutOffContext(GetScImport(), xAttrList, pChangeTrackingImportHelper);
    else if (nElement == XML_ELEMENT(TABLE, XML_MOVEMENT_CUT_OFF))
        pContext = new ScXMLMovementCutOffContext(GetScImport(), xAttrList, pChangeTrackingImportHelper);

    return pContext;
}

ScXMLDeletionContext::ScXMLDeletionContext( ScXMLImport& rImport,
                                            const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nMultiSpanned(0);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_DELETE_COLS);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            sal_Int32 nToken = aIter.getToken();
            switch (nToken)
            {
            case XML_ELEMENT( TABLE, XML_ID ):
                nActionNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_ACCEPTANCE_STATE ):
                if (IsXMLToken( aIter, XML_ACCEPTED ))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken( aIter, XML_REJECTED ))
                    nActionState = SC_CAS_REJECTED;
                break;
            case XML_ELEMENT( TABLE, XML_REJECTING_CHANGE_ID ):
                nRejectingNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_TYPE ):
                if (IsXMLToken( aIter, XML_ROW ))
                    nActionType = SC_CAT_DELETE_ROWS;
                else if (IsXMLToken( aIter, XML_TABLE ))
                    nActionType = SC_CAT_DELETE_TABS;
                break;
            case XML_ELEMENT( TABLE, XML_POSITION ):
                nPosition = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_TABLE ):
                nTable = aIter.toInt32();
                break;
            case XML_ELEMENT( TABLE, XML_MULTI_DELETION_SPANNED ):
                nMultiSpanned = aIter.toInt32();
                break;
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, 1, nTable);
    pChangeTrackingImportHelper->SetMultiSpanned(static_cast<sal_Int16>(nMultiSpanned));
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLDeletionContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( OFFICE, XML_CHANGE_INFO ):
        pContext = new ScXMLChangeInfoContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DEPENDENCIES ):
        pContext = new ScXMLDependingsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETIONS ):
        pContext = new ScXMLDeletionsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_CUT_OFFS ):
    case XML_ELEMENT( TABLE, XML_CUT_OFFS2 ): // cut_offs
            pContext = new ScXMLCutOffsContext(GetScImport(), pChangeTrackingImportHelper);
    }

    return pContext;
}

void SAL_CALL ScXMLDeletionContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLMovementContext::ScXMLMovementContext( ScXMLImport& rImport,
                                            const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_ID ):
                nActionNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_ACCEPTANCE_STATE ):
                if (IsXMLToken( aIter, XML_ACCEPTED ))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken( aIter, XML_REJECTED ))
                    nActionState = SC_CAS_REJECTED;
                break;
            case XML_ELEMENT( TABLE, XML_REJECTING_CHANGE_ID ):
                nRejectingNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLMovementContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( OFFICE, XML_CHANGE_INFO ):
        pContext = new ScXMLChangeInfoContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DEPENDENCIES ):
        pContext = new ScXMLDependingsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETIONS ):
        pContext = new ScXMLDeletionsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_SOURCE_RANGE_ADDRESS ):
        pContext = new ScXMLBigRangeContext(GetScImport(), pAttribList, aSourceRange);
        break;
    case XML_ELEMENT( TABLE, XML_TARGET_RANGE_ADDRESS ):
        pContext = new ScXMLBigRangeContext(GetScImport(), pAttribList, aTargetRange);
        break;
    }

    return pContext;
}

void SAL_CALL ScXMLMovementContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->SetMoveRanges(aSourceRange, aTargetRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLRejectionContext::ScXMLRejectionContext( ScXMLImport& rImport,
                                              const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    ScXMLImportContext( rImport ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( TABLE, XML_ID ):
                nActionNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            case XML_ELEMENT( TABLE, XML_ACCEPTANCE_STATE ):
                if (IsXMLToken( aIter, XML_ACCEPTED ))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken( aIter, XML_REJECTED ))
                    nActionState = SC_CAS_REJECTED;
                break;
            case XML_ELEMENT( TABLE, XML_REJECTING_CHANGE_ID ):
                nRejectingNumber = ScXMLChangeTrackingImportHelper::GetIDFromString( aIter.toView() );
                break;
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLRejectionContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);
    sax_fastparser::FastAttributeList *pAttribList =
        &sax_fastparser::castToFastAttributeList( xAttrList );

    switch (nElement)
    {
    case XML_ELEMENT( OFFICE, XML_CHANGE_INFO ):
        pContext = new ScXMLChangeInfoContext(GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DEPENDENCIES ):
        pContext = new ScXMLDependingsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_DELETIONS ):
        pContext = new ScXMLDeletionsContext(GetScImport(), pChangeTrackingImportHelper);
        break;
    }

    return pContext;
}

void SAL_CALL ScXMLRejectionContext::endFastElement( sal_Int32 /*nElement*/ )
{
    pChangeTrackingImportHelper->EndChangeAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
