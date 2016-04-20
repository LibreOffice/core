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

#include "XMLTrackedChangesContext.hxx"
#include "xmlimprt.hxx"
#include "xmlconti.hxx"
#include "XMLConverter.hxx"
#include "formulacell.hxx"
#include "textuno.hxx"
#include "editutil.hxx"
#include "document.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

class ScXMLChangeInfoContext : public SvXMLImportContext
{
    ScMyActionInfo                      aInfo;
    OUStringBuffer               sAuthorBuffer;
    OUStringBuffer               sDateTimeBuffer;
    OUStringBuffer               sCommentBuffer;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    sal_uInt32                          nParagraphCount;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLChangeInfoContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeInfoContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLBigRangeContext : public SvXMLImportContext
{
    ScBigRange&         rBigRange;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLBigRangeContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScBigRange& rBigRange);
    virtual ~ScXMLBigRangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLCellContentDeletionContext : public SvXMLImportContext
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
    formula::FormulaGrammar::Grammar                  eGrammar;
    sal_uInt16                          nType;
    sal_uInt8                           nMatrixFlag;
    bool                            bBigRange;
    bool                            bContainsCell;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLCellContentDeletionContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCellContentDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDependenceContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDependenceContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependenceContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDependingsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDependingsContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDependingsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLChangeDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLChangeDeletionContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLChangeDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLDeletionsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDeletionsContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLChangeCellContext;

class ScXMLChangeTextPContext : public SvXMLImportContext
{
    css::uno::Reference< css::xml::sax::XAttributeList> xAttrList;
    OUString                    sLName;
    OUStringBuffer              sText;
    ScXMLChangeCellContext*     pChangeCellContext;
    SvXMLImportContext*         pTextPContext;
    sal_uInt16                  nPrefix;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:

    ScXMLChangeTextPContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                       const OUString& rLName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                        ScXMLChangeCellContext* pChangeCellContext);

    virtual ~ScXMLChangeTextPContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void Characters( const OUString& rChars ) override;

    virtual void EndElement() override;
};

class ScXMLChangeCellContext : public SvXMLImportContext
{
    ScCellValue& mrOldCell;

    OUString           sText;
    OUString&          rInputString;
    ScEditEngineTextObj*    pEditTextObj;
    double&                 rDateTimeValue;
    double                  fValue;
    sal_uInt16&             rType;
    bool                bEmpty;
    bool                bFirstParagraph;
    bool                bString;
    bool                bFormula;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLChangeCellContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScCellValue& rOldCell, OUString& sAddress,
                                      OUString& rFormula, OUString& rFormulaNmsp,
                                      formula::FormulaGrammar::Grammar& rGrammar,
                                      OUString& rInputString, double& fValue, sal_uInt16& nType,
                                      sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows);
    virtual ~ScXMLChangeCellContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    void CreateTextPContext(bool bIsNewParagraph);
    bool IsEditCell() { return pEditTextObj != nullptr; }
    void SetText(const OUString& sTempText) { sText = sTempText; }

    virtual void EndElement() override;
};

class ScXMLPreviousContext : public SvXMLImportContext
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
    sal_uInt8                           nMatrixFlag;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLPreviousContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLPreviousContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class ScXMLContentChangeContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;
    ScBigRange                          aBigRange;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLContentChangeContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLContentChangeContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLInsertionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLInsertionContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLInsertionCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLInsertionCutOffContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLInsertionCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLMovementCutOffContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLMovementCutOffContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementCutOffContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLCutOffsContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLCutOffsContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLCutOffsContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLDeletionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLDeletionContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLDeletionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLMovementContext : public SvXMLImportContext
{
    ScBigRange                          aSourceRange;
    ScBigRange                          aTargetRange;
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLMovementContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLMovementContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

class ScXMLRejectionContext : public SvXMLImportContext
{
    ScXMLChangeTrackingImportHelper*    pChangeTrackingImportHelper;

    ScXMLImport& GetScImport() { return static_cast<ScXMLImport&>(GetImport()); }

public:
    ScXMLRejectionContext( ScXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList,
                                      ScXMLChangeTrackingImportHelper* pChangeTrackingImportHelper);
    virtual ~ScXMLRejectionContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;
};

ScXMLTrackedChangesContext::ScXMLTrackedChangesContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    rImport.LockSolarMutex();
    pChangeTrackingImportHelper->SetChangeTrack(true);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));
        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_PROTECTION_KEY))
            {
                if (!sValue.isEmpty())
                {
                    uno::Sequence<sal_Int8> aPass;
                    ::sax::Converter::decodeBase64(aPass, sValue);
                    pChangeTrackingImportHelper->SetProtection(aPass);
                }
            }
        }
    }
}

