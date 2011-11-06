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


#ifndef INCLUDED_OLEHANDLER_HXX
#define INCLUDED_OLEHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/LoggedResources.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>

#include <com/sun/star/drawing/XShape.hpp>

namespace com{ namespace sun{ namespace star{
    namespace embed{
        class XEmbeddedObject;
    }
    namespace graphic{
        class XGraphic;
    }
    namespace io{
        class XInputStream;
    }
    namespace text{
        class XTextDocument;
    }
    namespace uno{
        class XComponentContext;
    }
}}}
namespace writerfilter {
namespace dmapper
{
//class PropertyMap;
/** Handler for OLE objects
 */
class WRITERFILTER_DLLPRIVATE OLEHandler : public LoggedProperties
{
    ::rtl::OUString     m_sObjectType;
    ::rtl::OUString     m_sProgId;
    ::rtl::OUString     m_sShapeId;
    ::rtl::OUString     m_sDrawAspect;
    ::rtl::OUString     m_sObjectId;
    ::rtl::OUString     m_sr_id;

    sal_Int32                   m_nDxaOrig;
    sal_Int32                   m_nDyaOrig;
    sal_Int32                   m_nWrapMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > m_xShape;

    ::com::sun::star::awt::Size m_aShapeSize;
    ::com::sun::star::awt::Point m_aShapePosition;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xReplacement;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xInputStream;

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

public:
    OLEHandler();
    virtual ~OLEHandler();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getShape( ) { return m_xShape; };

    inline bool isOLEObject( ) { return m_xInputStream.is( ); };

    ::rtl::OUString copyOLEOStream( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument > xTextDocument );

    ::com::sun::star::awt::Size     getSize() const { return m_aShapeSize;}
    ::com::sun::star::awt::Point    getPosition() const { return m_aShapePosition;}
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                                    getReplacement() const { return m_xReplacement; }

};
typedef boost::shared_ptr< OLEHandler >  OLEHandlerPtr;
}}

#endif //
