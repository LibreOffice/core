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

class OoxIndexedColorsContext : public OoxWorkbookContextBase
{
public:
    explicit            OoxIndexedColorsContext( OoxWorkbookFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
};

// ============================================================================

class OoxFontContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     OoxFontContext( ParentType& rParent, const FontRef& rxFont ) :
                            OoxWorkbookContextBase( rParent ), mxFont( rxFont ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    FontRef             mxFont;
};

// ============================================================================

class OoxBorderContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     OoxBorderContext( ParentType& rParent, const BorderRef& rxBorder ) :
                            OoxWorkbookContextBase( rParent ), mxBorder( rxBorder ) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    BorderRef           mxBorder;
};

// ============================================================================

class OoxFillContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     OoxFillContext( ParentType& rParent, const FillRef& rxFill ) :
                            OoxWorkbookContextBase( rParent ), mxFill( rxFill ), mfGradPos( -1.0 ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    FillRef             mxFill;
    double              mfGradPos;      /// Gradient color position.
};

// ============================================================================

class OoxXfContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     OoxXfContext( ParentType& rParent, const XfRef& rxXf, bool bCellXf ) :
                            OoxWorkbookContextBase( rParent ), mxXf( rxXf ), mbCellXf( bCellXf ) {}

protected:
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    XfRef               mxXf;
    bool                mbCellXf;       /// True = cell XF, false = style XF.
};

// ============================================================================

class OoxDxfContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    inline explicit     OoxDxfContext( ParentType& rParent, const DxfRef& rxDxf ) :
                            OoxWorkbookContextBase( rParent ), mxDxf( rxDxf ) {}

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    DxfRef              mxDxf;
};

// ============================================================================

class OoxStylesFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxStylesFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