ScXMLTrackedChangesContext::~ScXMLTrackedChangesContext()
{
    GetScImport().UnlockSolarMutex();
}

SvXMLImportContext *ScXMLTrackedChangesContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CELL_CONTENT_CHANGE))
        {
            pContext = new ScXMLContentChangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_INSERTION))
        {
            pContext = new ScXMLInsertionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_DELETION))
        {
            pContext = new ScXMLDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_MOVEMENT))
        {
            pContext = new ScXMLMovementContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_REJECTION))
        {
            pContext = new ScXMLRejectionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLTrackedChangesContext::EndElement()
{
}

ScXMLChangeInfoContext::ScXMLChangeInfoContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                              ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aInfo(),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nParagraphCount(0)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken(aLocalName, XML_CHG_AUTHOR))
            {
                sAuthorBuffer = sValue;
            }
            else if (IsXMLToken(aLocalName, XML_CHG_DATE_TIME))
            {
                sDateTimeBuffer = sValue;
            }
        }
    }
}

ScXMLChangeInfoContext::~ScXMLChangeInfoContext()
{
}

SvXMLImportContext *ScXMLChangeInfoContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CREATOR ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLocalName, xAttrList, sAuthorBuffer);
        else if( IsXMLToken( rLocalName, XML_DATE ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLocalName, xAttrList, sDateTimeBuffer);
    }
    else if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLocalName, XML_P)) )
    {
        if(nParagraphCount)
            sCommentBuffer.append('\n');
        ++nParagraphCount;
        pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLocalName, xAttrList, sCommentBuffer);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeInfoContext::EndElement()
{
    aInfo.sUser = sAuthorBuffer.makeStringAndClear();
    ::sax::Converter::parseDateTime(aInfo.aDateTime, nullptr,
            sDateTimeBuffer.makeStringAndClear());
    aInfo.sComment = sCommentBuffer.makeStringAndClear();
    pChangeTrackingImportHelper->SetActionInfo(aInfo);
}

ScXMLBigRangeContext::ScXMLBigRangeContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScBigRange& rTempBigRange ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rBigRange(rTempBigRange)
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
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_COLUMN))
            {
                ::sax::Converter::convertNumber(nColumn, sValue);
                bColumn = true;
            }
            else if (IsXMLToken(aLocalName, XML_ROW))
            {
                ::sax::Converter::convertNumber(nRow, sValue);
                bRow = true;
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                ::sax::Converter::convertNumber(nTable, sValue);
                bTable = true;
            }
            else if (IsXMLToken(aLocalName, XML_START_COLUMN))
                ::sax::Converter::convertNumber(nStartColumn, sValue);
            else if (IsXMLToken(aLocalName, XML_END_COLUMN))
                ::sax::Converter::convertNumber(nEndColumn, sValue);
            else if (IsXMLToken(aLocalName, XML_START_ROW))
                ::sax::Converter::convertNumber(nStartRow, sValue);
            else if (IsXMLToken(aLocalName, XML_END_ROW))
                ::sax::Converter::convertNumber(nEndRow, sValue);
            else if (IsXMLToken(aLocalName, XML_START_TABLE))
                ::sax::Converter::convertNumber(nStartTable, sValue);
            else if (IsXMLToken(aLocalName, XML_END_TABLE))
                ::sax::Converter::convertNumber(nEndTable, sValue);
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

ScXMLBigRangeContext::~ScXMLBigRangeContext()
{
}

SvXMLImportContext *ScXMLBigRangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLBigRangeContext::EndElement()
{
}

ScXMLCellContentDeletionContext::ScXMLCellContentDeletionContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    fValue(0.0),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nID(0),
    nMatrixCols(0),
    nMatrixRows(0),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nType(css::util::NumberFormat::ALL),
    nMatrixFlag(MM_NONE),
    bBigRange(false),
    bContainsCell(false)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLCellContentDeletionContext::~ScXMLCellContentDeletionContext()
{
}

