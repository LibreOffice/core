/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excelhandlers.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:03:36 $
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

#ifndef OOX_XLS_EXCELHANDLERS_HXX
#define OOX_XLS_EXCELHANDLERS_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/xls/worksheethelper.hxx"

#define XML_TOKEN( token )          (::oox::NMSP_XML | XML_##token)
#define R_TOKEN( token )            (::oox::NMSP_RELATIONSHIPS | XML_##token)
#define XLS_TOKEN( token )          (::oox::NMSP_XLS | XML_##token)
#define XDR_TOKEN( token )          (::oox::NMSP_XDR | XML_##token)
#define XM_TOKEN( token )           (::oox::NMSP_XM | XML_##token)

namespace oox {
namespace xls {

const sal_Int32 XML_ROOT_CONTEXT    = ::oox::core::XML_ROOT_CONTEXT;

typedef ::oox::core::ContextWrapper ContextWrapper;

// ============================================================================

/** Context handler derived from the WorkbookHelper helper class.

    Used to import contexts in global workbook fragments.
 */
class OoxWorkbookContextBase : public ::oox::core::ContextHandler2, public WorkbookHelper
{
public:
    template< typename ParentType >
    explicit            OoxWorkbookContextBase( ParentType& rParent );
};

// ----------------------------------------------------------------------------

template< typename ParentType >
OoxWorkbookContextBase::OoxWorkbookContextBase( ParentType& rParent ) :
    ::oox::core::ContextHandler2( rParent ),
    WorkbookHelper( rParent )
{
}

// ============================================================================

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class OoxWorksheetContextBase : public ::oox::core::ContextHandler2, public WorksheetHelperRoot
{
public:
    template< typename ParentType >
    explicit            OoxWorksheetContextBase(
                            ParentType& rParent,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    template< typename ParentType >
    explicit            OoxWorksheetContextBase( ParentType& rParent );
};

// ----------------------------------------------------------------------------

template< typename ParentType >
OoxWorksheetContextBase::OoxWorksheetContextBase( ParentType& rParent,
        ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    ::oox::core::ContextHandler2( rParent ),
    WorksheetHelperRoot( rParent, xProgressBar, eSheetType, nSheet )
{
}

template< typename ParentType >
OoxWorksheetContextBase::OoxWorksheetContextBase( ParentType& rParent ) :
    ::oox::core::ContextHandler2( rParent ),
    WorksheetHelperRoot( rParent )
{
}

// ============================================================================

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class OoxWorkbookFragmentBase : public ::oox::core::FragmentHandler2, public WorkbookHelper
{
public:
    explicit            OoxWorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );
};

// ============================================================================

/** Fragment handler derived from the WorksheetHelper helper class.

    Used to import sheet fragments.
 */
class OoxWorksheetFragmentBase : public ::oox::core::FragmentHandler2, public WorksheetHelperRoot
{
public:
    explicit            OoxWorksheetFragmentBase(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    explicit            OoxWorksheetFragmentBase(
                            const WorksheetHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

