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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>

#include <memory>
#include <string_view>
#include <vector>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/prstylei.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/txtimp.hxx>
#include "txtparai.hxx"
#include <txtfldi.hxx>
#include "XMLFootnoteImportContext.hxx"
#include "XMLTextMarkImportContext.hxx"
#include "XMLTextFrameContext.hxx"
#include <xmloff/XMLCharContext.hxx>
#include "XMLTextFrameHyperlinkContext.hxx"
#include <xmloff/XMLEventsImportContext.hxx>
#include "XMLChangeImportContext.hxx"
#include <txtlists.hxx>

#include "txtparaimphint.hxx"
#include "xmllinebreakcontext.hxx"
#include "xmlcontentcontrolcontext.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;

class XMLHints_Impl
{
private:

    std::vector<std::unique_ptr<XMLHint_Impl>> m_Hints;
    std::unordered_map<OUString, XMLIndexMarkHint_Impl*> m_IndexHintsById;
    uno::Reference<uno::XInterface> m_xCrossRefHeadingBookmark;

public:
    void push_back(std::unique_ptr<XMLHint_Impl> pHint)
    {
        m_Hints.push_back(std::move(pHint));
    }

    void push_back(std::unique_ptr<XMLIndexMarkHint_Impl> pHint)
    {
        m_IndexHintsById.emplace(pHint->GetID(), pHint.get());
        m_Hints.push_back(std::move(pHint));
    }

    std::vector<std::unique_ptr<XMLHint_Impl>> const& GetHints() const
    {
        return m_Hints;
    }

    XMLIndexMarkHint_Impl* GetIndexHintById(const OUString& sID)
    {
        auto it = m_IndexHintsById.find(sID);
        return it == m_IndexHintsById.end() ? nullptr : it->second;
    }

    uno::Reference<uno::XInterface> & GetCrossRefHeadingBookmark()
    {
        return m_xCrossRefHeadingBookmark;
    }
};


XMLCharContext::XMLCharContext(
        SvXMLImport& rImport,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        sal_Unicode c,
        bool bCount ) :
    SvXMLImportContext( rImport )
    ,m_nControl(0)
    ,m_nCount(1)
    ,m_c(c)
{
    if( !bCount )
        return;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_C) )
        {
            sal_Int32 nTmp = aIter.toInt32();
            if( nTmp > 0 )
            {
                if( nTmp > SAL_MAX_UINT16 )
                    m_nCount = SAL_MAX_UINT16;
                else
                    m_nCount = static_cast<sal_uInt16>(nTmp);
            }
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

XMLCharContext::XMLCharContext(
        SvXMLImport& rImp,
        sal_Int16 nControl ) :
    SvXMLImportContext( rImp )
    ,m_nControl(nControl)
    ,m_nCount(0)
    ,m_c(0)
{
}

XMLCharContext::~XMLCharContext()
{
}
void XMLCharContext::endFastElement(sal_Int32 )
{
    if ( !m_nCount )
        InsertControlCharacter( m_nControl );
    else
    {
        if( 1U == m_nCount )
        {
            OUString sBuff( &m_c, 1 );
            InsertString(sBuff);
        }
        else
        {
            OUStringBuffer sBuff(static_cast<int>(m_nCount));
            while( m_nCount-- )
                sBuff.append( &m_c, 1 );

            InsertString(sBuff.makeStringAndClear() );
        }
    }
}
void XMLCharContext::InsertControlCharacter(sal_Int16   _nControl)
{
    GetImport().GetTextImport()->InsertControlCharacter( _nControl );
}
void XMLCharContext::InsertString(const OUString& _sString)
{
    GetImport().GetTextImport()->InsertString( _sString );
}

namespace {

/** import start of reference (<text:reference-start>) */
class XMLStartReferenceContext_Impl : public SvXMLImportContext
{
public:

    // Do everything in constructor. Well ...
    XMLStartReferenceContext_Impl (
        SvXMLImport& rImport,
        XMLHints_Impl& rHints,
        const Reference<xml::sax::XFastAttributeList> & xAttrList);

    static bool FindName(
        const Reference<xml::sax::XFastAttributeList> & xAttrList,
        OUString& rName);
};

}

XMLStartReferenceContext_Impl::XMLStartReferenceContext_Impl(
    SvXMLImport& rImport,
    XMLHints_Impl& rHints,
    const Reference<xml::sax::XFastAttributeList> & xAttrList) :
        SvXMLImportContext(rImport)
{
    OUString sName;

    if (FindName(xAttrList, sName))
    {
        std::unique_ptr<XMLHint_Impl> pHint(new XMLReferenceHint_Impl(
            sName, rImport.GetTextImport()->GetCursor()->getStart()));

        // degenerates to point reference, if no end is found!
        pHint->SetEnd(rImport.GetTextImport()->GetCursor()->getStart() );

        rHints.push_back(std::move(pHint));
    }
}

bool XMLStartReferenceContext_Impl::FindName(
    const Reference<xml::sax::XFastAttributeList> & xAttrList,
    OUString& rName)
{
    bool bNameOK( false );

    // find name attribute first
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_NAME) )
        {
            rName = aIter.toString();
            bNameOK = true;
            break;
        }
    }

    return bNameOK;
}

namespace {

/** import end of reference (<text:reference-end>) */
class XMLEndReferenceContext_Impl : public SvXMLImportContext
{
public:

    // Do everything in constructor. Well ...
    XMLEndReferenceContext_Impl(
        SvXMLImport& rImport,
        const XMLHints_Impl& rHints,
        const Reference<xml::sax::XFastAttributeList> & xAttrList);
};

}

XMLEndReferenceContext_Impl::XMLEndReferenceContext_Impl(
    SvXMLImport& rImport,
    const XMLHints_Impl& rHints,
    const Reference<xml::sax::XFastAttributeList> & xAttrList) :
        SvXMLImportContext(rImport)
{
    OUString sName;

    // borrow from XMLStartReferenceContext_Impl
    if (!XMLStartReferenceContext_Impl::FindName(xAttrList, sName))
        return;

    // search for reference start
    for (const auto& rHintPtr : rHints.GetHints())
    {
        XMLHint_Impl *const pHint = rHintPtr.get();
        if ( pHint->IsReference() &&
             sName == static_cast<XMLReferenceHint_Impl *>(pHint)->GetRefName() )
        {
            // set end and stop searching
            pHint->SetEnd(GetImport().GetTextImport()->
                                 GetCursor()->getStart() );
            break;
        }
    }
    // else: no start (in this paragraph) -> ignore
}

namespace {

class XMLImpHyperlinkContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&  m_rHints;
    XMLHyperlinkHint_Impl   *mpHint;

    bool&       mrbIgnoreLeadingSpace;

public:


    XMLImpHyperlinkContext_Impl(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList > & xAttrList,
            XMLHints_Impl& rHints,
            bool& rIgnLeadSpace );

    virtual ~XMLImpHyperlinkContext_Impl() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

}

