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
#ifndef INCLUDED_GRAPHICIMPORT_HXX
#define INCLUDED_GRAPHICIMPORT_HXX

//#ifndef INCLUDED_WRITERFILTERDLLAPI_H
//#include <WriterFilterDllApi.hxx>
//#endif
#include <resourcemodel/WW8ResourceModel.hxx>
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
    namespace drawing
    {
        class XShape;
    }
    namespace beans
    {
        class PropertyValue;
        typedef ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > PropertyValues;
    }
}}}

namespace writerfilter {
namespace dmapper
{
class GraphicImport_Impl;
class DomainMapper;

enum GraphicImportType
{
    IMPORT_AS_GRAPHIC,
    IMPORT_AS_SHAPE,
    IMPORT_AS_DETECTED_INLINE,  
    IMPORT_AS_DETECTED_ANCHOR
};    

class WRITERFILTER_DLLPRIVATE GraphicImport : public Properties, public Table
                    ,public BinaryObj, public Stream
{
    GraphicImport_Impl* m_pImpl;
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >    m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xTextFactory;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > m_xGraphicObject;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape> m_xShape;
    void ProcessShapeOptions(Value & val);
    
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > createGraphicObject( 
            const ::com::sun::star::beans::PropertyValues& aMediaProperties );

public:
    explicit GraphicImport(::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >    xComponentContext,
                  ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xTextFactory,
                  DomainMapper& rDomainMapper,
                  GraphicImportType eGraphicImportType);
    virtual ~GraphicImport();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len, writerfilter::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name,
                       writerfilter::Reference<Table>::Pointer_t ref);
    virtual void substream(Id name,
                           ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);
    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > GetGraphicObject();
    bool    IsGraphic() const;
};
typedef boost::shared_ptr< GraphicImport >          GraphicImportPtr;
}}

#endif //