SvXMLImportContext *ScXMLCellContentDeletionContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CHANGE_TRACK_TABLE_CELL))
        {
            bContainsCell = true;
            pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
                maCell, sFormulaAddress, sFormula, sFormulaNmsp, eGrammar, sInputString, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows );
        }
        else if (IsXMLToken(rLocalName, XML_CELL_ADDRESS))
        {
            OSL_ENSURE(!nID, "a action with a ID should not contain a BigRange");
            bBigRange = true;
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCellContentDeletionContext::EndElement()
{
    ScMyCellInfo* pCellInfo(new ScMyCellInfo(maCell, sFormulaAddress, sFormula, eGrammar, sInputString, fValue, nType,
            nMatrixFlag, nMatrixCols, nMatrixRows));
    if (nID)
        pChangeTrackingImportHelper->AddDeleted(nID, pCellInfo);
    else
        pChangeTrackingImportHelper->AddGenerated(pCellInfo, aBigRange);
}

ScXMLDependenceContext::ScXMLDependenceContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nID(0);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDependence(nID);
}

ScXMLDependenceContext::~ScXMLDependenceContext()
{
}

SvXMLImportContext *ScXMLDependenceContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLDependenceContext::EndElement()
{
}

ScXMLDependingsContext::ScXMLDependingsContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                            const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

ScXMLDependingsContext::~ScXMLDependingsContext()
{
}

SvXMLImportContext *ScXMLDependingsContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        // #i80033# read both old (dependence) and new (dependency) elements
        if (IsXMLToken(rLocalName, XML_DEPENDENCE) || IsXMLToken(rLocalName, XML_DEPENDENCY))
            pContext = new ScXMLDependenceContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDependingsContext::EndElement()
{
}

ScXMLChangeDeletionContext::ScXMLChangeDeletionContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nID(0);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
    pChangeTrackingImportHelper->AddDeleted(nID);
}

ScXMLChangeDeletionContext::~ScXMLChangeDeletionContext()
{
}

SvXMLImportContext *ScXMLChangeDeletionContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLChangeDeletionContext::EndElement()
{
}

ScXMLDeletionsContext::ScXMLDeletionsContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                            const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

ScXMLDeletionsContext::~ScXMLDeletionsContext()
{
}

SvXMLImportContext *ScXMLDeletionsContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CHANGE_DELETION))
            pContext = new ScXMLChangeDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_CELL_CONTENT_DELETION))
            pContext = new ScXMLCellContentDeletionContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionsContext::EndElement()
{
}

ScXMLChangeTextPContext::ScXMLChangeTextPContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const css::uno::Reference<css::xml::sax::XAttributeList>& xTempAttrList,
                                      ScXMLChangeCellContext* pTempChangeCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xAttrList(xTempAttrList),
    sLName(rLName),
    sText(),
    pChangeCellContext(pTempChangeCellContext),
    pTextPContext(nullptr),
    nPrefix(nPrfx)
{
    // here are no attributes
}

ScXMLChangeTextPContext::~ScXMLChangeTextPContext()
{
    delete pTextPContext;
}

