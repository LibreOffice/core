/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    const OUString sCDATA;
    const OUString sWS;
    const OUString sNumberingRules;
    const OUString sIsPhysical;
    const OUString sIsContinuousNumbering;
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

public:

    SvxXMLNumRuleExport( SvXMLExport& rExport );
    virtual ~SvxXMLNumRuleExport();

    void exportStyles( sal_Bool bUsed,
                       XMLTextListAutoStylePool *pPool,
                       sal_Bool bExportChapterNumbering = sal_True );
    void exportNumberingRule(
            const OUString& rName, sal_Bool bIsHidden,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > & xNumRule );
};

#endif  //  _XMLOFF_XMLNUME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
