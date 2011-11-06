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



#ifndef SAX_FASTSERIALIZER_HXX
#define SAX_FASTSERIALIZER_HXX

#include <com/sun/star/xml/sax/XFastSerializer.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase2.hxx>

#include <stack>

#include "sax/dllapi.h"
#include "sax/fshelper.hxx"

#define SERIALIZER_IMPLEMENTATION_NAME  "com.sun.star.comp.extensions.xml.sax.FastSerializer"
#define SERIALIZER_SERVICE_NAME     "com.sun.star.xml.sax.FastSerializer"

namespace sax_fastparser {

class SAX_DLLPUBLIC FastSaxSerializer : public ::cppu::WeakImplHelper2< ::com::sun::star::xml::sax::XFastSerializer, ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit            FastSaxSerializer(  );
    virtual             ~FastSaxSerializer();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > getOutputStream() {return mxOutputStream;}

    // The implementation details
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >   getSupportedServiceNames_Static(void);
    static ::rtl::OUString                                      getImplementationName_Static();

    // XFastSerializer
    virtual void SAL_CALL startDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL singleFastElement( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL singleUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
        throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutputStream )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFastTokenHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler >& xFastTokenHandler )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw ( ::com::sun::star::uno::RuntimeException );

    // C++ helpers
    virtual void SAL_CALL writeId( ::sal_Int32 Element );

    static ::rtl::OUString escapeXml( const ::rtl::OUString& s );

public:
    /** From now on, don't write directly to the stream, but to top of a stack.

        This is to be able to change the order of the data being written.
        If you need to write eg.
          p, r, rPr, [something], /rPr, t, [text], /r, /p,
        but get it in order
          p, r, t, [text], /t, rPr, [something], /rPr, /r, /p,
        simply do
          p, r, mark(), t, [text], /t, mark(), rPr, [something], /rPr,
          mergeTopMarks( true ), mergeTopMarks(), /r, /p
        and you are done.
     */
    void mark();

    /** Merge 2 topmost marks.

        There are 3 possibilities - prepend the top before the second top-most
        mark, append it, or append it later; prepending brings the possibility
        to switch parts of the output, appending later allows to write some
        output in advance.

        Writes the result to the output stream if the mark stack becomes empty
        by the operation.

        When the MERGE_MARKS_POSTPONE is specified, the merge happens just
        before the next merge.

        @see mark()
     */
    void mergeTopMarks( sax_fastparser::MergeMarksEnum eMergeType = sax_fastparser::MERGE_MARKS_APPEND );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > mxOutputStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastTokenHandler > mxFastTokenHandler;

    typedef ::com::sun::star::uno::Sequence< ::sal_Int8 > Int8Sequence;
    class ForMerge
    {
        Int8Sequence maData;
        Int8Sequence maPostponed;

    public:
        ForMerge() : maData(), maPostponed() {}

        Int8Sequence& getData();

        void prepend( const Int8Sequence &rWhat );
        void append( const Int8Sequence &rWhat );
        void postpone( const Int8Sequence &rWhat );

    private:
        static void merge( Int8Sequence &rTop, const Int8Sequence &rMerge, bool bAppend );
    };

    ::std::stack< ForMerge > maMarkStack;

    void writeFastAttributeList( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs );
    void write( const ::rtl::OUString& s );

protected:
    /** Forward the call to the output stream, or write to the stack.

        The latter in the case that we are inside a mark().
     */
    void writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

} // namespace sax_fastparser

#endif
