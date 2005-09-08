/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imgprod.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:50:32 $
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

#ifndef _PRODUCE_HXX
#define _PRODUCE_HXX

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/list.hxx>

#ifndef _COM_SUN_STAR_AWT_IMAGESTATUS_HPP_
#include <com/sun/star/awt/ImageStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGECONSUMER_HPP_
#include <com/sun/star/awt/XImageConsumer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XIMAGEPRODUCER_HPP_
#include <com/sun/star/awt/XImageProducer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif


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
    GraphicFilter*  mpFilter;
    sal_uInt32      mnTransIndex;
    sal_uInt32      mnStatus;
    sal_Bool        mbConsInit;
    sal_Bool        mbStmDel;
    Link            maErrorHdl;
    sal_uInt32      mnLastError;

    sal_uInt32      mnExtra2;

    sal_Bool        mbAsync;
    sal_Bool        mbExtra1;
    sal_Bool        mbExtra2;
    sal_Bool        mbExtra3;

    sal_Bool        ImplImportGraphic( Graphic& rGraphic );
    void            ImplUpdateData( const Graphic& rGraphic );
    void            ImplInitConsumer( const Graphic& rGraphic );
    void            ImplUpdateConsumer( const Graphic& rGraphic );

public:

                    ImageProducer();
                    ~ImageProducer();

    void            SetImage( const ::rtl::OUString& rPath );
    void            SetImage( SvStream& rStm );
    void            SetImage( const Graphic& rGraphic );

    void            SetErrorHandler( const Link& rErrorHdl ) { maErrorHdl = rErrorHdl; }
    const Link&     GetErrorHandler() const { return maErrorHdl; }

    sal_uInt32      GetLastError() const { return mnLastError; }
    void            ResetLastError() { mnLastError = 0; }

    void            NewDataAvailable();

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