XMLImpHyperlinkContext_Impl::XMLImpHyperlinkContext_Impl(
    SvXMLImport& rImport,
    sal_Int32 /*nElement*/,
    const Reference< xml::sax::XFastAttributeList > & xAttrList,
    XMLHints_Impl& rHints,
    bool& rIgnLeadSpace )
    : SvXMLImportContext( rImport )
    , m_rHints( rHints )
    , mpHint( new XMLHyperlinkHint_Impl( GetImport().GetTextImport()->GetCursorAsRange()->getStart() ) )
    , mrbIgnoreLeadingSpace( rIgnLeadSpace )
{
    OUString sShow;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
        case XML_ELEMENT(XLINK, XML_HREF):
            mpHint->SetHRef( GetImport().GetAbsoluteReference( sValue ) );
            break;
        case XML_ELEMENT(OFFICE, XML_NAME):
            mpHint->SetName( sValue );
            break;
        case XML_ELEMENT(OFFICE, XML_TARGET_FRAME_NAME):
            mpHint->SetTargetFrameName( sValue );
            break;
        case XML_ELEMENT(XLINK, XML_SHOW):
            sShow = sValue;
            break;
        case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            mpHint->SetStyleName( sValue );
            break;
        case XML_ELEMENT(TEXT, XML_VISITED_STYLE_NAME):
            mpHint->SetVisitedStyleName( sValue );
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !sShow.isEmpty() && mpHint->GetTargetFrameName().isEmpty() )
    {
        if( IsXMLToken( sShow, XML_NEW ) )
            mpHint->SetTargetFrameName(
                    u"_blank"_ustr );
        else if( IsXMLToken( sShow, XML_REPLACE ) )
            mpHint->SetTargetFrameName(
                    u"_self"_ustr );
    }

    if ( mpHint->GetHRef().isEmpty() )
    {
        // hyperlink without a URL is not imported.
        delete mpHint;
        mpHint = nullptr;
    }
    else
    {
        m_rHints.push_back(std::unique_ptr<XMLHyperlinkHint_Impl>(mpHint));
    }
}

XMLImpHyperlinkContext_Impl::~XMLImpHyperlinkContext_Impl()
{
    if (mpHint)
        mpHint->SetEnd( GetImport().GetTextImport()
                            ->GetCursorAsRange()->getStart() );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImpHyperlinkContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        XMLEventsImportContext* pCtxt = new XMLEventsImportContext(GetImport());
        if (mpHint)
            mpHint->SetEventsContext(pCtxt);
        return pCtxt;
    }
    else
    {
        return XMLImpSpanContext_Impl::CreateSpanContext(
            GetImport(), nElement, xAttrList,
            m_rHints, mrbIgnoreLeadingSpace );
    }
}

void XMLImpHyperlinkContext_Impl::characters( const OUString& rChars )
{
    GetImport().GetTextImport()->InsertString( rChars, mrbIgnoreLeadingSpace );
}

namespace {

class XMLImpRubyBaseContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&  m_rHints;

    bool&       rIgnoreLeadingSpace;

public:


    XMLImpRubyBaseContext_Impl(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList > & xAttrList,
            XMLHints_Impl& rHints,
            bool& rIgnLeadSpace );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

}

XMLImpRubyBaseContext_Impl::XMLImpRubyBaseContext_Impl(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > &,
        XMLHints_Impl& rHints,
        bool& rIgnLeadSpace )
    : SvXMLImportContext( rImport )
    , m_rHints( rHints )
    , rIgnoreLeadingSpace( rIgnLeadSpace )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImpRubyBaseContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    return XMLImpSpanContext_Impl::CreateSpanContext( GetImport(), nElement, xAttrList,
                               m_rHints, rIgnoreLeadingSpace );
}

void XMLImpRubyBaseContext_Impl::characters( const OUString& rChars )
{
    GetImport().GetTextImport()->InsertString( rChars, rIgnoreLeadingSpace );
}

namespace {

class XMLImpRubyContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl&  m_rHints;

    bool&       rIgnoreLeadingSpace;

    Reference < XTextRange > m_xStart;
    OUString        m_sStyleName;
    OUString        m_sTextStyleName;
    OUString        m_sText;

public:


    XMLImpRubyContext_Impl(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList > & xAttrList,
            XMLHints_Impl& rHints,
            bool& rIgnLeadSpace );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    void SetTextStyleName( const OUString& s ) { m_sTextStyleName = s; }
    void AppendText( std::u16string_view s ) { m_sText += s; }
};

class XMLImpRubyTextContext_Impl : public SvXMLImportContext
{
    XMLImpRubyContext_Impl & m_rRubyContext;

public:


    XMLImpRubyTextContext_Impl(
            SvXMLImport& rImport,
            sal_Int32 nElement,
            const Reference< xml::sax::XFastAttributeList > & xAttrList,
            XMLImpRubyContext_Impl & rParent );

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

}

XMLImpRubyTextContext_Impl::XMLImpRubyTextContext_Impl(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        XMLImpRubyContext_Impl & rParent )
    : SvXMLImportContext( rImport )
    , m_rRubyContext( rParent )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME) )
        {
            m_rRubyContext.SetTextStyleName( aIter.toString() );
            break;
        }
    }
}

void XMLImpRubyTextContext_Impl::characters( const OUString& rChars )
{
    m_rRubyContext.AppendText( rChars );
}


XMLImpRubyContext_Impl::XMLImpRubyContext_Impl(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        XMLHints_Impl& rHints,
        bool& rIgnLeadSpace )
    : SvXMLImportContext( rImport )
    , m_rHints( rHints )
    , rIgnoreLeadingSpace( rIgnLeadSpace )
    , m_xStart( GetImport().GetTextImport()->GetCursorAsRange()->getStart() )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME) )
        {
            m_sStyleName = aIter.toString();
            break;
        }
    }
}

void XMLImpRubyContext_Impl::endFastElement(sal_Int32 )
{
    const rtl::Reference < XMLTextImportHelper > xTextImport(
        GetImport().GetTextImport());
    const Reference < XTextCursor > xAttrCursor(
        xTextImport->GetText()->createTextCursorByRange( m_xStart ));
    if (!xAttrCursor.is())
    {
        SAL_WARN("xmloff.text", "cannot insert ruby");
        return;
    }
    xAttrCursor->gotoRange(xTextImport->GetCursorAsRange()->getStart(),
            true);
    xTextImport->SetRuby( GetImport(), xAttrCursor,
         m_sStyleName, m_sTextStyleName, m_sText );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImpRubyContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    if( nElement == XML_ELEMENT(TEXT, XML_RUBY_BASE) )
        return new XMLImpRubyBaseContext_Impl( GetImport(), nElement,
                                                   xAttrList,
                                                   m_rHints,
                                                   rIgnoreLeadingSpace );
    else if( nElement == XML_ELEMENT(TEXT, XML_RUBY_TEXT) )
        return new XMLImpRubyTextContext_Impl( GetImport(), nElement,
                                                       xAttrList,
                                                       *this );
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

namespace {

/** for text:meta and text:meta-field
 */
class XMLMetaImportContextBase : public SvXMLImportContext
{
    XMLHints_Impl&    m_rHints;

    bool& m_rIgnoreLeadingSpace;

    /// start position
    Reference<XTextRange> m_xStart;

protected:
    OUString m_XmlId;

public:

    XMLMetaImportContextBase(
        SvXMLImport& i_rImport,
        const sal_Int32 nElement,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace );

    virtual void SAL_CALL startFastElement(
            sal_Int32 nElement,
            const Reference<xml::sax::XFastAttributeList> & i_xAttrList) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL characters( const OUString& i_rChars ) override;

    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter);

    virtual void InsertMeta(const Reference<XTextRange> & i_xInsertionRange)
        = 0;
};

}

XMLMetaImportContextBase::XMLMetaImportContextBase(
        SvXMLImport& i_rImport,
        const sal_Int32 /*i_nElement*/,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace )
    : SvXMLImportContext( i_rImport )
    , m_rHints( i_rHints )
    , m_rIgnoreLeadingSpace( i_rIgnoreLeadingSpace )
    , m_xStart( GetImport().GetTextImport()->GetCursorAsRange()->getStart() )
{
}

void XMLMetaImportContextBase::startFastElement(
        sal_Int32 /*nElement*/,
        const Reference<xml::sax::XFastAttributeList> & xAttrList)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
        ProcessAttribute(aIter);
}

void XMLMetaImportContextBase::endFastElement(sal_Int32 )
{
    SAL_WARN_IF(!m_xStart.is(), "xmloff.text", "no mxStart?");
    if (!m_xStart.is()) return;

    const Reference<XTextRange> xEndRange(
        GetImport().GetTextImport()->GetCursorAsRange()->getStart() );

    // create range for insertion
    const Reference<XTextCursor> xInsertionCursor(
        GetImport().GetTextImport()->GetText()->createTextCursorByRange(
            xEndRange) );
    xInsertionCursor->gotoRange(m_xStart, true);

    InsertMeta(xInsertionCursor);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLMetaImportContextBase::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    return XMLImpSpanContext_Impl::CreateSpanContext( GetImport(), nElement,
        xAttrList, m_rHints, m_rIgnoreLeadingSpace );
}

void XMLMetaImportContextBase::characters( const OUString& i_rChars )
{
    GetImport().GetTextImport()->InsertString(i_rChars, m_rIgnoreLeadingSpace);
}

void XMLMetaImportContextBase::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    if ( aIter.getToken() == XML_ELEMENT(XML, XML_ID) )
        m_XmlId = aIter.toString();
    else
        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
}

namespace {

/** text:meta */
class XMLMetaImportContext : public XMLMetaImportContextBase
{
    // RDFa
    bool m_bHaveAbout;
    OUString m_sAbout;
    OUString m_sProperty;
    OUString m_sContent;
    OUString m_sDatatype;

public:

    XMLMetaImportContext(
        SvXMLImport& i_rImport,
        sal_Int32 nElement,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace );

    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter) override;

    virtual void InsertMeta(const Reference<XTextRange> & i_xInsertionRange) override;
};

}

XMLMetaImportContext::XMLMetaImportContext(
        SvXMLImport& i_rImport,
        sal_Int32 nElement,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace )
    : XMLMetaImportContextBase( i_rImport, nElement,
            i_rHints, i_rIgnoreLeadingSpace )
    , m_bHaveAbout(false)
{
}

void XMLMetaImportContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        // RDFa
        case XML_ELEMENT(XHTML, XML_ABOUT):
            m_sAbout = aIter.toString();
            m_bHaveAbout = true;
            break;
        case XML_ELEMENT(XHTML, XML_PROPERTY):
            m_sProperty = aIter.toString();
            break;
        case XML_ELEMENT(XHTML, XML_CONTENT):
            m_sContent = aIter.toString();
            break;
        case XML_ELEMENT(XHTML, XML_DATATYPE):
            m_sDatatype = aIter.toString();
            break;
        default:
            XMLMetaImportContextBase::ProcessAttribute(aIter);
    }
}

void XMLMetaImportContext::InsertMeta(
    const Reference<XTextRange> & i_xInsertionRange)
{
    SAL_WARN_IF(m_bHaveAbout == m_sProperty.isEmpty(), "xmloff.text", "XMLMetaImportContext::InsertMeta: invalid RDFa?");
    if (!m_XmlId.isEmpty() || (m_bHaveAbout && !m_sProperty.isEmpty()))
    {
        // insert mark
        const uno::Reference<rdf::XMetadatable> xMeta(
            XMLTextMarkImportContext::CreateAndInsertMark(
                GetImport(),
                u"com.sun.star.text.InContentMetadata"_ustr,
                OUString(),
                i_xInsertionRange, m_XmlId),
            uno::UNO_QUERY);
        SAL_WARN_IF(!xMeta.is(), "xmloff.text", "cannot insert Meta?");

        if (xMeta.is() && m_bHaveAbout)
        {
            GetImport().AddRDFa(xMeta,
                m_sAbout, m_sProperty, m_sContent, m_sDatatype);
        }
    }
    else
    {
        SAL_INFO("xmloff.text", "invalid <text:meta>: no xml:id, no valid RDFa");
    }
}

namespace {

/** text:meta-field */
class XMLMetaFieldImportContext : public XMLMetaImportContextBase
{
    OUString m_DataStyleName;

public:

    XMLMetaFieldImportContext(
        SvXMLImport& i_rImport,
        sal_Int32 nElement,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace );

    virtual void ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter) override;

    virtual void InsertMeta(const Reference<XTextRange> & i_xInsertionRange) override;
};

}

XMLMetaFieldImportContext::XMLMetaFieldImportContext(
        SvXMLImport& i_rImport,
        sal_Int32 nElement,
        XMLHints_Impl& i_rHints,
        bool & i_rIgnoreLeadingSpace )
    : XMLMetaImportContextBase( i_rImport, nElement,
            i_rHints, i_rIgnoreLeadingSpace )
{
}

void XMLMetaFieldImportContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
            m_DataStyleName = aIter.toString();
            break;
        default:
            XMLMetaImportContextBase::ProcessAttribute(aIter);
    }
}

void XMLMetaFieldImportContext::InsertMeta(
    const Reference<XTextRange> & i_xInsertionRange)
{
    if (!m_XmlId.isEmpty()) // valid?
    {
        // insert mark
        const Reference<XPropertySet> xPropertySet(
            XMLTextMarkImportContext::CreateAndInsertMark(
                GetImport(),
                u"com.sun.star.text.textfield.MetadataField"_ustr,
                OUString(),
                i_xInsertionRange, m_XmlId),
            UNO_QUERY);
        SAL_WARN_IF(!xPropertySet.is(), "xmloff.text", "cannot insert MetaField?");
        if (!xPropertySet.is()) return;

        if (!m_DataStyleName.isEmpty())
        {
            bool isDefaultLanguage(true);

            const sal_Int32 nKey( GetImport().GetTextImport()->GetDataStyleKey(
                                   m_DataStyleName, & isDefaultLanguage) );

            if (-1 != nKey)
            {
                OUString sPropertyIsFixedLanguage(u"IsFixedLanguage"_ustr);
                xPropertySet->setPropertyValue(u"NumberFormat"_ustr, Any(nKey));
                if ( xPropertySet->getPropertySetInfo()->
                        hasPropertyByName( sPropertyIsFixedLanguage ) )
                {
                    xPropertySet->setPropertyValue( sPropertyIsFixedLanguage,
                        Any(!isDefaultLanguage) );
                }
            }
        }
    }
    else
    {
        SAL_INFO("xmloff.text", "invalid <text:meta-field>: no xml:id");
    }
}

namespace {

/**
 * Process index marks.
 *
 * All *-mark-end index marks should instantiate *this* class (because
 * it doesn't process attributes other than ID), while the *-mark and
 * *-mark-start classes should instantiate the appropriate subclasses.
 */
class XMLIndexMarkImportContext_Impl : public SvXMLImportContext
{
    XMLHints_Impl& m_rHints;
    OUString sID;

public:

    XMLIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        XMLHints_Impl& rHints);

    void SAL_CALL startFastElement(sal_Int32 nElement, const Reference<xml::sax::XFastAttributeList> & xAttrList) override;

protected:

    /// process all attributes
    void ProcessAttributes(sal_Int32 nElement, const Reference<xml::sax::XFastAttributeList> & xAttrList,
                           Reference<beans::XPropertySet>& rPropSet);

    /**
     * All marks can be created immediately. Since we don't care about
     * the element content, ProcessAttribute should set the properties
     * immediately.
     *
     * This method tolerates an empty PropertySet; subclasses however
     * are not expected to.
     */
    virtual void ProcessAttribute(sal_Int32 nElement,
                                  const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
                                  Reference<beans::XPropertySet>& rPropSet);

    static void GetServiceName(OUString& sServiceName,
                               sal_Int32 nElement);

    bool CreateMark(Reference<beans::XPropertySet>& rPropSet,
                        const OUString& rServiceName);
};

}

XMLIndexMarkImportContext_Impl::XMLIndexMarkImportContext_Impl(
    SvXMLImport& rImport,
    XMLHints_Impl& rHints)
    : SvXMLImportContext(rImport)
    , m_rHints(rHints)
{
}

