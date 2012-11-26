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



#ifndef _XMLTEXTFRAMECONTEXT_HXX
#define _XMLTEXTFRAMECONTEXT_HXX

#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlmultiimagehelper.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; class XTextContent; }
} } }

class SvXMLAttributeList;
class XMLTextFrameContextHyperlink_Impl;

class XMLTextFrameContext : public SvXMLImportContext, public multiImageImportHelper
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;

    SvXMLImportContextRef m_xImplContext;
    SvXMLImportContextRef m_xReplImplContext;
    SvXMLAttributeList *m_pAttrList;

    XMLTextFrameContextHyperlink_Impl   *m_pHyperlink;
    // --> OD 2009-07-22 #i73249#
    ::rtl::OUString m_sTitle;
    // <--
    ::rtl::OUString m_sDesc;

    ::com::sun::star::text::TextContentAnchorType   m_eDefaultAnchorType;

    // --> OD 2006-03-10 #i51726#
    // The <draw:name> can longer be used to distinguish Writer graphic/text box
    // objects and Draw graphic/text box objects.
    // The new distinguish attribute is the parent style of the automatic style
    // of the object. All Draw objects have an automatic style without a parent style.
    sal_Bool m_HasAutomaticStyleWithoutParentStyle;
    // <--
    sal_Bool m_bSupportsReplacement;

    sal_Bool CreateIfNotThere();
    sal_Bool CreateIfNotThere( ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet >& rPropSet );

protected:
    /// helper to get the created xShape instance, needs to be overloaded
    virtual rtl::OUString getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const;
    virtual void removeGraphicFromImportContext(const SvXMLImportContext& rContext) const;

public:

    XMLTextFrameContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            ::com::sun::star::text::TextContentAnchorType eDfltAnchorType );
    virtual ~XMLTextFrameContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    void SetHyperlink( const ::rtl::OUString& rHRef,
                       const ::rtl::OUString& rName,
                       const ::rtl::OUString& rTargetFrameName,
                       sal_Bool bMap );

    ::com::sun::star::text::TextContentAnchorType GetAnchorType() const;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextContent > GetTextContent() const;
    // --> OD 2004-08-24 #i33242#
    ::com::sun::star::uno::Reference <
        ::com::sun::star::drawing::XShape > GetShape() const;
    // <--
};


#endif
