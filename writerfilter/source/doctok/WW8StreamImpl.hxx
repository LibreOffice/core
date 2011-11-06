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



#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <doctok/WW8Document.hxx>
#ifndef INCLUDED_OUTPUT_WITH_DEPTH_HXX
#include <resourcemodel/OutputWithDepth.hxx>
#endif

namespace writerfilter {
namespace doctok
{
using namespace ::com::sun::star;

class WW8StreamImpl : public WW8Stream
{
    uno::Reference<uno::XComponentContext> mrComponentContext;
    uno::Reference<io::XInputStream> mrStream;
    uno::Reference<container::XNameContainer> xOLESimpleStorage;
    uno::Reference<lang::XMultiComponentFactory> xFactory;

public:
    WW8StreamImpl(uno::Reference<uno::XComponentContext> rContext,
                  uno::Reference<io::XInputStream> rStream);
    virtual ~WW8StreamImpl();

    virtual WW8Stream::Pointer_t getSubStream(const ::rtl::OUString & rId);

    virtual Sequence get(sal_uInt32 nOffset, sal_uInt32 nCount)
        const;

    //virtual bool put(sal_uInt32 nOffset, const Sequence & rSeq);

    virtual string getSubStreamNames() const;
    virtual uno::Sequence<rtl::OUString> getSubStreamUNames() const;

    virtual void dump(OutputWithDepth<string> & o) const;
};
}}
