/*************************************************************************
 *
 *  $RCSfile: bibconfig.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-11-20 12:23:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <bibconfig.hxx>
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
/* -----------------11.11.99 14:34-------------------

 --------------------------------------------------*/
typedef Mapping* MappingPtr;
SV_DECL_PTRARR_DEL(MappingArray, MappingPtr, 2, 2);
SV_IMPL_PTRARR(MappingArray, MappingPtr);

#define C2U(cChar) OUString::createFromAscii(cChar)

const char* cDataSourceHistory = "DataSourceHistory";
/* -----------------------------13.11.00 12:21--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString>& BibConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        aNames.realloc(3);
        OUString* pNames = aNames.getArray();
        pNames[0] = C2U("CurrentDataSource/DataSourceName");
        pNames[1] = C2U("CurrentDataSource/Command");
        pNames[2] = C2U("CurrentDataSource/CommandType");
    }
    return aNames;
}
/* -----------------------------13.11.00 11:00--------------------------------

 ---------------------------------------------------------------------------*/
BibConfig::BibConfig() :
    ConfigItem(C2U("Office.DataAccess/Bibliography")),
    pMappingsArr(new MappingArray)
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


    Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0: pValues[nProp] >>= sDataSource; break;
                    case  1: pValues[nProp] >>= sTableOrQuery; break;
                    case  2: pValues[nProp] >>= nTblOrQuery;  break;
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
        Sequence<OUString> aNames(3);
        OUString* pNames = aNames.getArray();
        OUString sPrefix(C2U(cDataSourceHistory));
        sPrefix += C2U("/");
        sPrefix += pNodeNames[nNode];
        sPrefix += C2U("/");
        pNames[0] = sPrefix;
        pNames[0] += sName;
        pNames[1] = sPrefix;
        pNames[1] += sTable;
        pNames[2] = sPrefix;
        pNames[2] += sCommandType;
        Sequence<Any> aValues = GetProperties(aNames);
        const Any* pValues = aValues.getConstArray();
        if(aValues.getLength() == aNames.getLength())
        {
            Mapping* pMapping = new Mapping;
            sal_Int32 nColumnIndex = 0;
            pValues[0] >>= pMapping->sURL;
            pValues[1] >>= pMapping->sTableName;
            pValues[2] >>= pMapping->nCommandType;
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
                pAssignmentPropertyNames[nFieldIdx++] += C2U("/FieldName");
                pAssignmentPropertyNames[nFieldIdx] = sSubPrefix;
                pAssignmentPropertyNames[nFieldIdx++]   += C2U("/DatasourceFieldName");
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
                if(sTempLogical.getLength() && sTempReal.getLength())
                {
                    pMapping->aColumnPairs[nSetMapping].sLogicalColumnName = sTempLogical;
                    pMapping->aColumnPairs[nSetMapping++].sRealColumnName = sTempReal;
                }
            }
            pMappingsArr->Insert(pMapping, pMappingsArr->Count());
        }
        pValues = aValues.getConstArray();
    }
}
/* -----------------------------13.11.00 11:00--------------------------------

 ---------------------------------------------------------------------------*/
BibConfig::~BibConfig()
{
    if(IsModified())
        Commit();
    delete pMappingsArr;
}
/* -----------------------------13.11.00 12:08--------------------------------

 ---------------------------------------------------------------------------*/
BibDBDescriptor BibConfig::GetBibliographyURL()
{
    BibDBDescriptor aRet;
    aRet.sDataSource = sDataSource;
    aRet.sTableOrQuery = sTableOrQuery;
    aRet.nCommandType = nTblOrQuery;
    return aRet;
};
/* -----------------------------13.11.00 12:20--------------------------------

 ---------------------------------------------------------------------------*/
