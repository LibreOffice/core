/*************************************************************************
 *
 *  $RCSfile: bibconfig.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2001-03-14 12:28:21 $
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

#ifndef _BIBCONFIG_HXX
#define _BIBCONFIG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
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
    long            nBeamerSize;
    long            nViewSize;

    MappingArray*               pMappingsArr;
    rtl::OUString               aColumnDefaults[COLUMN_COUNT];

    com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
public:
    BibConfig();
    ~BibConfig();

    virtual void    Commit();

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
