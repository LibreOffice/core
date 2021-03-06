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

#pragma once

#include "condformatbuffer.hxx"
#include "excelhandlers.hxx"

namespace oox::xls {

class CondFormatContext;

class ColorScaleContext : public WorksheetContextBase
{
public:
    explicit ColorScaleContext( CondFormatContext& rFragment, CondFormatRuleRef const & xRule );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

private:
    CondFormatRuleRef mxRule;
};

class DataBarContext : public WorksheetContextBase
{
public:
    explicit DataBarContext( CondFormatContext& rFormat, CondFormatRuleRef const & xRule );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;

private:
    CondFormatRuleRef mxRule;
};

class IconSetContext : public WorksheetContextBase
{
public:
    explicit IconSetContext( WorksheetContextBase& rParent, IconSetRule* pIconSet );

    virtual oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void onStartElement( const AttributeList& rAttribs ) override;
    virtual void onCharacters(const OUString& rChars) override;
    virtual void onEndElement() override;

private:
    IconSetRule* mpIconSet;
    OUString maChars;
};

class CondFormatContext : public WorksheetContextBase
{
public:
    explicit            CondFormatContext( WorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) override;
    virtual void        onEndRecord() override;

private:
    CondFormatRef       mxCondFmt;
    CondFormatRuleRef   mxRule;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
