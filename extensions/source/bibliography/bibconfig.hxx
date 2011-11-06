/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BIBCONFIG_HXX
#define _BIBCONFIG_HXX

#include <unotools/configitem.hxx>
class MappingArray;

//-----------------------------------------------------------------------------
#define COLUMN_COUNT                31
#define IDENTIFIER_POS              0
#define AUTHORITYTYPE_POS           1
#define AUTHOR_POS                  2
#define TITLE_POS                   3
#define YEAR_POS                    4
#define ISBN_POS                    5
#define BOOKTITLE_POS               6
#define CHAPTER_POS                 7
#define EDITION_POS                 8
#define EDITOR_POS                  9
#define HOWPUBLISHED_POS            10
#define INSTITUTION_POS             11
#define JOURNAL_POS                 12
#define MONTH_POS                   13
#define NOTE_POS                    14
#define ANNOTE_POS                  15
#define NUMBER_POS                  16
#define ORGANIZATIONS_POS           17
#define PAGES_POS                   18
#define PUBLISHER_POS               19
#define ADDRESS_POS                 20
#define SCHOOL_POS                  21
#define SERIES_POS                  22
#define REPORTTYPE_POS              23
#define VOLUME_POS                  24
#define URL_POS                     25
#define CUSTOM1_POS                 26
#define CUSTOM2_POS                 27
#define CUSTOM3_POS                 28
#define CUSTOM4_POS                 29
#define CUSTOM5_POS                 30
//-----------------------------------------------------------------------------
struct StringPair
{
    rtl::OUString   sRealColumnName;
    rtl::OUString   sLogicalColumnName;
};
//-----------------------------------------------------------------------------
struct Mapping
{
    rtl::OUString       sTableName;
    rtl::OUString       sURL;
    sal_Int16           nCommandType;
    StringPair          aColumnPairs[COLUMN_COUNT];

    Mapping() :
        nCommandType(0){}
};
//-----------------------------------------------------------------------------
struct BibDBDescriptor
{
    rtl::OUString   sDataSource;
    rtl::OUString   sTableOrQuery;
    sal_Int32       nCommandType;
};
//-----------------------------------------------------------------------------

class BibConfig : public utl::ConfigItem
{
    rtl::OUString   sDataSource;
    rtl::OUString   sTableOrQuery;
    sal_Int32       nTblOrQuery;

    rtl::OUString   sQueryField;
    rtl::OUString   sQueryText;
    MappingArray*               pMappingsArr;
    long            nBeamerSize;
    long            nViewSize;
    sal_Bool        bShowColumnAssignmentWarning;

    rtl::OUString               aColumnDefaults[COLUMN_COUNT];

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    BibConfig();
    ~BibConfig();

    virtual void    Commit();
    virtual void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);

    BibDBDescriptor         GetBibliographyURL();
    void                    SetBibliographyURL(const BibDBDescriptor& rDesc);

    const Mapping*          GetMapping(const BibDBDescriptor& rDesc) const;
    void                    SetMapping(const BibDBDescriptor& rDesc, const Mapping* pMapping);

    const rtl::OUString&    GetDefColumnName(sal_uInt16 nIndex) const
                                            {return aColumnDefaults[nIndex];}


    void                    setBeamerSize(long nSize) {SetModified(); nBeamerSize = nSize;}
    long                    getBeamerSize()const {return nBeamerSize;}
    void                    setViewSize(long nSize) {SetModified(); nViewSize = nSize;}
    long                    getViewSize() {return nViewSize;}

    const rtl::OUString&    getQueryField() const {return sQueryField;}
    void                    setQueryField(const rtl::OUString& rSet) {SetModified(); sQueryField = rSet;}

    const rtl::OUString&    getQueryText() const {return sQueryText;}
    void                    setQueryText(const rtl::OUString& rSet) {SetModified(); sQueryText = rSet;}

    sal_Bool                IsShowColumnAssignmentWarning() const
                                { return bShowColumnAssignmentWarning;}
    void                    SetShowColumnAssignmentWarning(sal_Bool bSet)
                                { bShowColumnAssignmentWarning = bSet;}
};
/* -----------------------------20.11.00 11:47--------------------------------

 ---------------------------------------------------------------------------*/
class DBChangeDialogConfig_Impl
{
    com::sun::star::uno::Sequence<rtl::OUString> aSourceNames;
public:
    DBChangeDialogConfig_Impl();
    ~DBChangeDialogConfig_Impl();

    const com::sun::star::uno::Sequence<rtl::OUString>& GetDataSourceNames();

};
#endif
