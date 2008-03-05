/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autofiltercontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:59:33 $
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

#ifndef OOX_XLS_AUTOFILTERCONTEXT_HXX
#define OOX_XLS_AUTOFILTERCONTEXT_HXX

#define USE_SC_MULTI_STRING_FILTER_PATCH 0

#include "oox/xls/excelhandlers.hxx"
#include <com/sun/star/table/CellRangeAddress.hpp>

#if USE_SC_MULTI_STRING_FILTER_PATCH
#include <com/sun/star/sheet/TableFilterFieldBase.hpp>
#else
#include <com/sun/star/sheet/TableFilterField.hpp>
#endif

#include <boost/shared_ptr.hpp>
#include <list>

namespace com { namespace sun { namespace star { namespace sheet {
#if USE_SC_MULTI_STRING_FILTER_PATCH
    struct TableFilterFieldBase;
#else
    struct TableFilterField;
#endif
    struct TableFilterFieldMultiString;
}}}}

namespace oox {
namespace xls {

// ============================================================================

struct FilterFieldItem
{
#if USE_SC_MULTI_STRING_FILTER_PATCH
    typedef ::boost::shared_ptr< ::com::sun::star::sheet::TableFilterFieldBase > TableFilterFieldRef;
#else
    typedef ::boost::shared_ptr< ::com::sun::star::sheet::TableFilterField > TableFilterFieldRef;
#endif

    enum Type { NORMAL, MULTI_STRING };

    TableFilterFieldRef mpField;
    Type                meType;

    FilterFieldItem();
    FilterFieldItem(Type eType);
};

// ============================================================================

class OoxAutoFilterContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxAutoFilterContext( OoxWorksheetFragmentBase& rFragment );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    /** Initializes data members to prepare for autofilter parsing.  Call this
        method when a new autofilter context starts. */
    void                initialize();

    /** Commits the imported autofilter data to Calc. */
    void                setAutoFilter();

    /** Check if the shown blank bit is on, if so, add show blank filter to the
        list. */
    void                maybeShowBlank();

    /** Packs the collected filter names into a single regex string. */
    void                setFilterNames();

    void                importAutoFilter( const AttributeList& rAttribs );

    void                importFilterColumn( const AttributeList& rAttribs );

    void                importTop10( const AttributeList& rAttribs );

    /** Be sure to check for its 'and' attribute when there are two
        customFilter's.  If the attribute is not given, assume OR. */
    void                importCustomFilters( const AttributeList& rAttribs );

    /** Imports custumFilter element.  Note that the standard specifies there
        can be at most two custom filters specified, but no more than two. When
        there are two custom filters, then their relationship (AND or OR) must
        be specified by the parent element <customFilters>. */
    void                importCustomFilter( const AttributeList& rAttribs );

    void                importFilters( const AttributeList& rAttribs );

    void                importFilter( const AttributeList& rAttribs );

    void                importDynamicFilter( const AttributeList& rAttribs );

private:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > XCellRef;

    ::std::list< FilterFieldItem >  maFields;

    ::std::list< ::rtl::OUString > maFilterNames;
    ::com::sun::star::table::CellRangeAddress maAutoFilterRange;
    sal_Int32 mnCurColID;

    /** If this is false (i.e. the given cell range address is not valid), then
        don't do anything. */
    bool mbValidAddress:1;

    /** We use regex to compensate for Calc's lack of filtering by individual
        names (i.e. <filter> tag). */
    bool mbUseRegex:1;

    /** The <filters> tag may have a 'blank' attribute when the blank values
        need to be shown.  This flag stores that information. */
    bool mbShowBlank:1;

    /** true if FilterConnection_AND, or false if FilterConnection_OR.  This
        flag is used to store the relationship of paired customFilter's. */
    bool mbConnectionAnd:1;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
