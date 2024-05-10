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

#include <sfx2/sfxmodelfactory.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <comphelper/namedvaluecollection.hxx>

#include <osl/diagnose.h>

#include <algorithm>


namespace sfx2
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::lang::XInitialization;



    namespace
    {
        struct IsSpecialArgument
        {
            static bool isSpecialArgumentName( std::u16string_view _rValueName )
            {
                return  _rValueName == u"EmbeddedObject" || _rValueName == u"EmbeddedScriptSupport" || _rValueName == u"DocumentRecoverySupport";
            }

            bool operator()( const Any& _rArgument ) const
            {
                NamedValue aNamedValue;
                if ( ( _rArgument >>= aNamedValue ) && isSpecialArgumentName( aNamedValue.Name ) )
                    return true;
                PropertyValue aPropertyValue;
                return ( _rArgument >>= aPropertyValue ) && isSpecialArgumentName( aPropertyValue.Name );
            }
        };
    }


    css::uno::Reference<css::uno::XInterface> createSfxModelInstance(
        const css::uno::Sequence<css::uno::Any> & _rArguments,
        std::function<css::uno::Reference<css::uno::XInterface>(SfxModelFlags)> creationFunc)
    {
        ::comphelper::NamedValueCollection aArgs( _rArguments );
        const bool bEmbeddedObject = aArgs.getOrDefault( u"EmbeddedObject"_ustr, false );
        const bool bScriptSupport = aArgs.getOrDefault( u"EmbeddedScriptSupport"_ustr, true );
        const bool bDocRecoverySupport = aArgs.getOrDefault( u"DocumentRecoverySupport"_ustr, true );

        SfxModelFlags nCreationFlags =
                ( bEmbeddedObject ? SfxModelFlags::EMBEDDED_OBJECT : SfxModelFlags::NONE )
            |   ( bScriptSupport ? SfxModelFlags::NONE : SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS )
            |   ( bDocRecoverySupport ? SfxModelFlags::NONE : SfxModelFlags::DISABLE_DOCUMENT_RECOVERY );

        Reference< XInterface > xInstance( creationFunc(nCreationFlags ) );

        // to mimic the behaviour of the default factory's createInstanceWithArguments, we initialize
        // the object with the given arguments, stripped by the three special ones
        Sequence< Any > aStrippedArguments( _rArguments.getLength() );
        Any* pStrippedArgs = aStrippedArguments.getArray();
        Any* pStrippedArgsEnd = ::std::remove_copy_if(
            _rArguments.begin(),
            _rArguments.end(),
            pStrippedArgs,
            IsSpecialArgument()
        );
        aStrippedArguments.realloc( pStrippedArgsEnd - pStrippedArgs );

        if ( aStrippedArguments.hasElements() )
        {
            Reference< XInitialization > xModelInit( xInstance, UNO_QUERY );
            OSL_ENSURE( xModelInit.is(), "SfxModelFactory::createInstanceWithArguments: no XInitialization!" );
            if ( xModelInit.is() )
                xModelInit->initialize( aStrippedArguments );
        }

        return xInstance;
    }


} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
