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
#ifndef INCLUDED_XMLOFF_XMLIMPPR_HXX
#define INCLUDED_XMLOFF_XMLIMPPR_HXX

#include <rtl/ref.hxx>
#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>
#include <salhelper/simplereferenceobject.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XMultiPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySetInfo; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XTolerantMultiPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }
namespace com { namespace sun { namespace star { namespace xml { namespace sax { class XAttributeList; } } } } }

struct XMLPropertyState;
class XMLPropertySetMapper;


class SvXMLUnitConverter;
class SvXMLNamespaceMap;
class SvXMLImport;

/** This struct is used as an optional parameter to the static
 * _FillPropertySet() methods.
 *
 * It should not be used in any other context.
 */
struct ContextID_Index_Pair
{
    sal_Int16 const nContextID;
    sal_Int32 nIndex;
};

class XMLOFF_DLLPUBLIC SvXMLImportPropertyMapper : public salhelper::SimpleReferenceObject
{
    rtl::Reference< SvXMLImportPropertyMapper> mxNextMapper;

    SvXMLImport& rImport;   // access to error handling

    SvXMLImportPropertyMapper(SvXMLImportPropertyMapper const &) = delete;
    SvXMLImportPropertyMapper& operator =(SvXMLImportPropertyMapper const &) = delete;

protected:

    rtl::Reference< XMLPropertySetMapper > maPropMapper;
    SvXMLImport& GetImport() const { return rImport;}

public:

    SvXMLImportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~SvXMLImportPropertyMapper() override;

    // Add a ImportPropertyMapper at the end of the import mapper chain.
    // The added mapper MUST not be used outside the Mapper chain any longer,
    // because its PropertyMapper will be replaced.
    void ChainImportMapper(
        const rtl::Reference< SvXMLImportPropertyMapper>& rMapper );

    /** fills the given itemset with the attributes in the given list
      * the map is only searched within the range
      * [nStartIdx, nEndIdx[
      */
    void importXML(
            ::std::vector< XMLPropertyState >& rProperties,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            sal_uInt32 nPropType,
            sal_Int32 nStartIdx, sal_Int32 nEndIdx ) const;

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** This method is called when all attributes have benn processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;

    void CheckSpecialContext(
            const ::std::vector< XMLPropertyState >& rProperties,
            const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
            ContextID_Index_Pair* pSpecialContextIds ) const;

    bool FillPropertySet(
            const ::std::vector< XMLPropertyState >& rProperties,
            const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
            ContextID_Index_Pair* pSpecialContextIds = nullptr ) const;

    void FillPropertySequence(
            const ::std::vector< XMLPropertyState >& aProperties,
            css::uno::Sequence< css::beans::PropertyValue >& rValues )
            const;

    inline const rtl::Reference< XMLPropertySetMapper >&
        getPropertySetMapper() const;


    /** implementation helper for FillPropertySet: fill an XPropertySet.
     * Exceptions will be asserted. */
    static bool FillPropertySet_(
        const ::std::vector<XMLPropertyState> & rProperties,
        const css::uno::Reference< css::beans::XPropertySet> & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo> & rPropSetInfo,
        const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
        SvXMLImport& rImport,

        // parameter for use by txtstyli.cxx; allows efficient
        // catching the combined characters property
        ContextID_Index_Pair* pSpecialContextIds );

    /** implementation helper for FillPropertySet: fill an XMultiPropertySet.
     * If unsuccessul, set return value. */
    static bool FillMultiPropertySet_(
        const ::std::vector<XMLPropertyState> & rProperties,
        const css::uno::Reference< css::beans::XMultiPropertySet> & rMultiPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo> & rPropSetInfo,
        const rtl::Reference<XMLPropertySetMapper> & rPropMapper,

        // parameters for use by txtstyli.cxx; allows efficient
        // catching the combined characters property
        ContextID_Index_Pair* pSpecialContextIds );

    /** implementation helper for FillPropertySet: fill an XTolerantMultiPropertySet.
     * If unsuccessul, set return value. */
    static bool FillTolerantMultiPropertySet_(
        const ::std::vector<XMLPropertyState> & rProperties,
        const css::uno::Reference< css::beans::XTolerantMultiPropertySet> & rTolPropSet,
        const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
        SvXMLImport& rImport,

        // parameters for use by txtstyli.cxx; allows efficient
        // catching the combined characters property
        ContextID_Index_Pair* pSpecialContextIds );


    static void PrepareForMultiPropertySet_(
        const ::std::vector<XMLPropertyState> & rProperties,
        const css::uno::Reference< css::beans::XPropertySetInfo> & rPropSetInfo,
        const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
        ContextID_Index_Pair* pSpecialContextIds,
        css::uno::Sequence< OUString >& rNames,
        css::uno::Sequence< css::uno::Any >& rValues);
};


inline const rtl::Reference< XMLPropertySetMapper >&
    SvXMLImportPropertyMapper::getPropertySetMapper() const
{
    return maPropMapper;
}

#endif // INCLUDED_XMLOFF_XMLIMPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
