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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCHANGETRACKINGEXPORTHELPER_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCHANGETRACKINGEXPORTHELPER_HXX

#include <xmloff/xmltoken.hxx>
#include <list>
#include <map>
#include <memory>
#include <tools/solar.h>
#include <com/sun/star/text/XText.hpp>
#include <rtl/ustrbuf.hxx>

class ScChangeAction;
class ScChangeTrack;
class ScXMLExport;
struct ScCellValue;
class ScChangeActionDel;
class ScBigRange;
class ScEditEngineTextObj;

typedef std::map<sal_uLong, ScChangeAction*> ScChangeActionMap;

class ScChangeTrackingExportHelper
{
    ScXMLExport&    rExport;

    ScChangeTrack*  pChangeTrack;
    ScEditEngineTextObj* pEditTextObj;
    std::unique_ptr<ScChangeActionMap> pDependings;
    css::uno::Reference<css::text::XText> xText;

    static OUString GetChangeID(const sal_uInt32 nActionNumber);
    void GetAcceptanceState(const ScChangeAction* pAction);

    void WriteBigRange(const ScBigRange& rBigRange, xmloff::token::XMLTokenEnum aName);
    void WriteChangeInfo(const ScChangeAction* pAction);
    void WriteGenerated(const ScChangeAction* pDependAction);
    void WriteDeleted(const ScChangeAction* pDependAction);
    void WriteDepending(const ScChangeAction* pDependAction);
    void WriteDependings(ScChangeAction* pAction);

    void WriteEmptyCell();
    void SetValueAttributes(const double& fValue, const OUString& sValue);
    void WriteValueCell(const ScCellValue& rCell, const OUString& sValue);
    void WriteStringCell(const ScCellValue& rCell);
    void WriteEditCell(const ScCellValue& rCell);
    void WriteFormulaCell(const ScCellValue& rCell, const OUString& sValue);
    void WriteCell(const ScCellValue& rCell, const OUString& sValue);

    void WriteContentChange(ScChangeAction* pAction);
    void AddInsertionAttributes(const ScChangeAction* pAction);
    void WriteInsertion(ScChangeAction* pAction);
    void AddDeletionAttributes(const ScChangeActionDel* pAction, const ScChangeActionDel* pLastAction);
    void WriteCutOffs(const ScChangeActionDel* pAction);
    void WriteDeletion(ScChangeAction* pAction);
    void WriteMovement(ScChangeAction* pAction);
    void WriteRejection(ScChangeAction* pAction);

    void CollectCellAutoStyles(const ScCellValue& rCell);
    void CollectActionAutoStyles(ScChangeAction* pAction);
    void WorkWithChangeAction(ScChangeAction* pAction);
public:
    explicit ScChangeTrackingExportHelper(ScXMLExport& rExport);
    ~ScChangeTrackingExportHelper();

    void CollectAutoStyles();
    void CollectAndWriteChanges();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
