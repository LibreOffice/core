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

#ifndef INCLUDED_XMLOFF_SETTINGSEXPORTHELPER_HXX
#define INCLUDED_XMLOFF_SETTINGSEXPORTHELPER_HXX

#include <xmloff/dllapi.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace beans { struct PropertyValue; } } } }
namespace com { namespace sun { namespace star { namespace formula { struct SymbolDescriptor; } } } }
namespace com { namespace sun { namespace star { namespace util { class XStringSubstitution; } } } }

namespace com
{
    namespace sun { namespace star {
        namespace container { class XNameAccess; class XIndexAccess; }
        namespace util { struct DateTime; }
    } }
}

namespace xmloff
{
    class XMLSettingsExportContext;
}

class XMLOFF_DLLPUBLIC XMLSettingsExportHelper
{
    ::xmloff::XMLSettingsExportContext& m_rContext;

    css::uno::Reference< css::util::XStringSubstitution > mxStringSubsitution;

    void ManipulateSetting( css::uno::Any& rAny, const OUString& rName ) const;

    void CallTypeFunction(const css::uno::Any& rAny,
                        const OUString& rName) const;

    void exportBool(const bool bValue, const OUString& rName) const;
    static void exportByte();
    void exportShort(const sal_Int16 nValue, const OUString& rName) const;
    void exportInt(const sal_Int32 nValue, const OUString& rName) const;
    void exportLong(const sal_Int64 nValue, const OUString& rName) const;
    void exportDouble(const double fValue, const OUString& rName) const;
    void exportString(const OUString& sValue, const OUString& rName) const;
    void exportDateTime(const css::util::DateTime& aValue, const OUString& rName) const;
    void exportSequencePropertyValue(
        const css::uno::Sequence<css::beans::PropertyValue>& aProps,
        const OUString& rName) const;
    void exportbase64Binary(
        const css::uno::Sequence<sal_Int8>& aProps,
        const OUString& rName) const;
    void exportMapEntry(const css::uno::Any& rAny,
                        const OUString& rName,
                        const bool bNameAccess) const;
    void exportNameAccess(
        const css::uno::Reference<css::container::XNameAccess>& rNamed,
        const OUString& rName) const;
    void exportIndexAccess(
        const css::uno::Reference<css::container::XIndexAccess>& rIndexed,
        const OUString& rName) const;

    void exportSymbolDescriptors(
                    const css::uno::Sequence < css::formula::SymbolDescriptor > &rProps,
                    const OUString& rName) const;
    void exportForbiddenCharacters(
                    const css::uno::Any &rAny,
                    const OUString& rName) const;

public:
    XMLSettingsExportHelper( ::xmloff::XMLSettingsExportContext& i_rContext );
    ~XMLSettingsExportHelper();

    void exportAllSettings(
        const css::uno::Sequence<css::beans::PropertyValue>& aProps,
        const OUString& rName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
