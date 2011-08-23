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
 ************************************************************************/

#ifndef OOX_XLS_SHAREDSTRINGSBUFFER_HXX
#define OOX_XLS_SHAREDSTRINGSBUFFER_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/xls/richstring.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Collects all strings from the shared strings substream. */
class SharedStringsBuffer : public WorkbookHelper
{
public:
    explicit            SharedStringsBuffer( const WorkbookHelper& rHelper );

    /** Creates and returns a new string entry. */
    RichStringRef       createRichString();
    /** Imports the complete shared string table from a BIFF file. */
    void                importSst( BiffInputStream& rStrm );

    /** Final processing after import of all strings. */
    void                finalizeImport();

    /** Converts the specified string table entry. */
    void                convertString(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >& rxText,
                            sal_Int32 nStringId,
                            sal_Int32 nXfId ) const;

private:
    typedef RefVector< RichString > StringVec;
    StringVec           maStrings;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
