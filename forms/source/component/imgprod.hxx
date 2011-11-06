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



#ifndef _PRODUCE_HXX
#define _PRODUCE_HXX

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>
#include <com/sun/star/awt/ImageStatus.hpp>
#include <com/sun/star/awt/XImageConsumer.hpp>
#include <com/sun/star/awt/XImageProducer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/weak.hxx>


// -----------------
// - ImageProducer -
// -----------------



class SvStream;
class ImageConsumer;
class Graphic;
class GraphicFilter;

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
}}}}


class ImageProducer :   public ::com::sun::star::awt::XImageProducer,
                        public ::com::sun::star::lang::XInitialization,
                        public ::cppu::OWeakObject
{
private:

    ::rtl::OUString maURL;
    List            maConsList;
    Graphic*        mpGraphic;
    SvStream*       mpStm;
    sal_uInt32      mnTransIndex;
    sal_Bool        mbConsInit;
    Link            maDoneHdl;

    sal_Bool        ImplImportGraphic( Graphic& rGraphic );
    void            ImplUpdateData( const Graphic& rGraphic );
    void            ImplInitConsumer( const Graphic& rGraphic );
    void            ImplUpdateConsumer( const Graphic& rGraphic );

public:

                    ImageProducer();
                    ~ImageProducer();

    void            SetImage( const ::rtl::OUString& rPath );
    void            SetImage( SvStream& rStm );

    void            NewDataAvailable();

    void            SetDoneHdl( const Link& i_rHdl ) { maDoneHdl = i_rHdl; }
    const Link&     GetDoneHdl() const { return maDoneHdl; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // MT: ???
    void            setImage( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > & rStmRef );

    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& rxConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& rxConsumer ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XInitialization
    void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

};

#endif // _PRODUCE_HXX
