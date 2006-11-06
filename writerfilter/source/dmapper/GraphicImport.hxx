/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicImport.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2006-11-06 15:01:06 $
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
#ifndef INCLUDED_GRAPHICIMPORT_HXX
#define INCLUDED_GRAPHICIMPORT_HXX

//#ifndef INCLUDED_WRITERFILTERDLLAPI_H
//#include <WriterFilterDllApi.hxx>
//#endif
#include <doctok/WW8ResourceModel.hxx>
//#include <com/sun/star/lang/XComponent.hpp>
namespace com{ namespace sun { namespace star {
    namespace uno{
        class XComponentContext;
    }
    namespace lang
    {
        class XMultiServiceFactory;
    }
    namespace text
    {
        class XTextContent;
    }
}}}

namespace dmapper
{
using namespace std;
struct GraphicImport_Impl;
class WRITERFILTER_DLLPRIVATE GraphicImport : public doctok::Properties, public doctok::Table
                    ,public doctok::BinaryObj, public doctok::Stream
{
    GraphicImport_Impl* m_pImpl;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >    m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xTextFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > m_xGraphicObject;
public:
    GraphicImport(::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >    xComponentContext,
                  ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xTextFactory);
    virtual ~GraphicImport();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len, doctok::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(doctok::Reference<Properties>::Pointer_t ref);
    virtual void table(doctok::Id name,
                       doctok::Reference<Table>::Pointer_t ref);
    virtual void substream(doctok::Id name,
                           ::doctok::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > GetGraphicObject() {return m_xGraphicObject; }
};
typedef boost::shared_ptr< GraphicImport >          GraphicImportPtr;
}

#endif //
