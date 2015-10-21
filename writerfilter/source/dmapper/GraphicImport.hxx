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
#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_GRAPHICIMPORT_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_GRAPHICIMPORT_HXX

#include <queue>
#include <memory>

#include "LoggedResources.hxx"

namespace com { namespace sun { namespace star {
    namespace uno
    {
        class XComponentContext;
    }
    namespace lang
    {
        class XMultiServiceFactory;
    }
    namespace text
    {
        class XTextContent;
    }
    namespace drawing
    {
        class XShape;
    }
    namespace beans
    {
        struct PropertyValue;
    }
}}}

namespace writerfilter
{
namespace dmapper
{
class GraphicImport_Impl;
class DomainMapper;

enum GraphicImportType
{
    IMPORT_AS_DETECTED_INLINE,
    IMPORT_AS_DETECTED_ANCHOR
};

class GraphicImport : public LoggedProperties, public LoggedTable
                    ,public BinaryObj, public LoggedStream
{
    std::unique_ptr<GraphicImport_Impl> m_pImpl;

    css::uno::Reference<css::uno::XComponentContext>     m_xComponentContext;
    css::uno::Reference<css::lang::XMultiServiceFactory> m_xTextFactory;

    css::uno::Reference<css::text::XTextContent> m_xGraphicObject;

    css::uno::Reference<css::drawing::XShape> m_xShape;
    void ProcessShapeOptions(Value & val);

    css::uno::Reference<css::text::XTextContent > createGraphicObject(const css::beans::PropertyValues& aMediaProperties, const css::uno::Reference<css::beans::XPropertySet>& xShapeProps);

    void putPropertyToFrameGrabBag( const OUString& sPropertyName, const css::uno::Any& aPropertyValue );

public:
    explicit GraphicImport( css::uno::Reference<css::uno::XComponentContext> const& xComponentContext,
                            css::uno::Reference<css::lang::XMultiServiceFactory> const& xTextFactory,
                            DomainMapper& rDomainMapper,
                            GraphicImportType eGraphicImportType,
                            std::pair<OUString, OUString>& rPositionOffsets,
                            std::pair<OUString, OUString>& rAligns,
                            std::queue<OUString>& rPositivePercentages);
    virtual ~GraphicImport();

    // BinaryObj
    virtual void data(const sal_uInt8* buffer, size_t len, writerfilter::Reference<Properties>::Pointer_t ref) override;

    css::uno::Reference<css::text::XTextContent> GetGraphicObject();
    css::uno::Reference<css::drawing::XShape> GetXShapeObject() { return m_xShape;}
    bool IsGraphic() const;

 private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) override;

    // Stream
    virtual void lcl_startSectionGroup() override;
    virtual void lcl_endSectionGroup() override;
    virtual void lcl_startParagraphGroup() override;
    virtual void lcl_endParagraphGroup() override;
    virtual void lcl_startCharacterGroup() override;
    virtual void lcl_endCharacterGroup() override;
    virtual void lcl_text(const sal_uInt8 * data, size_t len) override;
    virtual void lcl_utext(const sal_uInt8 * data, size_t len) override;
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) override;
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref) override;
    virtual void lcl_substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) override;
    virtual void lcl_info(const std::string & info) override;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) override;
    virtual void lcl_endShape() override;

    void handleWrapTextValue(sal_uInt32 nVal);
};

typedef std::shared_ptr<GraphicImport> GraphicImportPtr;

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
