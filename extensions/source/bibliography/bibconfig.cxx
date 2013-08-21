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


#include <bibconfig.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;


const char cDataSourceHistory[] = "DataSourceHistory";

Sequence<OUString> BibConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        aNames.realloc(8);
        OUString* pNames = aNames.getArray();
        pNames[0] = "CurrentDataSource/DataSourceName";
        pNames[1] = "CurrentDataSource/Command";
        pNames[2] = "CurrentDataSource/CommandType";
        pNames[3] = "BeamerHeight";
        pNames[4] = "ViewHeight";
        pNames[5] = "QueryText";
        pNames[6] = "QueryField";
        pNames[7] = "ShowColumnAssignmentWarning";
    }
    return aNames;
}

BibConfig::BibConfig() :
    ConfigItem("Office.DataAccess/Bibliography", CONFIG_MODE_DELAYED_UPDATE),
    pMappingsArr(new MappingArray),
    nBeamerSize(0),
    nViewSize(0),
    bShowColumnAssignmentWarning(sal_False)
{
    //Names of the default columns
    aColumnDefaults[0] = "Identifier";
    aColumnDefaults[1] = "BibliographyType";
    aColumnDefaults[2] = "Author";
    aColumnDefaults[3] = "Title";
    aColumnDefaults[4] = "Year";
    aColumnDefaults[5] = "ISBN";
    aColumnDefaults[6] = "Booktitle";
    aColumnDefaults[7] = "Chapter";
    aColumnDefaults[8] = "Edition";
    aColumnDefaults[9] = "Editor";
    aColumnDefaults[10] = "Howpublished";
    aColumnDefaults[11] = "Institution";
    aColumnDefaults[12] = "Journal";
    aColumnDefaults[13] = "Month";
    aColumnDefaults[14] = "Note";
    aColumnDefaults[15] = "Annote";
    aColumnDefaults[16] = "Number";
    aColumnDefaults[17] = "Organizations";
    aColumnDefaults[18] = "Pages";
    aColumnDefaults[19] = "Publisher";
    aColumnDefaults[20] = "Address";
    aColumnDefaults[21] = "School";
    aColumnDefaults[22] = "Series";
    aColumnDefaults[23] = "ReportType";
    aColumnDefaults[24] = "Volume";
    aColumnDefaults[25] = "URL";
    aColumnDefaults[26] = "Custom1";
    aColumnDefaults[27] = "Custom2";
    aColumnDefaults[28] = "Custom3";
    aColumnDefaults[29] = "Custom4";
    aColumnDefaults[30] = "Custom5";


    const Sequence< OUString > aPropertyNames = GetPropertyNames();
    const Sequence<Any> aPropertyValues = GetProperties( aPropertyNames );
    const Any* pValues = aPropertyValues.getConstArray();
    if(aPropertyValues.getLength() == aPropertyNames.getLength())
    {
        for(int nProp = 0; nProp < aPropertyNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: pValues[nProp] >>= sDataSource; break;
                    case  1: pValues[nProp] >>= sTableOrQuery; break;
                    case  2: pValues[nProp] >>= nTblOrQuery;  break;
                    case  3: pValues[nProp] >>= nBeamerSize;  break;
                    case  4: pValues[nProp] >>= nViewSize  ;  break;
                    case  5: pValues[nProp] >>= sQueryText ;  break;
                    case  6: pValues[nProp] >>= sQueryField;  break;
                    case  7:
                        bShowColumnAssignmentWarning = *(sal_Bool*)pValues[nProp].getValue();
                    break;
                }
            }
        }
    }
    OUString sName("DataSourceName");
    OUString sTable("Command");
    OUString sCommandType("CommandType");
    Sequence< OUString > aNodeNames = GetNodeNames(cDataSourceHistory);
    const OUString* pNodeNames = aNodeNames.getConstArray();
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        Sequence<OUString> aHistoryNames(3);
        OUString* pHistoryNames = aHistoryNames.getArray();

        OUString sPrefix(cDataSourceHistory);
        sPrefix += "/";
        sPrefix += pNodeNames[nNode];
        sPrefix += "/";
        pHistoryNames[0] = sPrefix;
        pHistoryNames[0] += sName;
        pHistoryNames[1] = sPrefix;
        pHistoryNames[1] += sTable;
        pHistoryNames[2] = sPrefix;
        pHistoryNames[2] += sCommandType;

        Sequence<Any> aHistoryValues = GetProperties( aHistoryNames );
        const Any* pHistoryValues = aHistoryValues.getConstArray();

        if(aHistoryValues.getLength() == aHistoryNames.getLength())
        {
            Mapping* pMapping = new Mapping;
            pHistoryValues[0] >>= pMapping->sURL;
            pHistoryValues[1] >>= pMapping->sTableName;
            pHistoryValues[2] >>= pMapping->nCommandType;
            //field assignment is contained in another set
            sPrefix += "Fields";
            Sequence< OUString > aAssignmentNodeNames = GetNodeNames(sPrefix);
            const OUString* pAssignmentNodeNames = aAssignmentNodeNames.getConstArray();
            Sequence<OUString> aAssignmentPropertyNames(aAssignmentNodeNames.getLength() * 2);
            OUString* pAssignmentPropertyNames = aAssignmentPropertyNames.getArray();
            sal_Int16 nFieldIdx = 0;
            for(sal_Int16 nField = 0; nField < aAssignmentNodeNames.getLength(); nField++)
            {
                OUString sSubPrefix(sPrefix);
                sSubPrefix += "/";
                sSubPrefix += pAssignmentNodeNames[nField];
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++] += "/ProgrammaticFieldName";
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++]   += "/AssignedFieldName";
            }
            Sequence<Any> aAssignmentValues = GetProperties(aAssignmentPropertyNames);
            const Any* pAssignmentValues = aAssignmentValues.getConstArray();
            OUString sTempLogical;
            OUString sTempReal;
            sal_Int16 nSetMapping = 0;
            nFieldIdx = 0;
            for(sal_Int16 nFieldVal = 0; nFieldVal < aAssignmentValues.getLength() / 2; nFieldVal++)
            {
                pAssignmentValues[nFieldIdx++] >>= sTempLogical;
                pAssignmentValues[nFieldIdx++] >>= sTempReal;
                if(!(sTempLogical.isEmpty() || sTempReal.isEmpty()))
                {
                    pMapping->aColumnPairs[nSetMapping].sLogicalColumnName = sTempLogical;
                    pMapping->aColumnPairs[nSetMapping++].sRealColumnName = sTempReal;
                }
            }
            pMappingsArr->push_back(pMapping);
        }
    }
}

