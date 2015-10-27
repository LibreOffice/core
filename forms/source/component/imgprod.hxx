/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_IMGPROD_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_IMGPROD_HXX

#include <tools/link.hxx>
#include <com/sun/star/awt/ImageStatus.hpp>
#include <com/sun/star/awt/XImageConsumer.hpp>
#include <com/sun/star/awt/XImageProducer.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/weak.hxx>
#include <vector>


// - ImageProducer -




class SvStream;
class Graphic;

namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
}}}}


class ImageProducer :   public css::awt::XImageProducer,
                        public css::lang::XInitialization,
                        public ::cppu::OWeakObject
{
private:

    typedef std::vector< css::uno::Reference< css::awt::XImageConsumer > > ConsumerList_t;

    OUString        maURL;
    ConsumerList_t  maConsList;
    Graphic*        mpGraphic;
    SvStream*       mpStm;
    sal_uInt32      mnTransIndex;
    bool            mbConsInit;
    Link<Graphic*,void> maDoneHdl;

    bool             ImplImportGraphic( Graphic& rGraphic );
    void            ImplUpdateData( const Graphic& rGraphic );
    void            ImplInitConsumer( const Graphic& rGraphic );
    void            ImplUpdateConsumer( const Graphic& rGraphic );

public:

                    ImageProducer();
                    virtual ~ImageProducer();

    void            SetImage( const OUString& rPath );
    void            SetImage( SvStream& rStm );

    void            NewDataAvailable();

    void            SetDoneHdl( const Link<Graphic*,void>& i_rHdl ) { maDoneHdl = i_rHdl; }

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    void                           SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                           SAL_CALL release() throw() override  { OWeakObject::release(); }

    // MT: ???
    void            setImage( css::uno::Reference< css::io::XInputStream > & rStmRef );

    // css::awt::XImageProducer
    void SAL_CALL addConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer )
        throw(css::uno::RuntimeException,
              std::exception) override;
    void SAL_CALL removeConsumer( const css::uno::Reference< css::awt::XImageConsumer >& rxConsumer ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL startProduction(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XInitialization
    void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

};

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_IMGPROD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
