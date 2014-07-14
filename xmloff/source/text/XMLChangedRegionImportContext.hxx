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




#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace text {
        class XTextCursor;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}


class XMLChangedRegionImportContext : public SvXMLImportContext
{
    const ::rtl::OUString sEmpty;

    /// if we replace the current XTextCursor/XText by the ones for
    /// the redline, we remember the old cursor here.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextCursor> xOldCursor;

    ::rtl::OUString sID;        /// redline-ID

    sal_Bool bMergeLastPara;    /// merge-last-paragraph flag

public:

    XMLChangedRegionImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName);

    ~XMLChangedRegionImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
         const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();


    /// change info: To be called from change-info context
    void SetChangeInfo(const ::rtl::OUString& rType,
                       const ::rtl::OUString& rAuthor,
                       const ::rtl::OUString& rComment,
                       const ::rtl::OUString& rDate);

    /// create redline XText/XTextCursor on demand and register with
    /// XMLTextImportHelper
    void UseRedlineText();
};


#endif
