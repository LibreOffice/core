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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_CONDFORMATCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_CONDFORMATCONTEXT_HXX

#include "condformatbuffer.hxx"
#include "excelhandlers.hxx"

class IconSetRule;

namespace oox {
namespace xls {

class CondFormatContext;

class ColorScaleContext : public WorksheetContextBase
{
public:
    explicit ColorScaleContext( CondFormatContext& rFragment, CondFormatRuleRef xRule );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;

private:
    CondFormatRuleRef mxRule;
};

class DataBarContext : public WorksheetContextBase
{
public:
    explicit DataBarContext( CondFormatContext& rFormat, CondFormatRuleRef xRule );

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;

private:
    CondFormatRuleRef mxRule;
};

class IconSetContext : public WorksheetContextBase
{
public:
    explicit IconSetContext( WorksheetContextBase& rParent, IconSetRule* pIconSet );

    virtual oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void onCharacters(const OUString& rChars) SAL_OVERRIDE;
    virtual void onEndElement() SAL_OVERRIDE;

private:
    IconSetRule* mpIconSet;
    OUString maChars;
};

class CondFormatContext : public WorksheetContextBase
{
public:
    explicit            CondFormatContext( WorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onStartElement( const AttributeList& rAttribs ) SAL_OVERRIDE;
    virtual void        onCharacters( const OUString& rChars ) SAL_OVERRIDE;
    virtual void        onEndElement() SAL_OVERRIDE;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) SAL_OVERRIDE;
    virtual void        onStartRecord( SequenceInputStream& rStrm ) SAL_OVERRIDE;
    virtual void        onEndRecord() SAL_OVERRIDE;

private:
    CondFormatRef       mxCondFmt;
    CondFormatRuleRef   mxRule;
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
