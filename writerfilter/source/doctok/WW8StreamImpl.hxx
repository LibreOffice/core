/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8StreamImpl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:49:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef INCLUDED_WW8_DOCUMENT_HXX
#include <doctok/WW8Document.hxx>
#endif
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
