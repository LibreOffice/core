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
#include "bibconfig.hxx"
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <o3tl/any.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;


constexpr OUString cDataSourceHistory = u"DataSourceHistory"_ustr;

Sequence<OUString> const & BibConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames =
    {
        u"CurrentDataSource/DataSourceName"_ustr,
        u"CurrentDataSource/Command"_ustr,
        u"CurrentDataSource/CommandType"_ustr,
        u"BeamerHeight"_ustr,
        u"ViewHeight"_ustr,
        u"QueryText"_ustr,
        u"QueryField"_ustr,
        u"ShowColumnAssignmentWarning"_ustr
    };
    return aNames;
}

BibConfig::BibConfig()
    : ConfigItem(u"Office.DataAccess/Bibliography"_ustr, ConfigItemMode::NONE)
    , nTblOrQuery(0)
    , nBeamerSize(0)
    , nViewSize(0)
    , bShowColumnAssignmentWarning(false)
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
    aColumnDefaults[31] = "LocalURL";


    const Sequence< OUString > aPropertyNames = GetPropertyNames();
    const Sequence<Any> aPropertyValues = GetProperties( aPropertyNames );
    if(aPropertyValues.getLength() == aPropertyNames.getLength())
    {
        for(int nProp = 0; nProp < aPropertyNames.getLength(); nProp++)
        {
            if (aPropertyValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: aPropertyValues[nProp] >>= sDataSource; break;
                    case  1: aPropertyValues[nProp] >>= sTableOrQuery; break;
                    case  2: aPropertyValues[nProp] >>= nTblOrQuery;  break;
                    case  3: aPropertyValues[nProp] >>= nBeamerSize;  break;
                    case  4: aPropertyValues[nProp] >>= nViewSize  ;  break;
                    case  5: aPropertyValues[nProp] >>= sQueryText ;  break;
                    case  6: aPropertyValues[nProp] >>= sQueryField;  break;
                    case  7:
                        bShowColumnAssignmentWarning = *o3tl::doAccess<bool>(aPropertyValues[nProp]);
                    break;
                }
            }
        }
    }
    const Sequence< OUString > aNodeNames = GetNodeNames(cDataSourceHistory);
    for(OUString const & nodeName : aNodeNames)
    {
        Sequence<OUString> aHistoryNames(3);
        OUString* pHistoryNames = aHistoryNames.getArray();

        OUString sPrefix = OUString::Concat(cDataSourceHistory) + "/" + nodeName + "/";
        pHistoryNames[0] = sPrefix + "DataSourceName";
        pHistoryNames[1] = sPrefix + "Command";
        pHistoryNames[2] = sPrefix + "CommandType";

        Sequence<Any> aHistoryValues = GetProperties( aHistoryNames );

        if(aHistoryValues.getLength() == aHistoryNames.getLength())
        {
            Mapping* pMapping = new Mapping;
            aHistoryValues[0] >>= pMapping->sURL;
            aHistoryValues[1] >>= pMapping->sTableName;
            aHistoryValues[2] >>= pMapping->nCommandType;
            //field assignment is contained in another set
            sPrefix += "Fields";
            const Sequence< OUString > aAssignmentNodeNames = GetNodeNames(sPrefix);
            Sequence<OUString> aAssignmentPropertyNames(aAssignmentNodeNames.getLength() * 2);
            OUString* pAssignmentPropertyNames = aAssignmentPropertyNames.getArray();
            sal_Int16 nFieldIdx = 0;
            for(OUString const & assignName : aAssignmentNodeNames)
            {
                OUString sSubPrefix = sPrefix + "/" + assignName;
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++] += "/ProgrammaticFieldName";
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++]   += "/AssignedFieldName";
            }
            Sequence<Any> aAssignmentValues = GetProperties(aAssignmentPropertyNames);
            OUString sTempLogical;
            OUString sTempReal;
            sal_Int16 nSetMapping = 0;
            nFieldIdx = 0;
            for(sal_Int32 nFieldVal = 0; nFieldVal < aAssignmentValues.getLength() / 2; nFieldVal++)
            {
                aAssignmentValues[nFieldIdx++] >>= sTempLogical;
                aAssignmentValues[nFieldIdx++] >>= sTempReal;
                if(!(sTempLogical.isEmpty() || sTempReal.isEmpty()))
                {
                    pMapping->aColumnPairs[nSetMapping].sLogicalColumnName = sTempLogical;
                    pMapping->aColumnPairs[nSetMapping++].sRealColumnName = sTempReal;
                }
            }
            mvMappings.push_back(std::unique_ptr<Mapping>(pMapping));
        }
    }
}

