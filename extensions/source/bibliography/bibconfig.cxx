/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

using ::rtl::OUString;

#define C2U(cChar) OUString::createFromAscii(cChar)

const char* cDataSourceHistory = "DataSourceHistory";

Sequence<OUString> BibConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        aNames.realloc(8);
        OUString* pNames = aNames.getArray();
        pNames[0] = C2U("CurrentDataSource/DataSourceName");
        pNames[1] = C2U("CurrentDataSource/Command");
        pNames[2] = C2U("CurrentDataSource/CommandType");
        pNames[3] = C2U("BeamerHeight");
        pNames[4] = C2U("ViewHeight");
        pNames[5] = C2U("QueryText");
        pNames[6] = C2U("QueryField");
        pNames[7] = C2U("ShowColumnAssignmentWarning");
    }
    return aNames;
}

BibConfig::BibConfig() :
    ConfigItem(C2U("Office.DataAccess/Bibliography"), CONFIG_MODE_DELAYED_UPDATE),
    pMappingsArr(new MappingArray),
    nBeamerSize(0),
    nViewSize(0),
    bShowColumnAssignmentWarning(sal_False)
{
    //Names of the default columns
    aColumnDefaults[0] = C2U("Identifier");
    aColumnDefaults[1] = C2U("BibliographyType");
    aColumnDefaults[2] = C2U("Author");
    aColumnDefaults[3] = C2U("Title");
    aColumnDefaults[4] = C2U("Year");
    aColumnDefaults[5] = C2U("ISBN");
    aColumnDefaults[6] = C2U("Booktitle");
    aColumnDefaults[7] = C2U("Chapter");
    aColumnDefaults[8] = C2U("Edition");
    aColumnDefaults[9] = C2U("Editor");
    aColumnDefaults[10] = C2U("Howpublished");
    aColumnDefaults[11] = C2U("Institution");
    aColumnDefaults[12] = C2U("Journal");
    aColumnDefaults[13] = C2U("Month");
    aColumnDefaults[14] = C2U("Note");
    aColumnDefaults[15] = C2U("Annote");
    aColumnDefaults[16] = C2U("Number");
    aColumnDefaults[17] = C2U("Organizations");
    aColumnDefaults[18] = C2U("Pages");
    aColumnDefaults[19] = C2U("Publisher");
    aColumnDefaults[20] = C2U("Address");
    aColumnDefaults[21] = C2U("School");
    aColumnDefaults[22] = C2U("Series");
    aColumnDefaults[23] = C2U("ReportType");
    aColumnDefaults[24] = C2U("Volume");
    aColumnDefaults[25] = C2U("URL");
    aColumnDefaults[26] = C2U("Custom1");
    aColumnDefaults[27] = C2U("Custom2");
    aColumnDefaults[28] = C2U("Custom3");
    aColumnDefaults[29] = C2U("Custom4");
    aColumnDefaults[30] = C2U("Custom5");


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
    OUString sName(C2U("DataSourceName"));
    OUString sTable(C2U("Command"));
    OUString sCommandType(C2U("CommandType"));
    Sequence< OUString > aNodeNames = GetNodeNames(C2U(cDataSourceHistory));
    const OUString* pNodeNames = aNodeNames.getConstArray();
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        Sequence<OUString> aHistoryNames(3);
        OUString* pHistoryNames = aHistoryNames.getArray();

        OUString sPrefix(C2U(cDataSourceHistory));
        sPrefix += C2U("/");
        sPrefix += pNodeNames[nNode];
        sPrefix += C2U("/");
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
            sPrefix += C2U("Fields");
            Sequence< OUString > aAssignmentNodeNames = GetNodeNames(sPrefix);
            const OUString* pAssignmentNodeNames = aAssignmentNodeNames.getConstArray();
            Sequence<OUString> aAssignmentPropertyNames(aAssignmentNodeNames.getLength() * 2);
            OUString* pAssignmentPropertyNames = aAssignmentPropertyNames.getArray();
            sal_Int16 nFieldIdx = 0;
            for(sal_Int16 nField = 0; nField < aAssignmentNodeNames.getLength(); nField++)
            {
                OUString sSubPrefix(sPrefix);
                sSubPrefix += C2U("/");
                sSubPrefix += pAssignmentNodeNames[nField];
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++] += C2U("/ProgrammaticFieldName");
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++]   += C2U("/AssignedFieldName");
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
void BibConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& )
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
    ClearNodeSet( C2U(cDataSourceHistory));
    Sequence< PropertyValue > aNodeValues(pMappingsArr->size() * 3);
    PropertyValue* pNodeValues = aNodeValues.getArray();

    sal_Int32 nIndex = 0;
    OUString sName(C2U("DataSourceName"));
    OUString sTable(C2U("Command"));
    OUString sCommandType(C2U("CommandType"));
    for(sal_Int32 i = 0; i < (sal_Int32)pMappingsArr->size(); i++)
    {
        const Mapping* pMapping = &(*pMappingsArr)[i];
        OUString sPrefix(C2U(cDataSourceHistory));
        sPrefix += C2U("/_");
        sPrefix += OUString::valueOf(i);
        sPrefix += C2U("/");
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sName;
        pNodeValues[nIndex++].Value <<= pMapping->sURL;
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sTable;
        pNodeValues[nIndex++].Value <<= pMapping->sTableName;
        pNodeValues[nIndex].Name    = sPrefix;
        pNodeValues[nIndex].Name    += sCommandType;
        pNodeValues[nIndex++].Value <<= pMapping->nCommandType;
        SetSetProperties( C2U(cDataSourceHistory), aNodeValues);

        sPrefix += C2U("Fields");
        sal_Int32 nFieldAssignment = 0;
        OUString sFieldName = C2U("/ProgrammaticFieldName");
        OUString sDatabaseFieldName = C2U("/AssignedFieldName");
        ClearNodeSet( sPrefix );

        while(nFieldAssignment < COLUMN_COUNT &&
            !pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName.isEmpty())
        {
            OUString sSubPrefix(sPrefix);
            sSubPrefix += C2U("/_");
            sSubPrefix += OUString::valueOf(nFieldAssignment);
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
