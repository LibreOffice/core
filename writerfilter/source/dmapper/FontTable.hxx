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



#ifndef INCLUDED_FONTTABLE_HXX
#define INCLUDED_FONTTABLE_HXX

#include <boost/shared_ptr.hpp>
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl;
struct FontEntry
{
    typedef boost::shared_ptr<FontEntry> Pointer_t;

    ::rtl::OUString sFontName;
    ::rtl::OUString sFontName1;
    bool            bTrueType;
    sal_Int16       nPitchRequest;
    sal_Int32       nTextEncoding;
    sal_Int32       nFontFamilyId;
    sal_Int32       nBaseWeight;
    sal_Int32       nAltFontIndex;
    ::rtl::OUString sPanose;
    ::rtl::OUString sFontSignature;
    ::rtl::OUString sAlternativeFont;
    FontEntry() :
        bTrueType(false),
        nPitchRequest( 0 ),
        nTextEncoding( 0 ),
        nFontFamilyId( 0 ),
        nBaseWeight( 0 ),
        nAltFontIndex( 0 )
        {}
};

class WRITERFILTER_DLLPRIVATE FontTable : public LoggedProperties, public LoggedTable
    /*,public BinaryObj*/, public LoggedStream
{
    FontTable_Impl   *m_pImpl;

 public:
    FontTable();
    virtual ~FontTable();

    sal_uInt32          size();
    const FontEntry::Pointer_t  getFontEntry(sal_uInt32 nIndex);

 private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void lcl_startSectionGroup();
    virtual void lcl_endSectionGroup();
    virtual void lcl_startParagraphGroup();
    virtual void lcl_endParagraphGroup();
    virtual void lcl_startCharacterGroup();
    virtual void lcl_endCharacterGroup();
    virtual void lcl_text(const sal_uInt8 * data, size_t len);
    virtual void lcl_utext(const sal_uInt8 * data, size_t len);
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref);
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void lcl_info(const string & info);
    virtual void lcl_startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void lcl_endShape( );

};
typedef boost::shared_ptr< FontTable >          FontTablePtr;
}}

#endif //
