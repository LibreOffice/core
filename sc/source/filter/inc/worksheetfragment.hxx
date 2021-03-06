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

#include <memory>
#include "excelhandlers.hxx"

namespace oox::xls {

class DataValidationsContextBase
{
public:
    void SetSqref( const OUString& rChars ) { maSqref = rChars; }
    void SetFormula1( const OUString& rChars ) { maFormula1 = rChars; }
    void SetFormula2( const OUString& rChars ) { maFormula2 = rChars; }
    void SetValidation( WorksheetHelper& rTarget );
    /** Imports the dataValidation element containing data validation settings. */
    void importDataValidation( const AttributeList& rAttribs );
    /** Imports the DATAVALIDATION record containing data validation settings. */
    static void importDataValidation( SequenceInputStream& rStrm, WorksheetHelper& rTarget );
    bool isFormula1Set() const { return !maFormula1.isEmpty(); }
    bool isFormula2Set() const { return !maFormula2.isEmpty(); }

private:
    std::unique_ptr< ValidationModel > mxValModel;

    OUString maSqref;
    OUString maFormula1;
    OUString maFormula2;
};

// For following types of validations:
//
//  <dataValidations count="1">
//    <dataValidation allowBlank="true" operator="equal" showDropDown="false" showErrorMessage="true" showInputMessage="false" sqref="C1:C5" type="list">
//      <formula1>Sheet1!$A$1:$A$5</formula1>
//      <formula2>0</formula2>
//    </dataValidation>
//  </dataValidations>
//
// or
//
//  <dataValidations count="1">
//    <dataValidation type="list" operator="equal" allowBlank="1" showErrorMessage="1" sqref="A1">
//      <mc:AlternateContent xmlns:x12ac="http://schemas.microsoft.com/office/spreadsheetml/2011/1/ac" xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006">
//        <mc:Choice Requires="x12ac">
//          <x12ac:list>1,"2,3",4</x12ac:list>
//        </mc:Choice>
//        <mc:Fallback>
//          <formula1>"1,2,3,4"</formula1>
//        </mc:Fallback>
//      </mc:AlternateContent>
//    </dataValidation>
//  </dataValidations>

class DataValidationsContext : public WorksheetContextBase, private DataValidationsContextBase
{
public:
    explicit            DataValidationsContext( WorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
};

// For following types of validations:
//
//  <extLst>
//    <ext uri="{CCE6A557-97BC-4b89-ADB6-D9C93CAAB3DF}" xmlns:x14="http://schemas.microsoft.com/office/spreadsheetml/2009/9/main">
//      <x14:dataValidations count="1" xmlns:xm="http://schemas.microsoft.com/office/excel/2006/main">
//        <x14:dataValidation type="list" allowBlank="1" showInputMessage="1" showErrorMessage="1">
//          <x14:formula1>
//            <xm:f>Sheet1!$A$2:$A$272</xm:f>
//          </x14:formula1>
//          <xm:sqref>A6:A22</xm:sqref>
//        </x14:dataValidation>
//      </x14:dataValidations>
//    </ext>
//  </extLst>

class ExtDataValidationsContext : public WorksheetContextBase, private DataValidationsContextBase
{
public:
    explicit            ExtDataValidationsContext( WorksheetContextBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;
private:
    sal_Int32 mCurrFormula;
};

class WorksheetFragment : public WorksheetFragmentBase
{
public:
    explicit            WorksheetFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;
    virtual void        initializeImport() override;
    virtual void        finalizeImport() override;

private:
    /** Imports page settings from a pageSetUpPr element. */
    void                importPageSetUpPr( const AttributeList& rAttribs );
    /** Imports the dimension element containing the used area of the sheet. */
    void                importDimension( const AttributeList& rAttribs );
    /** Imports sheet format properties from a sheetFormatPr element. */
    void                importSheetFormatPr( const AttributeList& rAttribs );
    /** Imports column settings from a col element. */
    void                importCol( const AttributeList& rAttribs );
    /** Imports a merged cell range from a mergeCell element. */
    void                importMergeCell( const AttributeList& rAttribs );
    /** Imports the hyperlink element containing a hyperlink for a cell range. */
    void                importHyperlink( const AttributeList& rAttribs );
    /** Imports individual break that is either within row or column break context. */
    void                importBrk( const AttributeList& rAttribs, bool bRowBreak );
    /** Imports the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the relation identifier for the legacy VML drawing part. */
    void                importLegacyDrawing( const AttributeList& rAttribs );
    /** Imports additional data for an OLE object. */
    void                importOleObject( const AttributeList& rAttribs );
    /** Imports additional data for an OCX form control. */
    void                importControl( const AttributeList& rAttribs );

    /** Imports the DIMENSION record containing the used area of the sheet. */
    void                importDimension( SequenceInputStream& rStrm );
    /** Imports sheet format properties from a SHEETFORMATPR record. */
    void                importSheetFormatPr( SequenceInputStream& rStrm );
    /** Imports column settings from a COL record. */
    void                importCol( SequenceInputStream& rStrm );
    /** Imports a merged cell range from a MERGECELL record. */
    void                importMergeCell( SequenceInputStream& rStrm );
    /** Imports a hyperlink for a cell range from a HYPERLINK record. */
    void                importHyperlink( SequenceInputStream& rStrm );
    /** Imports the BRK record for an individual row or column page break. */
    void                importBrk( SequenceInputStream& rStrm, bool bRowBreak );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( SequenceInputStream& rStrm );
    /** Imports the LEGACYDRAWING record containing the relation identifier for the VML drawing part. */
    void                importLegacyDrawing( SequenceInputStream& rStrm );
    /** Imports additional data for an OLE object. */
    void                importOleObject( SequenceInputStream& rStrm );
    /** Imports additional data for an OCX form control. */
    void                importControl( SequenceInputStream& rStrm );

    /** Imports the binary data of an embedded OLE object from the fragment with the passed ID. */
    void                importEmbeddedOleData( StreamDataSequence& orEmbeddedData, const OUString& rRelId );
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