void BibConfig::SetBibliographyURL(const BibDBDescriptor& rDesc)
{
    sDataSource = rDesc.sDataSource;
    sTableOrQuery = rDesc.sTableOrQuery;
    nTblOrQuery = rDesc.nCommandType;
    SetModified();
};
/* -----------------------------13.11.00 12:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString lcl_GetRealNameFor(const OUString& rLogName, const Mapping& rMapping)
{
    for(sal_Int16 nField = 0; nField < COLUMN_COUNT; nField++)
        if(rMapping.aColumnPairs[nField].sLogicalColumnName == rLogName)
            return rMapping.aColumnPairs[nField].sRealColumnName;
    return OUString();
}

//---------------------------------------------------------------------------
void    BibConfig::Commit()
{
    Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= sDataSource; break;
            case  1: pValues[nProp] <<= sTableOrQuery; break;
            case  2: pValues[nProp] <<= nTblOrQuery;  break;
        }
    }
    PutProperties(aNames, aValues);
    ClearNodeSet( C2U(cDataSourceHistory));
    OUString sEmpty;
    Sequence< PropertyValue > aNodeValues(pMappingsArr->Count() * 3);
    PropertyValue* pNodeValues = aNodeValues.getArray();

    sal_Int32 nIndex = 0;
    OUString sName(C2U("DataSourceName"));
    OUString sTable(C2U("Command"));
    OUString sCommandType(C2U("CommandType"));
    for(sal_Int32 i = 0; i < pMappingsArr->Count(); i++)
    {
        const Mapping* pMapping = pMappingsArr->GetObject(i);
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
        OUString sFieldName = C2U("/FieldName");
        OUString sDatabaseFieldName = C2U("/DatasourceFieldName");
        ClearNodeSet( sPrefix );

        while(pMapping->aColumnPairs[nFieldAssignment].sLogicalColumnName.getLength())
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
/* -----------------------------13.11.00 12:23--------------------------------

 ---------------------------------------------------------------------------*/
const Mapping*  BibConfig::GetMapping(const BibDBDescriptor& rDesc) const
{
    for(sal_uInt16 i = 0; i < pMappingsArr->Count(); i++)
    {
        const Mapping* pMapping = pMappingsArr->GetObject(i);
        sal_Bool bURLEqual = rDesc.sDataSource.equals(pMapping->sURL);
        if(rDesc.sTableOrQuery == pMapping->sTableName && bURLEqual)
            return pMapping;
    }
    return 0;
}
/* -----------------------------13.11.00 12:23--------------------------------

 ---------------------------------------------------------------------------*/
void BibConfig::SetMapping(const BibDBDescriptor& rDesc, const Mapping* pSetMapping)
{
    for(sal_uInt16 i = 0; i < pMappingsArr->Count(); i++)
    {
        const Mapping* pMapping = pMappingsArr->GetObject(i);
        sal_Bool bURLEqual = rDesc.sDataSource.equals(pMapping->sURL);
        if(rDesc.sTableOrQuery == pMapping->sTableName && bURLEqual)
        {
            pMappingsArr->DeleteAndDestroy(i, 1);
            break;
        }
    }
    Mapping* pNew = new Mapping(*pSetMapping);
    pMappingsArr->Insert(pNew, pMappingsArr->Count());
    SetModified();
}
/* -----------------------------20.11.00 11:56--------------------------------

 ---------------------------------------------------------------------------*/
DBChangeDialogConfig_Impl::DBChangeDialogConfig_Impl() :
    ConfigItem(C2U("Office.DataAccess/DataSources"))
{
    aSourceNames = GetNodeNames(OUString());
}
/* -----------------------------20.11.00 11:57--------------------------------

 ---------------------------------------------------------------------------*/
DBChangeDialogConfig_Impl::~DBChangeDialogConfig_Impl()
{
}
/* -----------------------------20.11.00 11:57--------------------------------

 ---------------------------------------------------------------------------*/
void    DBChangeDialogConfig_Impl::Commit()
{
    // read only
}

