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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_STYLESFRAGMENT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_STYLESFRAGMENT_HXX

#include "excelhandlers.hxx"
#include "stylesbuffer.hxx"

namespace oox {
namespace xls {

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
    inline explicit     FontContext( ParentType& rParent, const FontRef& rxFont ) :
                            WorkbookContextBase( rParent ), mxFont( rxFont ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;

private:
    FontRef             mxFont;
};

class BorderContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     BorderContext( ParentType& rParent, const BorderRef& rxBorder ) :
                            WorkbookContextBase( rParent ), mxBorder( rxBorder ) {}

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
    inline explicit     FillContext( ParentType& rParent, const FillRef& rxFill ) :
                            WorkbookContextBase( rParent ), mxFill( rxFill ), mfGradPos( -1.0 ) {}

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
    inline explicit     XfContext( ParentType& rParent, const XfRef& rxXf, bool bCellXf ) :
                            WorkbookContextBase( rParent ), mxXf( rxXf ), mbCellXf( bCellXf ) {}

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
    inline explicit     DxfContext( ParentType& rParent, const DxfRef& rxDxf ) :
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

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