SvXMLImportContext *ScXMLChangeTextPContext::CreateChildContext( sal_uInt16 nTempPrefix,
                                            const OUString& rLName,
                                            const css::uno::Reference<css::xml::sax::XAttributeList>& xTempAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLName, XML_S)) && !pTextPContext)
    {
        sal_Int32 nRepeat(0);
        sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& sAttrName(xAttrList->getNameByIndex( i ));
            const OUString& sValue(xAttrList->getValueByIndex( i ));
            OUString aLocalName;
            sal_uInt16 nPrfx(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                                sAttrName, &aLocalName ));
            if ((nPrfx == XML_NAMESPACE_TEXT) && (IsXMLToken(aLocalName, XML_C)))
                nRepeat = sValue.toInt32();
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
                                    GetScImport(), nPrefix, sLName, xAttrList);
        }
        if (pTextPContext)
        {
            if (!bWasContext)
                pTextPContext->Characters(sText.makeStringAndClear());
            pContext = pTextPContext->CreateChildContext(nTempPrefix, rLName, xTempAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLChangeTextPContext::Characters( const OUString& rChars )
{
    if (!pTextPContext)
        sText.append(rChars);
    else
        pTextPContext->Characters(rChars);
}

void ScXMLChangeTextPContext::EndElement()
{
    if (!pTextPContext)
        pChangeCellContext->SetText(sText.makeStringAndClear());
}

ScXMLChangeCellContext::ScXMLChangeCellContext( ScXMLImport& rImport,
                                                sal_uInt16 nPrfx,
                                                const OUString& rLName,
                                                const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                                ScCellValue& rOldCell, OUString& rAddress,
                                                OUString& rFormula, OUString& rFormulaNmsp,
                                                formula::FormulaGrammar::Grammar& rGrammar,
                                                OUString& rTempInputString, double& fDateTimeValue, sal_uInt16& nType,
                                                sal_uInt8& nMatrixFlag, sal_Int32& nMatrixCols, sal_Int32& nMatrixRows )
    : SvXMLImportContext( rImport, nPrfx, rLName )
    , mrOldCell(rOldCell)
    , rInputString(rTempInputString)
    , pEditTextObj(nullptr)
    , rDateTimeValue(fDateTimeValue)
    , fValue(0.0)
    , rType(nType)
    , bEmpty(true)
    , bFirstParagraph(true)
    , bString(true)
    , bFormula(false)
{
    bool bIsMatrix(false);
    bool bIsCoveredMatrix(false);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                   sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_FORMULA))
            {
                bEmpty = false;
                GetScImport().ExtractFormulaNamespaceGrammar( rFormula, rFormulaNmsp, rGrammar, sValue );
                bFormula = true;
            }
            else if (IsXMLToken(aLocalName, XML_CELL_ADDRESS))
            {
                rAddress = sValue;
            }
            else if (IsXMLToken(aLocalName, XML_MATRIX_COVERED))
            {
                bIsCoveredMatrix = IsXMLToken(sValue, XML_TRUE);
            }
            else if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_COLUMNS_SPANNED))
            {
                bIsMatrix = true;
                ::sax::Converter::convertNumber(nMatrixCols, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_ROWS_SPANNED))
            {
                bIsMatrix = true;
                ::sax::Converter::convertNumber(nMatrixRows, sValue);
            }
        }
        else if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken(aLocalName, XML_VALUE_TYPE))
            {
                if (IsXMLToken(sValue, XML_FLOAT))
                    bString = false;
                else if (IsXMLToken(sValue, XML_DATE))
                {
                    rType = css::util::NumberFormat::DATE;
                    bString = false;
                }
                else if (IsXMLToken(sValue, XML_TIME))
                {
                    rType = css::util::NumberFormat::TIME;
                    bString = false;
                }
            }
            else if (IsXMLToken(aLocalName, XML_VALUE))
            {
                ::sax::Converter::convertDouble(fValue, sValue);
                bEmpty = false;
            }
            else if (IsXMLToken(aLocalName, XML_DATE_VALUE))
            {
                bEmpty = false;
                if (GetScImport().GetMM100UnitConverter().setNullDate(GetScImport().GetModel()))
                    GetScImport().GetMM100UnitConverter().convertDateTime(rDateTimeValue, sValue);
                fValue = rDateTimeValue;
            }
            else if (IsXMLToken(aLocalName, XML_TIME_VALUE))
            {
                bEmpty = false;
                ::sax::Converter::convertDuration(rDateTimeValue, sValue);
                fValue = rDateTimeValue;
            }
        }
    }
    if (bIsCoveredMatrix)
        nMatrixFlag = MM_REFERENCE;
    else if (bIsMatrix && nMatrixRows && nMatrixCols)
        nMatrixFlag = MM_FORMULA;
}

ScXMLChangeCellContext::~ScXMLChangeCellContext()
{
}

SvXMLImportContext *ScXMLChangeCellContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_TEXT) && (IsXMLToken(rLocalName, XML_P)))
    {
        bEmpty = false;
        if (bFirstParagraph)
        {
            pContext = new ScXMLChangeTextPContext(GetScImport(), nPrefix, rLocalName, xAttrList, this);
            bFirstParagraph = false;
        }
        else
        {
            if (!pEditTextObj)
                CreateTextPContext(true);
            pContext = GetScImport().GetTextImport()->CreateTextChildContext(
                GetScImport(), nPrefix, rLocalName, xAttrList);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLChangeCellContext::CreateTextPContext(bool bIsNewParagraph)
{
    if (GetScImport().GetDocument())
    {
        pEditTextObj = new ScEditEngineTextObj();
        pEditTextObj->acquire();
        pEditTextObj->GetEditEngine()->SetEditTextObjectPool(GetScImport().GetDocument()->GetEditPool());
        uno::Reference <text::XText> xText(pEditTextObj);
        if (xText.is())
        {
            uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor());
            if (bIsNewParagraph)
            {
                xText->setString(sText);
                xTextCursor->gotoEnd(false);
                uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
                if (xTextRange.is())
                    xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, false);
            }
            GetScImport().GetTextImport()->SetCursor(xTextCursor);
        }
    }
}

