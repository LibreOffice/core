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

#include <utility>

#include "excelhandlers.hxx"
#include "stylesbuffer.hxx"

namespace oox::xls {

class IndexedColorsContext : public WorkbookContextBase
{
public:
    explicit            IndexedColorsContext( WorkbookFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
};

class FontContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit     FontContext( ParentType& rParent, FontRef xFont ) :
                            WorkbookContextBase( rParent ), mxFont(std::move( xFont )) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    FontRef             mxFont;
};

class BorderContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit     BorderContext( ParentType& rParent, BorderRef xBorder ) :
                            WorkbookContextBase( rParent ), mxBorder(std::move( xBorder )) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    BorderRef           mxBorder;
};

class FillContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit     FillContext( ParentType& rParent, FillRef xFill ) :
                            WorkbookContextBase( rParent ), mxFill(std::move( xFill )), mfGradPos( -1.0 ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    FillRef             mxFill;
    double              mfGradPos;      /// Gradient color position.
};

class XfContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit     XfContext( ParentType& rParent, XfRef xXf, bool bCellXf ) :
                            WorkbookContextBase( rParent ), mxXf(std::move( xXf )), mbCellXf( bCellXf ) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    XfRef               mxXf;
    bool                mbCellXf;       /// True = cell XF, false = style XF.
};

class DxfContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    explicit     DxfContext( ParentType& rParent, const DxfRef& rxDxf ) :
                            WorkbookContextBase( rParent ), mxDxf( rxDxf ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    DxfRef              mxDxf;
};

class StylesFragment : public WorkbookFragmentBase
{
public:
    explicit            StylesFragment(
                            const WorkbookHelper& rHelper,
                            const OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;
    virtual void        finalizeImport() override;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
