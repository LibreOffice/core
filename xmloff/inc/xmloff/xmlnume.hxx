/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlnume.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:46:50 $
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

#ifndef _XMLOFF_XMLNUME_HXX
#define _XMLOFF_XMLNUME_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif

#ifndef _XMLOFF_XMLSTRINGVECTOR_HXX
#include <xmloff/XMLStringVector.hxx>
#endif
#ifndef _XMLOFF_ATTRLIST_HXX
#include <xmloff/attrlist.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace style { class XStyle; }
    namespace container { class XIndexReplace; }
    namespace beans { struct PropertyValue; }
} } }

class SvXMLNamespaceMap;
class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SvXMLExport;
class XMLTextListAutoStylePool;

class SvxXMLNumRuleExport
{
    SvXMLExport& rExport;
    const ::rtl::OUString sCDATA;
    const ::rtl::OUString sWS;
    const ::rtl::OUString sNumberingRules;
    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sIsContinuousNumbering;

    void exportLevelStyles(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > & xNumRule,
            sal_Bool bOutline=sal_False );

    void exportLevelStyle(
            sal_Int32 nLevel,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue>& rProps,
            sal_Bool bOutline=sal_False );

protected:

    // This method may be overloaded to add attributes to the <list-style>
    // element.
    virtual void AddListStyleAttributes();

    sal_Bool exportStyle( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::style::XStyle >& rStyle );
    void exportOutline();

    SvXMLExport& GetExport() { return rExport; }

     virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >  GetUNONumRule() const;

public:

    SvxXMLNumRuleExport( SvXMLExport& rExport );
    virtual ~SvxXMLNumRuleExport();

    void exportStyles( sal_Bool bUsed,
                       XMLTextListAutoStylePool *pPool,
                       sal_Bool bExportChapterNumbering = sal_True );
    void exportNumberingRule(
            const ::rtl::OUString& rName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > & xNumRule );

    void Export( const ::rtl::OUString& rOutline, sal_Bool bContNumbering );
    void ExportOutline();

    static sal_Bool GetOutlineStyles( XMLStringVector& rStyleNames,
               const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel > & rModel );

};

#endif  //  _XMLOFF_XMLNUME_HXX

