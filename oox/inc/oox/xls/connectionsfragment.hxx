/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: connectionsfragment.hxx,v $
 * $Revision: 1.4 $
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
 ************************************************************************/

#ifndef OOX_XLS_CONNECTIONSFRAGMENT_HXX
#define OOX_XLS_CONNECTIONSFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxConnectionsFragment : public OoxWorkbookFragmentBase
{
public:
    explicit            OoxConnectionsFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );

private:
    void                importConnection( const AttributeList& rAttribs );
    void                importWebPr( const AttributeList& rAttribs );
    void                importTables( const AttributeList& rAttribs );
    void                importS( const AttributeList& rAttribs );
    void                importX( const AttributeList& rAttribs );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