void XMLIndexMarkImportContext_Impl::startFastElement(
    sal_Int32 nElement,
    const Reference<xml::sax::XFastAttributeList> & xAttrList)
{
    // get Cursor position (needed for all cases)
    Reference<XTextRange> xPos(
        GetImport().GetTextImport()->GetCursor()->getStart());
    Reference<beans::XPropertySet> xMark;

    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_TOC_MARK):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK):
        {
            // single mark: create mark and insert
            OUString sService;
            GetServiceName(sService, nElement);
            if (CreateMark(xMark, sService))
            {
                ProcessAttributes(nElement, xAttrList, xMark);
                m_rHints.push_back(
                    std::make_unique<XMLIndexMarkHint_Impl>(xMark, xPos));
            }
            // else: can't create mark -> ignore
            break;
        }

        case XML_ELEMENT(TEXT, XML_TOC_MARK_START):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_START):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_START):
        {
            // start: create mark and insert (if ID is found)
            OUString sService;
            GetServiceName(sService, nElement);
            if (CreateMark(xMark, sService))
            {
                ProcessAttributes(nElement, xAttrList, xMark);
                if (!sID.isEmpty())
                {
                    // process only if we find an ID
                    m_rHints.push_back(
                        std::make_unique<XMLIndexMarkHint_Impl>(xMark, xPos, sID));
                }
                // else: no ID -> we'll never find the end -> ignore
            }
            // else: can't create mark -> ignore
            break;
        }

        case XML_ELEMENT(TEXT, XML_TOC_MARK_END):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_END):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_END):
        {
            // end: search for ID and set end of mark

            // call process attributes with empty XPropertySet:
            ProcessAttributes(nElement, xAttrList, xMark);
            if (!sID.isEmpty())
            {
                // if we have an ID, find the hint and set the end position
                XMLIndexMarkHint_Impl *const pHint = m_rHints.GetIndexHintById(sID);
                if (pHint)
                    // set end and stop searching
                    pHint->SetEnd(xPos);
            }
            // else: no ID -> ignore
            break;
        }

        default:
            SAL_WARN("xmloff.text", "unknown index mark type!");
            break;
    }
}

void XMLIndexMarkImportContext_Impl::ProcessAttributes(
    sal_Int32 nElement,
    const Reference<xml::sax::XFastAttributeList> & xAttrList,
    Reference<beans::XPropertySet>& rPropSet)
{
    // process attributes
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        ProcessAttribute(nElement, aIter, rPropSet);
    }
}

void XMLIndexMarkImportContext_Impl::ProcessAttribute(
    sal_Int32 nElement,
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
    Reference<beans::XPropertySet>& rPropSet)
{
    // we only know ID + string-value attribute;
    // (former: marks, latter: -start + -end-marks)
    // the remainder is handled in sub-classes
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_TOC_MARK):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK):
            if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_STRING_VALUE) )
            {
                rPropSet->setPropertyValue(u"AlternativeText"_ustr, uno::Any(aIter.toString()));
            }
            // else: ignore!
            break;

        case XML_ELEMENT(TEXT, XML_TOC_MARK_START):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_START):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_START):
        case XML_ELEMENT(TEXT, XML_TOC_MARK_END):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_END):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_END):
            if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_ID) )
            {
                sID = aIter.toString();
            }
            // else: ignore
            break;

        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
            break;
    }
}


void XMLIndexMarkImportContext_Impl::GetServiceName(
    OUString& sServiceName,
    sal_Int32 nElement)
{
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_TOC_MARK):
        case XML_ELEMENT(TEXT, XML_TOC_MARK_START):
        case XML_ELEMENT(TEXT, XML_TOC_MARK_END):
        {
            sServiceName = "com.sun.star.text.ContentIndexMark";
            break;
        }

        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_START):
        case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_END):
        {
            sServiceName = "com.sun.star.text.UserIndexMark";
            break;
        }

        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_START):
        case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_END):
        {
            sServiceName = "com.sun.star.text.DocumentIndexMark";
            break;
        }

        default:
        {
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
            sServiceName.clear();
            break;
        }
    }
}

bool XMLIndexMarkImportContext_Impl::CreateMark(
    Reference<beans::XPropertySet>& rPropSet,
    const OUString& rServiceName)
{
    Reference<lang::XMultiServiceFactory>
        xFactory(GetImport().GetModel(), UNO_QUERY);

    if( xFactory.is() )
    {
        Reference<beans::XPropertySet> xPropSet( xFactory->createInstance(rServiceName), UNO_QUERY );
        if (xPropSet.is())
            rPropSet = xPropSet;
        return true;
    }

    return false;
}

namespace {

class XMLTOCMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
public:

    XMLTOCMarkImportContext_Impl(
        SvXMLImport& rImport,
        XMLHints_Impl& rHints);

protected:

    /** process outline level */
    virtual void ProcessAttribute(sal_Int32 nElement,
                                  const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
                                  Reference<beans::XPropertySet>& rPropSet) override;
};

}

XMLTOCMarkImportContext_Impl::XMLTOCMarkImportContext_Impl(
    SvXMLImport& rImport, XMLHints_Impl& rHints) :
        XMLIndexMarkImportContext_Impl(rImport, rHints)
{
}

void XMLTOCMarkImportContext_Impl::ProcessAttribute(
    sal_Int32 nElement,
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
    Reference<beans::XPropertySet>& rPropSet)
{
    SAL_WARN_IF(!rPropSet.is(), "xmloff.text", "need PropertySet");

    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL):
        {
            // outline level: set Level property
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber( nTmp, aIter.toView() )
                && nTmp >= 1
                && nTmp < GetImport().GetTextImport()->
                                GetChapterNumbering()->getCount() )
            {
                rPropSet->setPropertyValue(u"Level"_ustr, uno::Any(static_cast<sal_Int16>(nTmp - 1)));
            }
            // else: value out of range -> ignore
            break;
        }
        default:
            // else: delegate to superclass
            XMLIndexMarkImportContext_Impl::ProcessAttribute(
                nElement, aIter, rPropSet);
    }
}

namespace {

class XMLUserIndexMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
public:

    XMLUserIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        XMLHints_Impl& rHints);

protected:

    /** process index name */
    virtual void ProcessAttribute(sal_Int32 nElement,
                                  const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
                                  Reference<beans::XPropertySet>& rPropSet) override;
};

}

XMLUserIndexMarkImportContext_Impl::XMLUserIndexMarkImportContext_Impl(
    SvXMLImport& rImport, XMLHints_Impl& rHints) :
        XMLIndexMarkImportContext_Impl(rImport, rHints)
{
}

void XMLUserIndexMarkImportContext_Impl::ProcessAttribute(
    sal_Int32 nElement,
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
    Reference<beans::XPropertySet>& rPropSet)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_INDEX_NAME):
            rPropSet->setPropertyValue(u"UserIndexName"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL):
        {
            // outline level: set Level property
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(
                nTmp, aIter.toView(), 0,
               GetImport().GetTextImport()->GetChapterNumbering()->getCount()))
            {
                rPropSet->setPropertyValue(u"Level"_ustr, uno::Any(static_cast<sal_Int16>(nTmp - 1)));
            }
            // else: value out of range -> ignore
            break;
        }
        default:
            // else: unknown text property: delegate to super class
            XMLIndexMarkImportContext_Impl::ProcessAttribute(
                nElement, aIter, rPropSet);
    }
}

namespace {

class XMLAlphaIndexMarkImportContext_Impl : public XMLIndexMarkImportContext_Impl
{
public:

    XMLAlphaIndexMarkImportContext_Impl(
        SvXMLImport& rImport,
        XMLHints_Impl& rHints);

protected:

    /** process primary + secondary keys */
    virtual void ProcessAttribute(sal_Int32 nElement,
                                  const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
                                  Reference<beans::XPropertySet>& rPropSet) override;
};

}

