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

#ifndef INCLUDED_COMPHELPER_OFOPXMLHELPER_HXX
#define INCLUDED_COMPHELPER_OFOPXMLHELPER_HXX

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/uno/Sequence.h>

#include <comphelper/comphelperdllapi.h>

namespace com::sun::star::beans { struct StringPair; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XOutputStream; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

namespace comphelper::OFOPXMLHelper {

    // returns sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    /// @throws css::uno::Exception
    COMPHELPER_DLLPUBLIC
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >
    ReadRelationsInfoSequence(
        const css::uno::Reference< css::io::XInputStream >& xInStream,
        std::u16string_view aStreamName,
        const css::uno::Reference< css::uno::XComponentContext >& rContext );

    // returns sequence containing two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    /// @throws css::uno::Exception
    COMPHELPER_DLLPUBLIC
    css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >
    ReadContentTypeSequence(
        const css::uno::Reference< css::io::XInputStream >& xInStream,
        const css::uno::Reference< css::uno::XComponentContext >& rContext );

    // returns the ContentType for the given name, or empty when not found.
    // rContentTypes is a sequence containing two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    // The "Override" sequence is searched first before falling back on "Default".
    COMPHELPER_DLLPUBLIC
    OUString
    GetContentTypeByName(const css::uno::Sequence<css::uno::Sequence<css::beans::StringPair>>& rContentTypes,
                         const OUString& rFilename);

    // writes sequence of elements, where each element is described by sequence of tags,
    // where each tag is described by StringPair ( First - name, Second - value )
    // the first tag of each element sequence must be "Id"
    /// @throws css::uno::Exception
    COMPHELPER_DLLPUBLIC
    void WriteRelationsInfoSequence(
        const css::uno::Reference< css::io::XOutputStream >& xOutStream,
        const css::uno::Sequence< css::uno::Sequence< css::beans::StringPair > >& aSequence,
        const css::uno::Reference< css::uno::XComponentContext >& rContext );

    // writes two entries of type sequence<StringPair>
    // the first sequence describes "Default" elements, where each element is described
    // by StringPair object ( First - Extension, Second - ContentType )
    // the second sequence describes "Override" elements, where each element is described
    // by StringPair object ( First - PartName, Second - ContentType )
    /// @throws css::uno::Exception
    COMPHELPER_DLLPUBLIC
    void WriteContentSequence(
        const css::uno::Reference< css::io::XOutputStream >& xOutStream,
        const css::uno::Sequence< css::beans::StringPair >& aDefaultsSequence,
        const css::uno::Sequence< css::beans::StringPair >& aOverridesSequence,
        const css::uno::Reference< css::uno::XComponentContext >& rContext );

} // namespace comphelper::OFOPXMLHelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