void ScXMLChangeCellContext::EndElement()
{
    if (!bEmpty)
    {
        ScDocument* pDoc = GetScImport().GetDocument();
        if (pEditTextObj)
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
            mrOldCell.mpEditText = pEditTextObj->CreateTextObject();
            GetScImport().GetTextImport()->ResetCursor();
            pEditTextObj->release();
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
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    fValue(0.0),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper),
    nID(0),
    nMatrixCols(0),
    nMatrixRows(0),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nType(css::util::NumberFormat::ALL),
    nMatrixFlag(MM_NONE)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
        }
    }
}

ScXMLPreviousContext::~ScXMLPreviousContext()
{
}

SvXMLImportContext *ScXMLPreviousContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_TABLE) && (IsXMLToken(rLocalName, XML_CHANGE_TRACK_TABLE_CELL)))
        pContext = new ScXMLChangeCellContext(GetScImport(), nPrefix, rLocalName, xAttrList,
            maOldCell, sFormulaAddress, sFormula, sFormulaNmsp, eGrammar, sInputString, fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLPreviousContext::EndElement()
{
    pChangeTrackingImportHelper->SetPreviousChange(nID, new ScMyCellInfo(maOldCell, sFormulaAddress, sFormula, eGrammar, sInputString,
        fValue, nType, nMatrixFlag, nMatrixCols, nMatrixRows));
}

ScXMLContentChangeContext::ScXMLContentChangeContext(  ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_CONTENT);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLContentChangeContext::~ScXMLContentChangeContext()
{
}

SvXMLImportContext *ScXMLContentChangeContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_CELL_ADDRESS))
        {
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aBigRange);
        }
        else if (IsXMLToken(rLocalName, XML_DEPENDENCIES))
        {
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_PREVIOUS))
        {
            pContext = new ScXMLPreviousContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLContentChangeContext::EndElement()
{
    pChangeTrackingImportHelper->SetBigRange(aBigRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionContext::ScXMLInsertionContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nCount(1);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_INSERT_COLS);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TYPE))
            {
                if (IsXMLToken(sValue, XML_ROW))
                    nActionType = SC_CAT_INSERT_ROWS;
                else if (IsXMLToken(sValue, XML_TABLE))
                    nActionType = SC_CAT_INSERT_TABS;
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                ::sax::Converter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                ::sax::Converter::convertNumber(nTable, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_COUNT))
            {
                ::sax::Converter::convertNumber(nCount, sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(nActionType);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
    pChangeTrackingImportHelper->SetPosition(nPosition, nCount, nTable);
}

ScXMLInsertionContext::~ScXMLInsertionContext()
{
}

SvXMLImportContext *ScXMLInsertionContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCIES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLInsertionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLInsertionCutOffContext::ScXMLInsertionCutOffContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                ::sax::Converter::convertNumber(nPosition, sValue);
            }
        }
    }
    pChangeTrackingImportHelper->SetInsertionCutOff(nID, nPosition);
}

ScXMLInsertionCutOffContext::~ScXMLInsertionCutOffContext()
{
}

SvXMLImportContext *ScXMLInsertionCutOffContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLInsertionCutOffContext::EndElement()
{
}

ScXMLMovementCutOffContext::ScXMLMovementCutOffContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nID(0);
    sal_Int32 nPosition(0);
    sal_Int32 nStartPosition(0);
    sal_Int32 nEndPosition(0);
    bool bPosition(false);
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nID = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                bPosition = true;
                ::sax::Converter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_START_POSITION))
            {
                ::sax::Converter::convertNumber(nStartPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_END_POSITION))
            {
                ::sax::Converter::convertNumber(nEndPosition, sValue);
            }
        }
    }
    if (bPosition)
        nStartPosition = nEndPosition = nPosition;
    pChangeTrackingImportHelper->AddMoveCutOff(nID, nStartPosition, nEndPosition);
}

ScXMLMovementCutOffContext::~ScXMLMovementCutOffContext()
{
}

SvXMLImportContext *ScXMLMovementCutOffContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void ScXMLMovementCutOffContext::EndElement()
{
}

ScXMLCutOffsContext::ScXMLCutOffsContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                            const uno::Reference<xml::sax::XAttributeList>& /* xAttrList */,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    // here are no attributes
}

