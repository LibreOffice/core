/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stylesfragment.hxx,v $
 * $Revision: 1.4.20.1 $
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

class OoxStylesFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxStylesFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        finalizeImport();

private:
    FontRef             mxFont;         /// Current font object.
    BorderRef           mxBorder;       /// Current cell border object.
    FillRef             mxFill;         /// Current cell area fill object.
    XfRef               mxXf;           /// Current cell format/style object.
    DxfRef              mxDxf;          /// Current differential format object.
    double              mfGradPos;      /// Gradient color position.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