BibConfig::~BibConfig()
{
    if(IsModified())
        Commit();
    delete pMappingsArr;
}

BibDBDescriptor BibConfig::GetBibliographyURL()
{
    BibDBDescriptor aRet;
    aRet.sDataSource = sDataSource;
    aRet.sTableOrQuery = sTableOrQuery;
    aRet.nCommandType = nTblOrQuery;
    return aRet;
};

void BibConfig::SetBibliographyURL(const BibDBDescriptor& rDesc)
{
    sDataSource = rDesc.sDataSource;
    sTableOrQuery = rDesc.sTableOrQuery;
    nTblOrQuery = rDesc.nCommandType;
    SetModified();
};
//---------------------------------------------------------------------------
void BibConfig::Notify( const com::sun::star::uno::Sequence<OUString>& )
{
}

void    BibConfig::Commit()
{
    const Sequence<OUString> aPropertyNames = GetPropertyNames();
    Sequence<Any> aValues(aPropertyNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aPropertyNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= sDataSource; break;
            case  1: pValues[nProp] <<= sTableOrQuery; break;
            case  2: pValues[nProp] <<= nTblOrQuery;  break;
            case  3: pValues[nProp] <<= nBeamerSize;  break;
            case  4: pValues[nProp] <<= nViewSize;  break;
            case  5: pValues[nProp] <<= sQueryText;  break;
            case  6: pValues[nProp] <<= sQueryField;  break;
            case  7:
                pValues[nProp].setValue(&bShowColumnAssignmentWarning, ::getBooleanCppuType());
            break;
        }
    }
    PutProperties(aPropertyNames, aValues);
    ClearNodeSet(cDataSourceHistory);
    Sequence< PropertyValue > aNodeValues(pMappingsArr->size() * 3);
    PropertyValue* pNodeValues = aNodeValues.getArray();

    sal_Int32 nIndex = 0;
    OUString sName("DataSourceName");
    OUString sTable("Command");
    OUString sCommandType("CommandType");
    for(sal_Int32 i = 0; i < (sal_Int32)pMappingsArr->size(); i++)
    {
        const Mapping* pMapping = &(*pMappingsArr)[i];
        OUString sPrefix(cDataSourceHistory);
        sPrefix += "/_";
        sPrefix += OUString::number(i);
        sPrefix += "/";
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sName;
        pNodeValues[nIndex++].Value <<= pMapping->sURL;
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sTable;
        pNodeValues[nIndex++].Value <<= pMapping->sTableName;
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sCommandType;
        pNodeValues[nIndex++].Value <<= pMapping->nCommandType;
        SetSetProperties(cDataSourceHistory, aNodeValues);

        sPrefix += "Fields";
        sal_Int32 nFieldAssignment = 0;
        OUString sFieldName = "/ProgrammaticFieldName";
        OUString sDatabaseFieldName = "/AssignedFieldName";
        ClearNodeSet( sPrefix );

        while(nFieldAssignment < COLUMN_COUNT &&
            !pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName.isEmpty())
        {
            OUString sSubPrefix(sPrefix);
            sSubPrefix += "/_";
            sSubPrefix += OUString::number(nFieldAssignment);
            Sequence< PropertyValue > aAssignmentValues(2);
            PropertyValue* pAssignmentValues = aAssignmentValues.getArray();
            pAssignmentValues[0].Name   = sSubPrefix;
            pAssignmentValues[0].Name   += sFieldName;
            pAssignmentValues[0].Value <<= pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName;
            pAssignmentValues[1].Name   = sSubPrefix;
            pAssignmentValues[1].Name   += sDatabaseFieldName;
            pAssignmentValues[1].Value <<= pMapping->aColumnPairs[nFieldAssignment].sRealColumnName;
            SetSetProperties( sPrefix, aAssignmentValues );
            nFieldAssignment++;
        }
    }
}

const Mapping*  BibConfig::GetMapping(const BibDBDescriptor& rDesc) const
{
    for(sal_uInt16 i = 0; i < pMappingsArr->size(); i++)
    {
        Mapping& rMapping = (*pMappingsArr)[i];
        sal_Bool bURLEqual = rDesc.sDataSource.equals(rMapping.sURL);
        if(rDesc.sTableOrQuery == rMapping.sTableName && bURLEqual)
            return &rMapping;
    }
    return 0;
}

void BibConfig::SetMapping(const BibDBDescriptor& rDesc, const Mapping* pSetMapping)
{
    for(sal_uInt16 i = 0; i < pMappingsArr->size(); i++)
    {
        Mapping& rMapping = (*pMappingsArr)[i];
        sal_Bool bURLEqual = rDesc.sDataSource.equals(rMapping.sURL);
        if(rDesc.sTableOrQuery == rMapping.sTableName && bURLEqual)
        {
            pMappingsArr->erase(pMappingsArr->begin()+i);
            break;
        }
    }
    Mapping* pNew = new Mapping(*pSetMapping);
    pMappingsArr->push_back(pNew);
    SetModified();
}

DBChangeDialogConfig_Impl::DBChangeDialogConfig_Impl()
{
}

DBChangeDialogConfig_Impl::~DBChangeDialogConfig_Impl()
{
}

const Sequence<OUString>& DBChangeDialogConfig_Impl::GetDataSourceNames()
{
    if(!aSourceNames.getLength())
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);
        aSourceNames = xDBContext->getElementNames();
    }
    return aSourceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
