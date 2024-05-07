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

#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include "xolefactory.hxx"
#include <oleembobj.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;

// TODO: do not create OLE objects that represent OOo documents


uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitFromEntry(
                                                                    const uno::Reference< embed::XStorage >& xStorage,
                                                                    const OUString& sEntName,
                                                                    const uno::Sequence< beans::PropertyValue >& aMedDescr,
                                                                    const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( officecfg::Office::Common::Security::Scripting::DisableActiveContent::get() )
        throw lang::NoSupportException(u"Active OLE content is disabled!"_ustr);
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< container::XNameAccess > xNameAccess( xStorage, uno::UNO_QUERY_THROW );

    // detect entry existence
    if ( !xNameAccess->hasByName( sEntName ) )
        throw container::NoSuchElementException();

    if ( !xStorage->isStreamElement( sEntName ) )
    {
        // if it is not an OLE object throw an exception
        throw io::IOException(); // TODO:
    }

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xContext, false ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::DEFAULT_INIT,
                                    aMedDescr,
                                    lObjArgs );

    for ( beans::PropertyValue const & prop : lObjArgs )
    {
        if ( prop.Name == "CloneFrom" )
        {
            try
            {
                uno::Reference < embed::XEmbeddedObject > xObj;
                uno::Reference < embed::XEmbeddedObject > xNew( xResult, uno::UNO_QUERY );
                prop.Value >>= xObj;
                if ( xObj.is() )
                    xNew->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, xObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT ) );
            }
            catch ( const uno::Exception& ) {}
            break;
        }
    }

    return xResult;
}


uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitFromMediaDescriptor(
        const uno::Reference< embed::XStorage >& xStorage,
        const OUString& sEntName,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( officecfg::Office::Common::Security::Scripting::DisableActiveContent::get() )
        throw lang::NoSupportException(u"Active OLE content is disabled!"_ustr);
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xContext, false ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}


uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceInitNew(
                                            const uno::Sequence< sal_Int8 >& aClassID,
                                            const OUString& aClassName,
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( officecfg::Office::Common::Security::Scripting::DisableActiveContent::get() )
        throw lang::NoSupportException(u"Active OLE content is disabled!"_ustr);
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            3 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            4 );

    uno::Reference< uno::XInterface > xResult(
                    static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xContext, aClassID, aClassName ) ),
                    uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::TRUNCATE_INIT,
                                    uno::Sequence< beans::PropertyValue >(),
                                    lObjArgs );

    return xResult;
}


uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceLink(
                                            const uno::Reference< embed::XStorage >& xStorage,
                                            const OUString& sEntName,
                                            const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if ( officecfg::Office::Common::Security::Scripting::DisableActiveContent::get() )
        throw lang::NoSupportException(u"Active OLE content is disabled!"_ustr);
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xContext, true ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                    sEntName,
                                    embed::EntryInitModes::MEDIA_DESCRIPTOR_INIT,
                                    aMediaDescr,
                                    lObjArgs );

    return xResult;
}


uno::Reference< uno::XInterface > SAL_CALL OleEmbeddedObjectFactory::createInstanceUserInit(
            const uno::Sequence< sal_Int8 >& aClassID,
            const OUString& aClassName,
            const uno::Reference< embed::XStorage >& xStorage,
            const OUString& sEntName,
            sal_Int32 /*nEntryConnectionMode*/,
            const uno::Sequence< beans::PropertyValue >& /*lArguments*/,
            const uno::Sequence< beans::PropertyValue >& lObjArgs )
{
    if (officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
        throw lang::NoSupportException(u"Active OLE content is disabled!"_ustr);
    // the initialization is completely controlled by user
    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    uno::Reference< uno::XInterface > xResult(
                static_cast< ::cppu::OWeakObject* > ( new OleEmbeddedObject( m_xContext, aClassID, aClassName ) ),
                uno::UNO_QUERY );

    uno::Reference< embed::XEmbedPersist > xPersist( xResult, uno::UNO_QUERY_THROW );
    xPersist->setPersistentEntry( xStorage,
                                  sEntName,
                                  embed::EntryInitModes::DEFAULT_INIT,
                                  uno::Sequence< beans::PropertyValue >(),
                                  lObjArgs );

    return xResult;
}


OUString SAL_CALL OleEmbeddedObjectFactory::getImplementationName()
{
    return u"com.sun.star.comp.embed.OLEEmbeddedObjectFactory"_ustr;
}

sal_Bool SAL_CALL OleEmbeddedObjectFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL OleEmbeddedObjectFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.embed.OLEEmbeddedObjectFactory"_ustr,
             u"com.sun.star.comp.embed.OLEEmbeddedObjectFactory"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
embeddedobj_OleEmbeddedObjectFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OleEmbeddedObjectFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
