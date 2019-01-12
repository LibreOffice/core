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

#ifndef INCLUDED_XMLOFF_XMLAUSTP_HXX
#define INCLUDED_XMLOFF_XMLAUSTP_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <memory>
#include <vector>

class SvXMLExportPropertyMapper;
class SvXMLNamespaceMap;
class SvXMLAutoStylePoolP_Impl;
class SvXMLAttributeList;
class SvXMLExport;
class SvXMLUnitConverter;
struct XMLPropertyState;

namespace com { namespace sun { namespace star { namespace uno
    { template<typename A> class Sequence; }
} } }

namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

namespace com { namespace sun { namespace star { namespace xml { namespace sax { class XDocumentHandler; } } } } }
namespace xmloff { struct AutoStyleEntry; }
namespace rtl { template <class reference_type> class Reference; }

class XMLOFF_DLLPUBLIC SvXMLAutoStylePoolP : public salhelper::SimpleReferenceObject
{
    friend class Test;
    friend class SvXMLAutoStylePoolP_Impl;

    std::unique_ptr<SvXMLAutoStylePoolP_Impl>    pImpl;

protected:

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;

// this method is executed after Properties Export, so you can e.g. export a map or so on
    virtual void exportStyleContent(
            const css::uno::Reference< css::xml::sax::XDocumentHandler > & rHandler,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;


public:

    SvXMLAutoStylePoolP( SvXMLExport& rExport);

    virtual ~SvXMLAutoStylePoolP() override;

    SvXMLExport& GetExport() const;

    /** register a new family with its appropriate instance of a derivation of XMLPropertySetMapper
        for family numbers see families.hxx
     */
    // TODO: Remove this ugly method as soon as possible
    void AddFamily( sal_Int32 nFamily, const OUString& rStrName, SvXMLExportPropertyMapper* pMapper, const OUString& aStrPrefix );
    void AddFamily( sal_Int32 nFamily, const OUString& rStrName,
                    const rtl::Reference< SvXMLExportPropertyMapper >& rMapper,
                    const OUString& rStrPrefix, bool bAsFamily = true );
    void SetFamilyPropSetMapper( sal_Int32 nFamily,
                    const rtl::Reference< SvXMLExportPropertyMapper >& rMapper );

    /// Register a name that must not be used as a generated name.
    void RegisterName( sal_Int32 nFamily, const OUString& rName );

    /// Register a name that may only be used through AddNamed
    void RegisterDefinedName(sal_Int32 nFamily, const OUString& rName);

    /// retrieve the registered names (names + families)
    void GetRegisteredNames(
        css::uno::Sequence<sal_Int32>& aFamilies,
        css::uno::Sequence<OUString>& aNames );

    /// register (families + names)
    void RegisterNames(
        css::uno::Sequence<sal_Int32> const & aFamilies,
        css::uno::Sequence<OUString> const & aNames );

    /// Add an item set to the pool and return its generated name.
    OUString Add( sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties );
    OUString Add( sal_Int32 nFamily, const OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties, bool bDontSeek = false );
    bool        Add( OUString& rName, sal_Int32 nFamily, const OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );

    /// Add an item set with a pre-defined name (needed for saving sheets separately in Calc).
    bool        AddNamed( const OUString& rName, sal_Int32 nFamily, const OUString& rParent,
                              const ::std::vector< XMLPropertyState >& rProperties );

    /// Find an item set's name.
    OUString Find( sal_Int32 nFamily, const OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties ) const;

    /** Export all item sets ofs a certain class in the order in that they have been added. */
    void exportXML( sal_Int32 nFamily ) const;

    void ClearEntries();

    std::vector<xmloff::AutoStyleEntry> GetAutoStyleEntries() const;
};

#endif // INCLUDED_XMLOFF_XMLAUSTP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
