/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    sal_uInt32 _Write( const com::sun::star::uno::Reference < com::sun::star::task::XStatusIndicator>&, const rtl::OUString&  );

    using StgWriter::Write;

protected:
    virtual sal_uLong WriteStorage();
    virtual sal_uLong WriteMedium( SfxMedium& aTargetMedium );

public:

    SwXMLWriter( const String& rBaseURL );
    virtual ~SwXMLWriter();

    virtual sal_uLong Write( SwPaM&, SfxMedium&, const String* = 0 );

private:

    // helper methods to write XML streams

    /// write a single XML stream into the package
    sal_Bool WriteThroughComponent(
        /// the component we export
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pStreamName,        /// the stream name
        /// service factory for pServiceName
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory> & rFactory,
        const sal_Char* pServiceName,       /// service name of the component
        /// the argument (XInitialization)
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        /// output descriptor
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc,
        sal_Bool bPlainStream );            /// neither compress nor encrypt

    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    sal_Bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XOutputStream> & xOutputStream,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory> & rFactory,
        const sal_Char* pServiceName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc );
};


#endif  //  _WRTXML_HXX

