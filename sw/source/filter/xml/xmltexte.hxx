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



#ifndef _XMLTEXTE_HXX
#define _XMLTEXTE_HXX

#include <xmloff/txtparae.hxx>
#include <tools/globname.hxx>

class SwXMLExport;
class SvXMLAutoStylePoolP;
class SwNoTxtNode;

namespace com { namespace sun { namespace star { namespace style {
                class XStyle; } } } }

class SwXMLTextParagraphExport : public XMLTextParagraphExport
{
    const ::rtl::OUString sTextTable;
    const ::rtl::OUString sEmbeddedObjectProtocol;
    const ::rtl::OUString sGraphicObjectProtocol;

    const SvGlobalName aAppletClassId;
    const SvGlobalName aPluginClassId;
    const SvGlobalName aIFrameClassId;
    const SvGlobalName aOutplaceClassId;

    SwNoTxtNode *GetNoTxtNode(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet ) const;

protected:
    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    virtual void _collectTextEmbeddedAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet );
    virtual void _exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    virtual void exportTable(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress );

public:
    SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool );
    ~SwXMLTextParagraphExport();

    virtual void setTextEmbeddedGraphicURL(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        ::rtl::OUString& rStreamName ) const;
};


#endif  //  _XMLTEXTE_HXX
