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

#ifndef _XMLOFF_XMLNUME_HXX
#define _XMLOFF_XMLNUME_HXX

#include <rtl/ustring.hxx>
#include <tools/mapunit.hxx>
#include <xmloff/XMLStringVector.hxx>
#include <xmloff/attrlist.hxx>

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
    // Boolean indicating, if properties for position-and-space-mode LABEL_ALIGNEMNT
    // are exported or not. (#i89178#)
    // These properties have been introduced in ODF 1.2. Thus, its export have
    // to be suppressed on writing ODF 1.0 respectively ODF 1.1
    bool mbExportPositionAndSpaceModeLabelAlignment;

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

    static sal_Bool GetOutlineStyles( XMLStringVector& rStyleNames,
               const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel > & rModel );

};

#endif  //  _XMLOFF_XMLNUME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
