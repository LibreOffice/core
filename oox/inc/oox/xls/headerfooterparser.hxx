/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: headerfooterparser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