XMLAlphaIndexMarkImportContext_Impl::XMLAlphaIndexMarkImportContext_Impl(
    SvXMLImport& rImport, XMLHints_Impl& rHints) :
        XMLIndexMarkImportContext_Impl(rImport, rHints)
{
}

void XMLAlphaIndexMarkImportContext_Impl::ProcessAttribute(
    sal_Int32 nElement,
    const sax_fastparser::FastAttributeList::FastAttributeIter & aIter,
    Reference<beans::XPropertySet>& rPropSet)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_KEY1):
            rPropSet->setPropertyValue(u"PrimaryKey"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_KEY2):
            rPropSet->setPropertyValue(u"SecondaryKey"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_KEY1_PHONETIC):
            rPropSet->setPropertyValue(u"PrimaryKeyReading"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_KEY2_PHONETIC):
            rPropSet->setPropertyValue(u"SecondaryKeyReading"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_STRING_VALUE_PHONETIC):
            rPropSet->setPropertyValue(u"TextReading"_ustr, uno::Any(aIter.toString()));
            break;
        case XML_ELEMENT(TEXT, XML_MAIN_ENTRY):
        {
            bool bMainEntry = false;
            bool bTmp(false);

            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                bMainEntry = bTmp;

            rPropSet->setPropertyValue(u"IsMainEntry"_ustr, uno::Any(bMainEntry));
            break;
        }
        default:
            XMLIndexMarkImportContext_Impl::ProcessAttribute(
                nElement, aIter, rPropSet);
    }
}


XMLImpSpanContext_Impl::XMLImpSpanContext_Impl(
        SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        XMLHints_Impl& rHints,
        bool& rIgnLeadSpace,
        sal_uInt8 nSFConvFlags)
:   SvXMLImportContext( rImport )
,   m_rHints( rHints )
,   pHint( nullptr  )
,   rIgnoreLeadingSpace( rIgnLeadSpace )
,   nStarFontsConvFlags( nSFConvFlags & (CONV_FROM_STAR_BATS|CONV_FROM_STAR_MATH) )
{
    OUString aStyleName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(TEXT, XML_STYLE_NAME) )
        {
            aStyleName = aIter.toString();
            break;
        }
    }

    if( !aStyleName.isEmpty() )
    {
        pHint = new XMLStyleHint_Impl( aStyleName,
                  GetImport().GetTextImport()->GetCursorAsRange()->getStart() );
        m_rHints.push_back(std::unique_ptr<XMLStyleHint_Impl>(pHint));
    }
}

void XMLImpSpanContext_Impl::endFastElement(sal_Int32 )
{
    if (!pHint)
        return;

    Reference<XTextRange> xCrsrRange(GetImport().GetTextImport()->GetCursorAsRange());
    if (!xCrsrRange.is())
        return; // Robust (defective file)

    pHint->SetEnd(xCrsrRange->getStart());
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImpSpanContext_Impl::CreateSpanContext(
        SvXMLImport& rImport,
        sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList,
        XMLHints_Impl& rHints,
        bool& rIgnoreLeadingSpace,
        sal_uInt8 nStarFontsConvFlags
     )
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
    case XML_ELEMENT(TEXT, XML_SPAN):
        pContext = new XMLImpSpanContext_Impl( rImport, nElement,
                                               xAttrList,
                                               rHints,
                                               rIgnoreLeadingSpace
                                               ,nStarFontsConvFlags
                                             );
        break;

    case XML_ELEMENT(TEXT, XML_TAB):
        pContext = new XMLCharContext( rImport, xAttrList,
                                               0x0009, false );
        rIgnoreLeadingSpace = false;
        break;

    case XML_ELEMENT(TEXT, XML_LINE_BREAK):
        if (xAttrList->hasAttribute(XML_ELEMENT(LO_EXT, XML_CLEAR)))
        {
            pContext = new SvXMLLineBreakContext(rImport, *rImport.GetTextImport());
        }
        else
        {
            pContext = new XMLCharContext(rImport, ControlCharacter::LINE_BREAK);
        }
        rIgnoreLeadingSpace = false;
        break;

    case XML_ELEMENT(TEXT, XML_S):
        pContext = new XMLCharContext( rImport, xAttrList, 0x0020, true );
        rIgnoreLeadingSpace = false;
        break;

    case XML_ELEMENT(TEXT, XML_A):
    {
        // test for HyperLinkURL property. If present, insert link as
        // text property (StarWriter), else try to insert as text
        // field (StarCalc, StarDraw, ...)
        Reference< beans::XPropertySet > xPropSet( rImport.GetTextImport()->GetCursor(), UNO_QUERY );

        if ( xPropSet->getPropertySetInfo()->hasPropertyByName( u"HyperLinkURL"_ustr ) )
        {
            pContext = new XMLImpHyperlinkContext_Impl(
                    rImport,
                    nElement,
                    xAttrList,
                    rHints,
                    rIgnoreLeadingSpace );
        }
        else
        {
            pContext = new XMLUrlFieldImportContext(rImport, *rImport.GetTextImport());
            //whitespace handling like other fields
            rIgnoreLeadingSpace = false;

        }
        break;
    }

    case XML_ELEMENT(TEXT, XML_RUBY):
        pContext = new XMLImpRubyContext_Impl( rImport, nElement,
                                               xAttrList,
                                               rHints,
                                               rIgnoreLeadingSpace );
        break;

    case XML_ELEMENT(TEXT, XML_NOTE):
        if (rImport.GetTextImport()->IsInFrame())
        {
            // we must not insert footnotes into text frames
            pContext = new SvXMLImportContext( rImport );
        }
        else
        {
            pContext = new XMLFootnoteImportContext(rImport, *rImport.GetTextImport());
        }
        rIgnoreLeadingSpace = false;
        break;

    case XML_ELEMENT(TEXT, XML_REFERENCE_MARK):
    case XML_ELEMENT(TEXT, XML_BOOKMARK):
    case XML_ELEMENT(TEXT, XML_BOOKMARK_START):
    case XML_ELEMENT(TEXT, XML_BOOKMARK_END):
        pContext = new XMLTextMarkImportContext(rImport, *rImport.GetTextImport(),
                                                rHints.GetCrossRefHeadingBookmark());
        break;

    case XML_ELEMENT(FIELD, XML_FIELDMARK):
    case XML_ELEMENT(FIELD, XML_FIELDMARK_START):
    case XML_ELEMENT(FIELD, XML_FIELDMARK_SEPARATOR):
    case XML_ELEMENT(FIELD, XML_FIELDMARK_END):
        pContext = new XMLTextMarkImportContext(rImport, *rImport.GetTextImport(),
                                                rHints.GetCrossRefHeadingBookmark());
        break;

    case XML_ELEMENT(TEXT, XML_REFERENCE_MARK_START):
        pContext = new XMLStartReferenceContext_Impl( rImport,
                                                      rHints, xAttrList );
        break;

    case XML_ELEMENT(TEXT, XML_REFERENCE_MARK_END):
        pContext = new XMLEndReferenceContext_Impl( rImport,
                                                    rHints, xAttrList );
        break;

    case XML_ELEMENT(DRAW, XML_FRAME):
        {
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            XMLTextFrameContext *pTextFrameContext =
                            new XMLTextFrameContext(rImport,
                                         xAttrList,
                                         TextContentAnchorType_AS_CHARACTER );
            // Remove check for text content. (#i33242#)
            // Check for text content is done on the processing of the hint
            if( TextContentAnchorType_AT_CHARACTER ==
                                            pTextFrameContext->GetAnchorType() )
            {
                rHints.push_back(std::make_unique<XMLTextFrameHint_Impl>(
                                    pTextFrameContext, xAnchorPos));
            }
            pContext = pTextFrameContext;
            rIgnoreLeadingSpace = false;
        }
        break;
    case XML_ELEMENT(DRAW, XML_A):
        {
            Reference < XTextRange > xAnchorPos(rImport.GetTextImport()->GetCursor()->getStart());
            pContext =
                new XMLTextFrameHyperlinkContext( rImport, nElement,
                                        xAttrList,
                                        TextContentAnchorType_AS_CHARACTER );
            rHints.push_back(
                std::make_unique<XMLTextFrameHint_Impl>(pContext, xAnchorPos));
        }
        break;

    case XML_ELEMENT(TEXT, XML_TOC_MARK):
    case XML_ELEMENT(TEXT, XML_TOC_MARK_START):
        pContext = new XMLTOCMarkImportContext_Impl(
            rImport, rHints);
        break;

    case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK):
    case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_START):
        pContext = new XMLUserIndexMarkImportContext_Impl(
            rImport, rHints);
        break;

    case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK):
    case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_START):
        pContext = new XMLAlphaIndexMarkImportContext_Impl(
            rImport, rHints);
        break;

    case XML_ELEMENT(TEXT, XML_TOC_MARK_END):
    case XML_ELEMENT(TEXT, XML_USER_INDEX_MARK_END):
    case XML_ELEMENT(TEXT, XML_ALPHABETICAL_INDEX_MARK_END):
        pContext = new XMLIndexMarkImportContext_Impl(
            rImport, rHints);
        break;

    case XML_ELEMENT(TEXT, XML_CHANGE_START):
    case XML_ELEMENT(TEXT, XML_CHANGE_END):
    case XML_ELEMENT(TEXT, XML_CHANGE):
        pContext = new XMLChangeImportContext(
            rImport,
            ((nElement == XML_ELEMENT(TEXT, XML_CHANGE_END))
                ? XMLChangeImportContext::Element::END
                : (nElement == XML_ELEMENT(TEXT, XML_CHANGE_START))
                    ? XMLChangeImportContext::Element::START
                    : XMLChangeImportContext::Element::POINT),
            false);
        break;

    case  XML_ELEMENT(TEXT, XML_META):
        pContext = new XMLMetaImportContext(rImport, nElement,
            rHints, rIgnoreLeadingSpace );
        break;

    case XML_ELEMENT(TEXT, XML_META_FIELD):
        pContext = new XMLMetaFieldImportContext(rImport, nElement,
            rHints, rIgnoreLeadingSpace );
        break;

    case XML_ELEMENT(LO_EXT, XML_CONTENT_CONTROL):
        pContext = new XMLContentControlContext(rImport, nElement, rHints, rIgnoreLeadingSpace);
        break;

    default:
        // none of the above? then it's probably  a text field!
        pContext = XMLTextFieldImportContext::CreateTextFieldImportContext(
            rImport, *rImport.GetTextImport(), nElement);
        // #108784# import draw elements (except control shapes in headers)
        if( pContext == nullptr &&
            !( rImport.GetTextImport()->IsInHeaderFooter() &&
               nElement == XML_ELEMENT(DRAW, XML_CONTROL ) ) )
        {
            Reference < XShapes > xShapes;
            SvXMLShapeContext* pShapeContext = XMLShapeImportHelper::CreateGroupChildContext(
                rImport, nElement, xAttrList, xShapes );
            pContext = pShapeContext;
            // OD 2004-04-20 #i26791# - keep shape in a text frame hint to
            // adjust its anchor position, if it's at-character anchored
            Reference < XTextRange > xAnchorPos =
                rImport.GetTextImport()->GetCursor()->getStart();
            rHints.push_back(
                std::make_unique<XMLDrawHint_Impl>(pShapeContext, xAnchorPos));
        }
        // Behind fields, shapes and any unknown content blanks aren't ignored
        rIgnoreLeadingSpace = false;
    }

    if (!pContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return pContext;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLImpSpanContext_Impl::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    return CreateSpanContext( GetImport(), nElement, xAttrList,
                               m_rHints, rIgnoreLeadingSpace
                               ,nStarFontsConvFlags
                             );
}

