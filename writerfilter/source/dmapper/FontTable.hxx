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

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_FONTTABLE_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_FONTTABLE_HXX

#include <memory>
#include "LoggedResources.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XInputStream.hpp>

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl;
struct FontEntry : public virtual SvRefBase
{
    typedef tools::SvRef<FontEntry> Pointer_t;

    OUString        sFontName;
    sal_Int32       nTextEncoding;
    FontEntry() :
        nTextEncoding( RTL_TEXTENCODING_DONTKNOW )
        {}
};

class FontTable : public LoggedProperties, public LoggedTable
    /*,public BinaryObj*/, public LoggedStream
{
    std::unique_ptr<FontTable_Impl> m_pImpl;

 public:
    FontTable();
    virtual ~FontTable() override;

    sal_uInt32          size();
    const FontEntry::Pointer_t  getFontEntry(sal_uInt32 nIndex);

 private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val) override;
    virtual void lcl_sprm(Sprm & sprm) override;
    void resolveSprm(Sprm & r_sprm);

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
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref) override;
    virtual void lcl_info(const std::string & info) override;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) override;
    virtual void lcl_endShape( ) override;

};
typedef tools::SvRef< FontTable >          FontTablePtr;

class EmbeddedFontHandler : public LoggedProperties
{
public:
    EmbeddedFontHandler( const OUString& fontName, const char* style );
    virtual ~EmbeddedFontHandler() override;
private:
    virtual void lcl_attribute( Id name, Value& val ) override;
    virtual void lcl_sprm( Sprm& rSprm ) override;
    OUString const fontName;
    const char* const style;
    OUString fontKey;
    css::uno::Reference<css::io::XInputStream> inputStream;
};


}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