BibConfig::~BibConfig()
{
    assert(!IsModified()); // should have been committed
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

void BibConfig::Notify( const css::uno::Sequence<OUString>& )
{
}

void    BibConfig::ImplCommit()
{
    PutProperties(
        GetPropertyNames(),
        {css::uno::Any(sDataSource), css::uno::Any(sTableOrQuery),
         css::uno::Any(nTblOrQuery), css::uno::Any(nBeamerSize),
         css::uno::Any(nViewSize), css::uno::Any(sQueryText),
         css::uno::Any(sQueryField),
         css::uno::Any(bShowColumnAssignmentWarning)});
    ClearNodeSet(cDataSourceHistory);
    Sequence< PropertyValue > aNodeValues(mvMappings.size() * 3);
    PropertyValue* pNodeValues = aNodeValues.getArray();

    sal_Int32 nIndex = 0;
    for(sal_Int32 i = 0; i < static_cast<sal_Int32>(mvMappings.size()); i++)
    {
        const Mapping* pMapping = mvMappings[i].get();
        OUString sPrefix = OUString::Concat(cDataSourceHistory) + "/_" + OUString::number(i) + "/";
        pNodeValues[nIndex].Name    = sPrefix + "DataSourceName";
        pNodeValues[nIndex++].Value <<= pMapping->sURL;
        pNodeValues[nIndex].Name    = sPrefix + "Command";
        pNodeValues[nIndex++].Value <<= pMapping->sTableName;
        pNodeValues[nIndex].Name    = sPrefix + "CommandType";
        pNodeValues[nIndex++].Value <<= pMapping->nCommandType;
        SetSetProperties(cDataSourceHistory, aNodeValues);

        sPrefix += "Fields";
        sal_Int32 nFieldAssignment = 0;
        OUString sFieldName = u"/ProgrammaticFieldName"_ustr;
        OUString sDatabaseFieldName = u"/AssignedFieldName"_ustr;
        ClearNodeSet( sPrefix );

        while(nFieldAssignment < COLUMN_COUNT &&
            !pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName.isEmpty())
        {
            OUString sSubPrefix = sPrefix + "/_" + OUString::number(nFieldAssignment);
            Sequence< PropertyValue > aAssignmentValues
            {
                comphelper::makePropertyValue(sSubPrefix + sFieldName, pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName),
                comphelper::makePropertyValue(sSubPrefix + sDatabaseFieldName, pMapping->aColumnPairs[nFieldAssignment].sRealColumnName)
            };
            SetSetProperties( sPrefix, aAssignmentValues );
            nFieldAssignment++;
        }
    }
}

const Mapping*  BibConfig::GetMapping(const BibDBDescriptor& rDesc) const
{
    for(std::unique_ptr<Mapping> const & i : mvMappings)
    {
        Mapping& rMapping = *i;
        bool bURLEqual = rDesc.sDataSource == rMapping.sURL;
        if(rDesc.sTableOrQuery == rMapping.sTableName && bURLEqual)
            return &rMapping;
    }
    return nullptr;
}

void BibConfig::SetMapping(const BibDBDescriptor& rDesc, const Mapping* pSetMapping)
{
    for(size_t i = 0; i < mvMappings.size(); i++)
    {
        Mapping& rMapping = *mvMappings[i];
        bool bURLEqual = rDesc.sDataSource == rMapping.sURL;
        if(rDesc.sTableOrQuery == rMapping.sTableName && bURLEqual)
        {
            mvMappings.erase(mvMappings.begin()+i);
            break;
        }
    }
    mvMappings.push_back(std::make_unique<Mapping>(*pSetMapping));
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
    if(!aSourceNames.hasElements())
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDatabaseContext> xDBContext = DatabaseContext::create(xContext);
        aSourceNames = xDBContext->getElementNames();
    }
    return aSourceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