ScXMLCutOffsContext::~ScXMLCutOffsContext()
{
}

SvXMLImportContext *ScXMLCutOffsContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_INSERTION_CUT_OFF))
            pContext = new ScXMLInsertionCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_MOVEMENT_CUT_OFF))
            pContext = new ScXMLMovementCutOffContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLCutOffsContext::EndElement()
{
}

ScXMLDeletionContext::ScXMLDeletionContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    sal_Int32 nPosition(0);
    sal_Int32 nMultiSpanned(0);
    sal_Int32 nTable(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);
    ScChangeActionType nActionType(SC_CAT_DELETE_COLS);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TYPE))
            {
                if (IsXMLToken(sValue, XML_ROW))
                {
                    nActionType = SC_CAT_DELETE_ROWS;
                }
                else if (IsXMLToken(aLocalName, XML_TABLE))
                {
                    nActionType = SC_CAT_DELETE_TABS;
                }
            }
            else if (IsXMLToken(aLocalName, XML_POSITION))
            {
                ::sax::Converter::convertNumber(nPosition, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_TABLE))
            {
                ::sax::Converter::convertNumber(nTable, sValue);
            }
            else if (IsXMLToken(aLocalName, XML_MULTI_DELETION_SPANNED))
            {
                ::sax::Converter::convertNumber(nMultiSpanned, sValue);
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

ScXMLDeletionContext::~ScXMLDeletionContext()
{
}

SvXMLImportContext *ScXMLDeletionContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCIES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_CUT_OFFS) || rLocalName == "cut_offs")
            pContext = new ScXMLCutOffsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else
        {
            OSL_FAIL("don't know this");
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLDeletionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLMovementContext::ScXMLMovementContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLMovementContext::~ScXMLMovementContext()
{
}

SvXMLImportContext *ScXMLMovementContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCIES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_SOURCE_RANGE_ADDRESS))
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aSourceRange);
        else if (IsXMLToken(rLocalName, XML_TARGET_RANGE_ADDRESS))
            pContext = new ScXMLBigRangeContext(GetScImport(), nPrefix, rLocalName, xAttrList, aTargetRange);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLMovementContext::EndElement()
{
    pChangeTrackingImportHelper->SetMoveRanges(aSourceRange, aTargetRange);
    pChangeTrackingImportHelper->EndChangeAction();
}

ScXMLRejectionContext::ScXMLRejectionContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ScXMLChangeTrackingImportHelper* pTempChangeTrackingImportHelper ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(pTempChangeTrackingImportHelper)
{
    sal_uInt32 nActionNumber(0);
    sal_uInt32 nRejectingNumber(0);
    ScChangeActionState nActionState(SC_CAS_VIRGIN);

    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_ID))
            {
                nActionNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
            else if (IsXMLToken(aLocalName, XML_ACCEPTANCE_STATE))
            {
                if (IsXMLToken(sValue, XML_ACCEPTED))
                    nActionState = SC_CAS_ACCEPTED;
                else if (IsXMLToken(sValue, XML_REJECTED))
                    nActionState = SC_CAS_REJECTED;
            }
            else if (IsXMLToken(aLocalName, XML_REJECTING_CHANGE_ID))
            {
                nRejectingNumber = pChangeTrackingImportHelper->GetIDFromString(sValue);
            }
        }
    }

    pChangeTrackingImportHelper->StartChangeAction(SC_CAT_MOVE);
    pChangeTrackingImportHelper->SetActionNumber(nActionNumber);
    pChangeTrackingImportHelper->SetActionState(nActionState);
    pChangeTrackingImportHelper->SetRejectingNumber(nRejectingNumber);
}

ScXMLRejectionContext::~ScXMLRejectionContext()
{
}

SvXMLImportContext *ScXMLRejectionContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext(nullptr);

    if ((nPrefix == XML_NAMESPACE_OFFICE) && (IsXMLToken(rLocalName, XML_CHANGE_INFO)))
    {
        pContext = new ScXMLChangeInfoContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    else if (nPrefix == XML_NAMESPACE_TABLE)
    {
        if (IsXMLToken(rLocalName, XML_DEPENDENCIES))
            pContext = new ScXMLDependingsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        else if (IsXMLToken(rLocalName, XML_DELETIONS))
            pContext = new ScXMLDeletionsContext(GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLRejectionContext::EndElement()
{
    pChangeTrackingImportHelper->EndChangeAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
