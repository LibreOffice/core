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
#ifndef _WRTXML_HXX
#define _WRTXML_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <shellio.hxx>

class SwPaM;
class SfxMedium;

namespace com { namespace sun { namespace star {
    namespace uno { template<class A> class Reference; }
    namespace uno { template<class A> class Sequence; }
    namespace uno { class Any; }
    namespace lang { class XComponent; }
    namespace lang { class XMultiServiceFactory; }
    namespace beans { struct PropertyValue; }
} } }


class SwXMLWriter : public StgWriter
{
    sal_uInt32 _Write( const com::sun::star::uno::Reference < com::sun::star::task::XStatusIndicator>&, const OUString&  );

    using StgWriter::Write;

protected:
    virtual sal_uLong WriteStorage();
    virtual sal_uLong WriteMedium( SfxMedium& aTargetMedium );

public:

    SwXMLWriter( const OUString& rBaseURL );
    virtual ~SwXMLWriter();

    virtual sal_uLong Write( SwPaM&, SfxMedium&, const OUString* = 0 );

private:

    // helper methods to write XML streams

    /// write a single XML stream into the package
    bool WriteThroughComponent(
        /// the component we export
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pStreamName,        /// the stream name
        /// service factory for pServiceName
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext> & rFactory,
        const sal_Char* pServiceName,       /// service name of the component
        /// the argument (XInitialization)
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        /// output descriptor
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc );

    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XOutputStream> & xOutputStream,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext> & rFactory,
        const sal_Char* pServiceName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc );
};


#endif  //  _WRTXML_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