void XMLImpSpanContext_Impl::characters( const OUString& rChars )
{
    OUString sStyleName;
    if( pHint )
        sStyleName = pHint->GetStyleName();
    OUString sChars =
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       false, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, rIgnoreLeadingSpace );
}


XMLParaContext::XMLParaContext(
        SvXMLImport& rImport,
        sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport ),
    xStart( rImport.GetTextImport()->GetCursorAsRange()->getStart() ),
    m_bHaveAbout(false),
    nOutlineLevel( (nElement & TOKEN_MASK) == XML_H ? 1 : -1 ),
    // Lost outline numbering in master document (#i73509#)
    mbOutlineLevelAttrFound( false ),
    mbOutlineContentVisible(true),
    bIgnoreLeadingSpace( true ),
    bHeading( (nElement & TOKEN_MASK) == XML_H ),
    bIsListHeader( false ),
    bIsRestart (false),
    nStartValue(0),
    nStarFontsConvFlags( 0 )
{
    bool bHaveXmlId( false );
    OUString aCondStyleName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(XML, XML_ID):
            m_sXmlId = aIter.toString();
            bHaveXmlId = true;
            break;
        case XML_ELEMENT(XHTML, XML_ABOUT):
            m_sAbout = aIter.toString();
            m_bHaveAbout = true;
            break;
        case XML_ELEMENT(XHTML, XML_PROPERTY):
            m_sProperty = aIter.toString();
            break;
        case XML_ELEMENT(XHTML, XML_CONTENT):
            m_sContent = aIter.toString();
            break;
        case XML_ELEMENT(XHTML, XML_DATATYPE):
            m_sDatatype = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_ID):
            if (!bHaveXmlId) { m_sXmlId = aIter.toString(); }
            break;
        case XML_ELEMENT(TEXT, XML_STYLE_NAME):
            sStyleName = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_COND_STYLE_NAME):
            aCondStyleName = aIter.toString();
            break;
        case XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL):
            {
                sal_Int32 nTmp = aIter.toInt32();
                if( nTmp > 0 )
                {
                    if( nTmp > 127 )
                        nTmp = 127;
                    nOutlineLevel = static_cast<sal_Int8>(nTmp);
                }
                // Lost outline numbering in master document (#i73509#)
                mbOutlineLevelAttrFound = true;
            }
            break;
        case XML_ELEMENT(LO_EXT, XML_OUTLINE_CONTENT_VISIBLE):
            {
                bool bBool(false);
                if (::sax::Converter::convertBool(bBool, aIter.toView()))
                    mbOutlineContentVisible = bBool;
            }
            break;
        case XML_ELEMENT(TEXT, XML_IS_LIST_HEADER):
            {
                bool bBool(false);
                if (::sax::Converter::convertBool(bBool, aIter.toView()))
                    bIsListHeader = bBool;
            }
            break;
        case XML_ELEMENT(TEXT, XML_RESTART_NUMBERING):
            {
                bool bBool(false);
                if (::sax::Converter::convertBool(bBool, aIter.toView()))
                    bIsRestart = bBool;
            }
            break;
        case XML_ELEMENT(TEXT, XML_START_VALUE):
            {
                nStartValue = sal::static_int_cast< sal_Int16 >(aIter.toInt32());
            }
            break;
        case XML_ELEMENT(LO_EXT, XML_MARKER_STYLE_NAME):
            if (auto pStyle = rImport.GetTextImport()->FindAutoCharStyle(aIter.toString()))
                m_aMarkerStyleName = pStyle->GetAutoName();
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !aCondStyleName.isEmpty() )
        sStyleName = aCondStyleName;
}

