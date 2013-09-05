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

#ifndef _SD_TRANSITIONPRESET_HXX
#define _SD_TRANSITIONPRESET_HXX

#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/stl_types.hxx>

#include <list>
#include <boost/unordered_map.hpp>

namespace com { namespace sun { namespace star {
    namespace animations { class XAnimationNode; }
    namespace uno { template<class X> class Reference; }
} } }

class SdPage;

namespace sd {

class TransitionPreset;
typedef boost::shared_ptr< TransitionPreset > TransitionPresetPtr;
typedef std::list< TransitionPresetPtr > TransitionPresetList;
typedef boost::unordered_map< OUString, OUString, OUStringHash, comphelper::UStringEqual > UStringMap;

class TransitionPreset
{
public:
    static const TransitionPresetList& getTransitionPresetList();
    static bool importTransitionPresetList( TransitionPresetList& rList );

    void apply( SdPage* pSlide ) const;

    sal_Int16 getTransition() const { return mnTransition; }
    sal_Int16 getSubtype() const { return mnSubtype; }
    sal_Bool getDirection() const { return mbDirection; }
    sal_Int32 getFadeColor() const { return mnFadeColor; }

    const OUString& getUIName() const { return maUIName; }
    const OUString& getPresetId() const { return maPresetId; }

private:
    TransitionPreset( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode );

    sal_Int16 mnTransition;
    sal_Int16 mnSubtype;
    sal_Bool mbDirection;
    sal_Int32 mnFadeColor;
    OUString maPresetId;
    OUString maUIName;

    static bool importTransitionsFile( TransitionPresetList& rList,
                                       ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
                                       UStringMap& rTransitionNameMape,
                                       OUString aFilename );
};

}

#endif // _SD_TRANSITIONPRESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
