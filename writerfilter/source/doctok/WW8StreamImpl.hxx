/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