void XMLParaContext::endFastElement(sal_Int32 )
{
    rtl::Reference < XMLTextImportHelper > xTxtImport(
        GetImport().GetTextImport());
    Reference<XTextRange> xEnd;
    try
    {
        Reference<XTextRange> const xCrsrRange(xTxtImport->GetCursorAsRange());
        if (!xCrsrRange.is())
            return; // Robust (defective file)
        xEnd = xCrsrRange->getStart();
    }
    catch (uno::Exception const&)
    {
        SAL_INFO("xmloff.text", "XMLParaContext: cursor disposed?");
        return;
    }

    // if we have an id set for this paragraph, get a cursor for this
    // paragraph and register it with the given identifier
    // FIXME: this is just temporary, and should be removed when
    // EditEngine paragraphs implement XMetadatable!
    if (!m_sXmlId.isEmpty())
    {
        Reference < XTextCursor > xIdCursor( xTxtImport->GetText()->createTextCursorByRange( xStart ) );
        if( xIdCursor.is() )
        {
            xIdCursor->gotoRange( xEnd, true );
            GetImport().getInterfaceToIdentifierMapper().registerReference(
                m_sXmlId, Reference<XInterface>( xIdCursor, UNO_QUERY ));
        }
    }

    // insert a paragraph break
    xTxtImport->InsertControlCharacter( ControlCharacter::APPEND_PARAGRAPH );

    // create a cursor that select the whole last paragraph
    Reference < XTextCursor > xAttrCursor;
    try {
        xAttrCursor = xTxtImport->GetText()->createTextCursorByRange( xStart );
        if( !xAttrCursor.is() )
            return; // Robust (defective file)
    } catch (const uno::Exception &) {
        // createTextCursorByRange() likes to throw runtime exception, even
        // though it just means 'we were unable to create the cursor'
        return;
    }
    xAttrCursor->gotoRange( xEnd, true );

    // xml:id for RDF metadata
    if (!m_sXmlId.isEmpty() || m_bHaveAbout || !m_sProperty.isEmpty())
    {
        try {
            const uno::Reference<container::XEnumerationAccess> xEA
                (xAttrCursor, uno::UNO_QUERY_THROW);
            const uno::Reference<container::XEnumeration> xEnum(
                xEA->createEnumeration(), uno::UNO_SET_THROW);
            SAL_WARN_IF(!xEnum->hasMoreElements(), "xmloff.text", "xml:id: no paragraph?");
            if (xEnum->hasMoreElements()) {
                uno::Reference<rdf::XMetadatable> xMeta;
                xEnum->nextElement() >>= xMeta;
                SAL_WARN_IF(!xMeta.is(), "xmloff.text", "xml:id: not XMetadatable");
                GetImport().SetXmlId(xMeta, m_sXmlId);
                if (m_bHaveAbout)
                {
                    GetImport().AddRDFa(xMeta,
                        m_sAbout, m_sProperty, m_sContent, m_sDatatype);
                }
                SAL_WARN_IF(xEnum->hasMoreElements(), "xmloff.text", "xml:id: > 1 paragraph?");
            }
        } catch (const uno::Exception &) {
            SAL_INFO("xmloff.text", "XMLParaContext::~XMLParaContext: exception");
        }
    }

    OUString const sCellParaStyleName(xTxtImport->GetCellParaStyleDefault());
    if( !sCellParaStyleName.isEmpty() )
    {
        /* Suppress handling of outline and list attributes,
           because of side effects of method <SetStyleAndAttrs(..)> (#i80724#)
        */
        xTxtImport->SetStyleAndAttrs( GetImport(), xAttrCursor,
                                      sCellParaStyleName,
                                      true,
                                      false, -1, // suppress outline handling
                                      false );   // suppress list attributes handling
    }

    // #103445# for headings without style name, find the proper style
    if( bHeading && sStyleName.isEmpty() )
        xTxtImport->FindOutlineStyleName( sStyleName, nOutlineLevel );

    // set style and hard attributes at the previous paragraph
    // Add parameter <mbOutlineLevelAttrFound> (#i73509#)
    sStyleName = xTxtImport->SetStyleAndAttrs( GetImport(), xAttrCursor,
                                               sStyleName,
                                               true,
                                               mbOutlineLevelAttrFound,
                                               bHeading ? nOutlineLevel : -1,
                                               true,
                                               mbOutlineContentVisible);

    if (m_aMarkerStyleName.hasValue())
    {
        if (auto xPropSet = xStart.query<css::beans::XPropertySet>())
        {
            try
            {
                xPropSet->setPropertyValue(u"ListAutoFormat"_ustr, m_aMarkerStyleName);
            }
            catch (const css::beans::UnknownPropertyException&)
            {
                // no problem
            }
        }
    }

    // handle list style header
    if (bHeading && (bIsListHeader || bIsRestart))
    {
        Reference<XPropertySet> xPropSet( xAttrCursor, UNO_QUERY );

        if (xPropSet.is())
        {
            if (bIsListHeader)
            {
                OUString sNumberingIsNumber
                    (u"NumberingIsNumber"_ustr);
                if(xPropSet->getPropertySetInfo()->
                   hasPropertyByName(sNumberingIsNumber))
                {
                    xPropSet->setPropertyValue
                        (sNumberingIsNumber, Any( false ) );
                }
            }
            if (bIsRestart)
            {
                OUString sParaIsNumberingRestart
                    (u"ParaIsNumberingRestart"_ustr);
                OUString sNumberingStartValue
                    (u"NumberingStartValue"_ustr);
                if (xPropSet->getPropertySetInfo()->
                    hasPropertyByName(sParaIsNumberingRestart))
                {
                    xPropSet->setPropertyValue
                        (sParaIsNumberingRestart, Any(true));
                }

                if (xPropSet->getPropertySetInfo()->
                    hasPropertyByName(sNumberingStartValue))
                {
                    xPropSet->setPropertyValue
                        (sNumberingStartValue, Any(nStartValue));
                }
            }

        }
    }

    if (m_xHints)
    {
        bool bEmptyHints = false;
        if (auto xCompare = xTxtImport->GetText().query<text::XTextRangeCompare>())
        {
            try
            {
                for (const auto& pHint : m_xHints->GetHints())
                {
                    if (xCompare->compareRegionStarts(pHint->GetStart(), pHint->GetEnd()) == 0)
                    {
                        bEmptyHints = true;
                    }
                }
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("xmloff.text", "");
            }
        }
        bool bSetNoFormatAttr = false;
        uno::Reference<beans::XPropertySet> xCursorProps(xAttrCursor, uno::UNO_QUERY);
        if (bEmptyHints || m_aMarkerStyleName.hasValue())
        {
            // We have at least one empty hint, then make try to ask the cursor to not upgrade our character
            // attributes to paragraph-level formatting, which would lead to incorrect rendering.
            uno::Reference<beans::XPropertySetInfo> xCursorPropsInfo = xCursorProps->getPropertySetInfo();
            bSetNoFormatAttr = xCursorPropsInfo->hasPropertyByName(u"NoFormatAttr"_ustr);
        }
        if (bSetNoFormatAttr)
        {
            xCursorProps->setPropertyValue(u"NoFormatAttr"_ustr, uno::Any(true));
        }
        for (const auto & i : m_xHints->GetHints())
        {
            XMLHint_Impl *const pHint = i.get();
            xAttrCursor->gotoRange( pHint->GetStart(), false );
            xAttrCursor->gotoRange( pHint->GetEnd(), true );
            switch( pHint->GetType() )
            {
            case XMLHintType::XML_HINT_STYLE:
                {
                    const OUString& rStyleName =
                            static_cast<XMLStyleHint_Impl *>(pHint)->GetStyleName();
                    if( !rStyleName.isEmpty() )
                        xTxtImport->SetStyleAndAttrs( GetImport(),
                                                      xAttrCursor, rStyleName,
                                                      false );
                }
                break;
            case XMLHintType::XML_HINT_REFERENCE:
                {
                    const OUString& rRefName =
                            static_cast<XMLReferenceHint_Impl *>(pHint)->GetRefName();
                    if( !rRefName.isEmpty() )
                    {
                        if( !pHint->GetEnd().is() )
                            pHint->SetEnd(xEnd);

                        // reference name uses rStyleName member
                        // borrow from XMLTextMarkImportContext
                        XMLTextMarkImportContext::CreateAndInsertMark(
                            GetImport(),
                            u"com.sun.star.text.ReferenceMark"_ustr,
                            rRefName,
                            xAttrCursor);
                    }
                }
                break;
            case XMLHintType::XML_HINT_HYPERLINK:
                {
                    const XMLHyperlinkHint_Impl *pHHint =
                        static_cast<const XMLHyperlinkHint_Impl *>(pHint);
                    xTxtImport->SetHyperlink( GetImport(),
                                              xAttrCursor,
                                              pHHint->GetHRef(),
                                              pHHint->GetName(),
                                              pHHint->GetTargetFrameName(),
                                              pHHint->GetStyleName(),
                                              pHHint->GetVisitedStyleName(),
                                              pHHint->GetEventsContext() );
                }
                break;
            case XMLHintType::XML_HINT_INDEX_MARK:
                {
                    Reference<beans::XPropertySet> xMark(
                        static_cast<const XMLIndexMarkHint_Impl *>(pHint)->GetMark());
                    Reference<XTextContent> xContent(xMark, UNO_QUERY);
                    try
                    {
                        xTxtImport->GetText()->insertTextContent(
                            xAttrCursor, xContent, true );
                    }
                    catch (uno::RuntimeException const&)
                    {
                        TOOLS_INFO_EXCEPTION("xmloff.text", "could not insert index mark, presumably in editengine text");
                    }
                }
                break;
            case XMLHintType::XML_HINT_TEXT_FRAME:
                {
                    const XMLTextFrameHint_Impl *pFHint =
                        static_cast<const XMLTextFrameHint_Impl *>(pHint);
                    // Check for text content (#i33242#)
                    Reference < XTextContent > xTextContent =
                                                    pFHint->GetTextContent();
                    if ( xTextContent.is() )
                    {
                        /* Core impl. of the unification of drawing objects and
                           Writer fly frames (#i26791#)
                        */
                        if ( pFHint->IsBoundAtChar() )
                        {
                            xTextContent->attach( xAttrCursor );
                        }
                    }
                    /* Consider, that hint can also contain a shape -
                       e.g. drawing object of type 'Text'. (#i33242#)
                    */
                    else
                    {
                        Reference < XShape > xShape = pFHint->GetShape();
                        if ( xShape.is() )
                        {
                            // determine anchor type
                            Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );
                            TextContentAnchorType eAnchorType =
                                            TextContentAnchorType_AT_PARAGRAPH;
                            {
                                Any aAny = xPropSet->getPropertyValue( u"AnchorType"_ustr );
                                aAny >>= eAnchorType;
                            }
                            if ( TextContentAnchorType_AT_CHARACTER == eAnchorType )
                            {
                                // set anchor position for at-character anchored objects
                                xPropSet->setPropertyValue(u"TextRange"_ustr, Any(xAttrCursor));
                            }
                        }
                    }
                }
                break;
            /* Core impl. of the unification of drawing objects and
               Writer fly frames (#i26791#)
            */
            case XMLHintType::XML_HINT_DRAW:
                {
                    const XMLDrawHint_Impl *pDHint =
                        static_cast<const XMLDrawHint_Impl*>(pHint);
                    // Improvement: hint directly provides the shape. (#i33242#)
                    const Reference < XShape >& xShape = pDHint->GetShape();
                    if ( xShape.is() )
                    {
                        // determine anchor type
                        Reference < XPropertySet > xPropSet( xShape, UNO_QUERY );
                        TextContentAnchorType eAnchorType = TextContentAnchorType_AT_PARAGRAPH;
                        {
                            Any aAny = xPropSet->getPropertyValue( u"AnchorType"_ustr );
                            aAny >>= eAnchorType;
                        }
                        if ( TextContentAnchorType_AT_CHARACTER == eAnchorType )
                        {
                            // set anchor position for at-character anchored objects
                            xPropSet->setPropertyValue(u"TextRange"_ustr, Any(xAttrCursor));
                        }
                    }
                }
                break;
            default:
                SAL_WARN( "xmloff.text", "What's this" );
                break;
            }
        }
        if (bSetNoFormatAttr)
        {
            xCursorProps->setPropertyValue(u"NoFormatAttr"_ustr, uno::Any(false));
        }
    }
    m_xHints.reset();
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLParaContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if (!m_xHints)
        m_xHints.reset(new XMLHints_Impl);
    return XMLImpSpanContext_Impl::CreateSpanContext(
                                GetImport(), nElement, xAttrList,
                                *m_xHints, bIgnoreLeadingSpace,
                                nStarFontsConvFlags);
}

