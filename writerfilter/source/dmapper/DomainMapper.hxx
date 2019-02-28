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
#include "PropertyMap.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/TabAlign.hpp>

#include <map>
#include <vector>
#include <memory>

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
class GraphicNamingHelper;

typedef tools::SvRef<StyleSheetTable> StyleSheetTablePtr;

class DomainMapper : public LoggedProperties, public LoggedTable,
                    public BinaryObj, public LoggedStream
{
    std::unique_ptr<DomainMapper_Impl> m_pImpl;

public:
    DomainMapper(const css::uno::Reference<css::uno::XComponentContext>& xContext,
                 css::uno::Reference<css::io::XInputStream> const& xInputStream,
                 css::uno::Reference<css::lang::XComponent> const& xModel,
                 bool bRepairStorage,
                 SourceDocumentType eDocumentType,
                 utl::MediaDescriptor const & rMediaDesc);
    virtual ~DomainMapper() override;

    // Stream
    virtual void markLastParagraphInSection() override;
    virtual void markLastSectionGroup() override;

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref) override;

    void sprmWithProps( Sprm& sprm, const PropertyMapPtr& pContext );

    void PushStyleSheetProperties( const PropertyMapPtr& pStyleProperties, bool bAffectTableMngr = false );
    void PopStyleSheetProperties( bool bAffectTableMngr = false );

    void PushListProperties( const ::tools::SvRef<PropertyMap>& pListProperties );
    void PopListProperties();

    bool IsOOXMLImport() const;
    bool IsRTFImport() const;
    css::uno::Reference<css::lang::XMultiServiceFactory> const & GetTextFactory() const;
    css::uno::Reference<css::text::XTextRange> GetCurrentTextRange();

    OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties, bool bAlwaysCreate );
    StyleSheetTablePtr const & GetStyleSheetTable( );
    GraphicZOrderHelper* graphicZOrderHelper();
    GraphicNamingHelper& GetGraphicNamingHelper();

    /// Return the first from the pending (not inserted to the document) shapes, if there are any.
    css::uno::Reference<css::drawing::XShape> PopPendingShape();

    bool IsInHeaderFooter() const;
    bool IsInTable() const;
    bool IsStyleSheetImport() const;

    void hasControls( const bool bSet ) { mbHasControls = bSet; }

    /**
     @see DomainMapper_Impl::processDeferredCharacterProperties()
    */
    void processDeferredCharacterProperties(const std::map<sal_Int32, css::uno::Any>& rDeferredCharacterProperties);
    void setInTableStyleRunProps(bool bInTableStyleRunProps);

    /// Enable storing of seen tokens in a named grab bag.
    void enableInteropGrabBag(const OUString& aName);
    /// Get the stored tokens and clear the internal storage.
    css::beans::PropertyValue getInteropGrabBag();

    void HandleRedline( Sprm& rSprm );

private:
    // Stream
    virtual void lcl_startSectionGroup() override;
    virtual void lcl_endSectionGroup() override;
    virtual void lcl_startParagraphGroup() override;
    virtual void lcl_endParagraphGroup() override;
    virtual void lcl_startCharacterGroup() override;
    virtual void lcl_endCharacterGroup() override;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) override;
    virtual void lcl_endShape( ) override;

    virtual void lcl_text(const sal_uInt8 * data, size_t len) override;
    virtual void lcl_utext(const sal_uInt8 * data, size_t len) override;
    virtual void lcl_positionOffset(const OUString& rText, bool bVertical) override;
    virtual css::awt::Point getPositionOffset() override;
    virtual void lcl_align(const OUString& rText, bool bVertical) override;
    virtual void lcl_positivePercentage(const OUString& rText) override;
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) override;
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref) override;
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref) override;
    virtual void lcl_info(const std::string & info) override;
    virtual void lcl_startGlossaryEntry() override;
    virtual void lcl_endGlossaryEntry() override;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) override;

    static void handleUnderlineType(const Id nId, const ::tools::SvRef<PropertyMap>& rContext);
    void handleParaJustification(const sal_Int32 nIntValue, const ::tools::SvRef<PropertyMap>& rContext, const bool bExchangeLeftRight);
    static bool getColorFromId(const Id, sal_Int32 &nColor);
    static sal_Int16 getEmphasisValue(const sal_Int32 nIntValue);
    static OUString getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix = true);
    static css::style::TabAlign getTabAlignFromValue(const sal_Int32 nIntValue);
    static sal_Unicode getFillCharFromValue(const sal_Int32 nIntValue);
    bool mbIsSplitPara;
    bool mbHasControls;
    std::unique_ptr< GraphicZOrderHelper > zOrderHelper;
    std::unique_ptr<GraphicNamingHelper> m_pGraphicNamingHelper;
    OUString m_sGlossaryEntryName;
};

} // namespace dmapper
} // namespace writerfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
