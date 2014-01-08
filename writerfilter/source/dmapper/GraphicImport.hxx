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


#ifndef INCLUDED_GRAPHICIMPORT_HXX
#define INCLUDED_GRAPHICIMPORT_HXX

//#ifndef INCLUDED_WRITERFILTERDLLAPI_H
//#include <WriterFilterDllApi.hxx>
//#endif
#include <resourcemodel/LoggedResources.hxx>
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
        struct PropertyValue;
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

class WRITERFILTER_DLLPRIVATE GraphicImport : public LoggedProperties, public LoggedTable
                    ,public BinaryObj, public LoggedStream
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

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len, writerfilter::Reference<Properties>::Pointer_t ref);

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > GetGraphicObject();
    bool    IsGraphic() const;

 private:
    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void lcl_startSectionGroup();
    virtual void lcl_endSectionGroup();
    virtual void lcl_startParagraphGroup();
    virtual void lcl_endParagraphGroup();
    virtual void lcl_startCharacterGroup();
    virtual void lcl_endCharacterGroup();
    virtual void lcl_text(const sal_uInt8 * data, size_t len);
    virtual void lcl_utext(const sal_uInt8 * data, size_t len);
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref);
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void lcl_info(const string & info);
    virtual void lcl_startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void lcl_endShape( );

    void handleWrapTextValue(sal_uInt32 nVal);
};

typedef boost::shared_ptr< GraphicImport >          GraphicImportPtr;
}}

#endif //