void XMLParaContext::characters( const OUString& rChars )
{
    OUString sChars =
        GetImport().GetTextImport()->ConvertStarFonts( rChars, sStyleName,
                                                       nStarFontsConvFlags,
                                                       true, GetImport() );
    GetImport().GetTextImport()->InsertString( sChars, bIgnoreLeadingSpace );
}


XMLNumberedParaContext::XMLNumberedParaContext(
        SvXMLImport& i_rImport,
        sal_Int32 /*nElement*/,
        const Reference< xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( i_rImport ),
    m_Level(0),
    m_StartValue(-1)
{
    OUString StyleName;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(XML, XML_ID):
//FIXME: there is no UNO API for lists
                break;
            case XML_ELEMENT(TEXT, XML_LIST_ID):
                m_ListId = aIter.toString();
                break;
            case XML_ELEMENT(TEXT, XML_LEVEL):
                {
                    sal_Int32 nTmp = aIter.toInt32();
                    if ( nTmp >= 1 && nTmp <= SHRT_MAX ) {
                        m_Level = static_cast<sal_uInt16>(nTmp) - 1;
                    }
                }
                break;
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                StyleName = aIter.toString();
                break;
            case XML_ELEMENT(TEXT, XML_CONTINUE_NUMBERING):
                // this attribute is deprecated
//                ContinueNumbering = IsXMLToken(sValue, XML_TRUE);
                break;
            case XML_ELEMENT(TEXT, XML_START_VALUE):
                {
                    sal_Int32 nTmp = aIter.toInt32();
                    if ( nTmp >= 0 && nTmp <= SHRT_MAX ) {
                        m_StartValue = static_cast<sal_Int16>(nTmp);
                    }
                }
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    XMLTextListsHelper& rTextListsHelper(
        i_rImport.GetTextImport()->GetTextListHelper() );
    if (m_ListId.isEmpty())
    {
        SAL_WARN_IF(0 <= i_rImport.GetODFVersion().compareTo(u"1.2"), "xmloff.text", "invalid numbered-paragraph: no list-id (1.2)");
        m_ListId = rTextListsHelper.GetNumberedParagraphListId(m_Level,
            StyleName);
        SAL_WARN_IF(m_ListId.isEmpty(), "xmloff.text", "numbered-paragraph: no ListId");
        if (m_ListId.isEmpty()) {
            return;
        }
    }
    m_xNumRules = rTextListsHelper.EnsureNumberedParagraph( i_rImport,
        m_ListId, m_Level, StyleName);

    SAL_WARN_IF(!m_xNumRules.is(), "xmloff.text", "numbered-paragraph: no NumRules");

    i_rImport.GetTextImport()->GetTextListHelper().PushListContext( this );
}

void XMLNumberedParaContext::endFastElement(sal_Int32 )
{
    if (!m_ListId.isEmpty()) {
        GetImport().GetTextImport()->PopListContext();
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLNumberedParaContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_H):
        case XML_ELEMENT(LO_EXT, XML_H):
        case XML_ELEMENT(TEXT, XML_P):
        case XML_ELEMENT(LO_EXT, XML_P):
            return new XMLParaContext( GetImport(), nElement, xAttrList );
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
