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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_DOMAINMAPPER_HXX

#include <dmapper/DomainMapperFactory.hxx>
#include "LoggedResources.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/TabAlign.hpp>

#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>

namespace com{ namespace sun {namespace star{
    namespace beans{
        struct PropertyValue;
    }
    namespace io{
        class XInputStream;
    }
    namespace uno{
        class XComponentContext;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace text{
        class XTextRange;
    }
}}}

namespace utl
{
class MediaDescriptor;
}

typedef std::vector<css::beans::PropertyValue> PropertyValueVector_t;

namespace writerfilter {
namespace dmapper
{

class PropertyMap;
class DomainMapper_Impl;
class ListsManager;
class StyleSheetTable;
class GraphicZOrderHelper;

// different context types require different sprm handling (e.g. names)
enum SprmType
{
    SPRM_DEFAULT,
    SPRM_LIST
};
class DomainMapper : public LoggedProperties, public LoggedTable,
                    public BinaryObj, public LoggedStream
{
    DomainMapper_Impl   *m_pImpl;

public:
    DomainMapper(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                 css::uno::Reference<css::io::XInputStream> const& xInputStream,
                 css::uno::Reference<css::lang::XComponent> const& xModel,
                 bool bRepairStorage,
                 SourceDocumentType eDocumentType,
                 css::uno::Reference<css::text::XTextRange> const& xInsertTextRange,
                 utl::MediaDescriptor& rMediaDesc);
    virtual ~DomainMapper();

    // Stream
    virtual void markLastParagraphInSection() SAL_OVERRIDE;
    virtual void markLastSectionGroup() SAL_OVERRIDE;

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;

    void sprmWithProps( Sprm& sprm, ::std::shared_ptr<PropertyMap> pContext );

    void PushStyleSheetProperties( ::std::shared_ptr<PropertyMap> pStyleProperties, bool bAffectTableMngr = false );
    void PopStyleSheetProperties( bool bAffectTableMngr = false );

    void PushListProperties( ::std::shared_ptr<PropertyMap> pListProperties );
    void PopListProperties();

    bool IsOOXMLImport() const;
    bool IsRTFImport() const;
    css::uno::Reference<css::lang::XMultiServiceFactory> GetTextFactory() const;
    void  AddListIDToLFOTable( sal_Int32 nAbstractNumId );
    css::uno::Reference<css::text::XTextRange> GetCurrentTextRange();

    OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );
    std::shared_ptr< StyleSheetTable > GetStyleSheetTable( );
    GraphicZOrderHelper* graphicZOrderHelper();

    /// Return the first from the pending (not inserted to the document) shapes, if there are any.
    css::uno::Reference<css::drawing::XShape> PopPendingShape();

    bool IsInHeaderFooter() const;
    bool IsStyleSheetImport() const;
    /**
     @see DomainMapper_Impl::processDeferredCharacterProperties()
    */
    void processDeferredCharacterProperties(const std::map<sal_Int32, css::uno::Any>& rDeferredCharacterProperties);
    void setInTableStyleRunProps(bool bInTableStyleRunProps);

    /// Enable storing of seen tokens in a named grab bag.
    void enableInteropGrabBag(const OUString& aName);
    /// Get the stored tokens and clear the internal storage.
    css::beans::PropertyValue getInteropGrabBag();

    css::uno::Sequence<css::beans::PropertyValue> GetThemeFontLangProperties() const;
    css::uno::Sequence<css::beans::PropertyValue> GetCompatSettings() const;

    void HandleRedline( Sprm& rSprm );

private:
    // Stream
    virtual void lcl_startSectionGroup() SAL_OVERRIDE;
    virtual void lcl_endSectionGroup() SAL_OVERRIDE;
    virtual void lcl_startParagraphGroup() SAL_OVERRIDE;
    virtual void lcl_endParagraphGroup() SAL_OVERRIDE;
    virtual void lcl_startCharacterGroup() SAL_OVERRIDE;
    virtual void lcl_endCharacterGroup() SAL_OVERRIDE;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) SAL_OVERRIDE;
    virtual void lcl_endShape( ) SAL_OVERRIDE;

    virtual void lcl_text(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    virtual void lcl_utext(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    virtual void lcl_positionOffset(const OUString& rText, bool bVertical) SAL_OVERRIDE;
    virtual css::awt::Point getPositionOffset() SAL_OVERRIDE;
    virtual void lcl_align(const OUString& rText, bool bVertical) SAL_OVERRIDE;
    virtual void lcl_positivePercentage(const OUString& rText) SAL_OVERRIDE;
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref) SAL_OVERRIDE;
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref) SAL_OVERRIDE;
    virtual void lcl_info(const std::string & info) SAL_OVERRIDE;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) SAL_OVERRIDE;
    virtual void lcl_sprm(Sprm & sprm) SAL_OVERRIDE;

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;

    static void handleUnderlineType(const Id nId, const ::std::shared_ptr<PropertyMap>& rContext);
    void handleParaJustification(const sal_Int32 nIntValue, const ::std::shared_ptr<PropertyMap>& rContext, const bool bExchangeLeftRight);
    static bool getColorFromId(const Id, sal_Int32 &nColor);
    static sal_Int16 getEmphasisValue(const sal_Int32 nIntValue);
    static OUString getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix = true);
    static css::style::TabAlign getTabAlignFromValue(const sal_Int32 nIntValue);
    static sal_Unicode getFillCharFromValue(const sal_Int32 nIntValue);
    bool mbIsSplitPara;
    boost::scoped_ptr< GraphicZOrderHelper > zOrderHelper;
};

}
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
