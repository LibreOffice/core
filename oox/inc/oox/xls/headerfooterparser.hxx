/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: headerfooterparser.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_XLS_HEADERFOOTERPARSER_HXX
#define OOX_XLS_HEADERFOOTERPARSER_HXX

#include <memory>
#include "oox/xls/workbookhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace sheet { class XHeaderFooterContent; }
} } }

namespace oox {
namespace xls {

// ============================================================================

class HeaderFooterParserImpl;

class HeaderFooterParser : public WorkbookHelper
{
public:
    explicit            HeaderFooterParser( const WorkbookHelper& rHelper );
    virtual             ~HeaderFooterParser();

    /** Parses the passed string and creates the header/footer contents. */
    void                parse(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XHeaderFooterContent >& rxContext,
                            const ::rtl::OUString& rData );

    /** Returns the total height of the converted header or footer in points. */
    double              getTotalHeight() const;

private:
    ::std::auto_ptr< HeaderFooterParserImpl > mxImpl;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

