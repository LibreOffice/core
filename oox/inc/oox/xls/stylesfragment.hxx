/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ***********************************************************************/

#ifndef OOX_XLS_STYLESFRAGMENT_HXX
#define OOX_XLS_STYLESFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/stylesbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

class IndexedColorsContext : public WorkbookContextBase
{
public:
    explicit            IndexedColorsContext( WorkbookFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );
};

// ============================================================================

class FontContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     FontContext( ParentType& rParent, const FontRef& rxFont ) :
                            WorkbookContextBase( rParent ), mxFont( rxFont ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    FontRef             mxFont;
};

// ============================================================================

class BorderContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     BorderContext( ParentType& rParent, const BorderRef& rxBorder ) :
                            WorkbookContextBase( rParent ), mxBorder( rxBorder ) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    BorderRef           mxBorder;
};

// ============================================================================

class FillContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     FillContext( ParentType& rParent, const FillRef& rxFill ) :
                            WorkbookContextBase( rParent ), mxFill( rxFill ), mfGradPos( -1.0 ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    FillRef             mxFill;
    double              mfGradPos;      /// Gradient color position.
};

// ============================================================================

class XfContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     XfContext( ParentType& rParent, const XfRef& rxXf, bool bCellXf ) :
                            WorkbookContextBase( rParent ), mxXf( rxXf ), mbCellXf( bCellXf ) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    XfRef               mxXf;
    bool                mbCellXf;       /// True = cell XF, false = style XF.
};

// ============================================================================

class DxfContext : public WorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     DxfContext( ParentType& rParent, const DxfRef& rxDxf ) :
                            WorkbookContextBase( rParent ), mxDxf( rxDxf ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    DxfRef              mxDxf;
};

// ============================================================================

class StylesFragment : public WorkbookFragmentBase
{
public:
    explicit            StylesFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */