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
#ifndef INCLUDED_XMLOFF_PRSTYLEI_HXX
#define INCLUDED_XMLOFF_PRSTYLEI_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/style/XStyle.hpp>
#include <vector>
#include <xmloff/xmlstyle.hxx>

//UUUU
#include <boost/unordered_set.hpp>

struct XMLPropertyState;
class SvXMLStylesContext;

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

//UUUU
typedef boost::unordered_set<OUString, OUStringHash> OldFillStyleDefinitionSet;

class XMLOFF_DLLPUBLIC XMLPropStyleContext : public SvXMLStyleContext
{
private:
    const OUString msIsPhysical;
    const OUString msFollowStyle;
    ::std::vector< XMLPropertyState > maProperties;
    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > mxStyle;
    SvXMLImportContextRef               mxStyles;

    //UUUU
    static OldFillStyleDefinitionSet maStandardSet;
    static OldFillStyleDefinitionSet maHeaderSet;
    static OldFillStyleDefinitionSet maFooterSet;
    static OldFillStyleDefinitionSet maParaSet;

    SAL_DLLPRIVATE XMLPropStyleContext(XMLPropStyleContext &); // not defined
    SAL_DLLPRIVATE void operator =(XMLPropStyleContext &); // not defined

protected:

    //UUUU Helper to check if the local maProperties contzains the given
    // FillStyle tag and if the FillStyle there is different from FillStyle_NONE
    bool doNewDrawingLayerFillStyleDefinitionsExist(
        const ::rtl::OUString& rFillStyleTag) const;

    //UUUU Helper which will deactivate all old fill definitions (identified by
    // the given OldFillStyleDefinitionSet) in the local maProperties. Deactivation
    // is done setting theindex to -1. It returns true when actually old fill
    // definitions existed and were deactivated
    bool deactivateOldFillStyleDefinitions(
        const OldFillStyleDefinitionSet& rHashSetOfTags);

    //UUUU Helper to translate new DrawingLayer FillStyle values which are name-based
    // from ODF internal name to style display names which can be found in the current
    // document model (using NameOrIndex Items). The change is executed on the internal
    // maProperties. The return value is true when actually names were changed
    bool translateNameBasedDrawingLayerFillStyleDefinitionsToStyleDisplayNames();

    //UUUU provider for often used sets
    const OldFillStyleDefinitionSet& getStandardSet();
    const OldFillStyleDefinitionSet& getHeaderSet();
    const OldFillStyleDefinitionSet& getFooterSet();
    const OldFillStyleDefinitionSet& getParaSet();

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) SAL_OVERRIDE;
    SvXMLStylesContext *GetStyles() { return (SvXMLStylesContext *)&mxStyles; }
    ::std::vector< XMLPropertyState > & GetProperties() { return maProperties; }

    // This methos my be overloaded to create a new style. Its called by
    // CreateInsert to create a style if a style with the requested family and
    // name couldn't be found. The st
    virtual ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();

public:

    TYPEINFO_OVERRIDE();

    XMLPropStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            SvXMLStylesContext& rStyles, sal_uInt16 nFamily = 0,
            bool bDefaultStyle=false );
    virtual ~XMLPropStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) SAL_OVERRIDE;

    virtual void FillPropertySet(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & rPropSet );

    const SvXMLStylesContext *GetStyles() const { return (const SvXMLStylesContext *)&mxStyles; }
    const ::std::vector< XMLPropertyState > & GetProperties() const { return maProperties; }

    const ::com::sun::star::uno::Reference <
                ::com::sun::star::style::XStyle >&
               GetStyle() const { return mxStyle; }
    void SetStyle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle >& xStl) { mxStyle = xStl; }

    virtual void SetDefaults() SAL_OVERRIDE;

    virtual void CreateAndInsert( bool bOverwrite ) SAL_OVERRIDE;
    virtual void Finish( bool bOverwrite ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
