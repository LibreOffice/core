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

#ifndef INCLUDED_SD_INC_TRANSITIONPRESET_HXX
#define INCLUDED_SD_INC_TRANSITIONPRESET_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <list>
#include <memory>
#include <unordered_map>

namespace com { namespace sun { namespace star {
    namespace animations { class XAnimationNode; }
    namespace uno { template<class X> class Reference; }
} } }

namespace sd {

class TransitionPreset;
typedef std::shared_ptr< TransitionPreset > TransitionPresetPtr;
typedef std::list< TransitionPresetPtr > TransitionPresetList;
typedef std::unordered_map< OUString, OUString > UStringMap;

class TransitionPreset
{
public:
    static const TransitionPresetList& getTransitionPresetList();
    static bool importTransitionPresetList( TransitionPresetList& rList );

    sal_Int16 getTransition() const { return mnTransition; }
    sal_Int16 getSubtype() const { return mnSubtype; }
    bool getDirection() const { return mbDirection; }
    sal_Int32 getFadeColor() const { return mnFadeColor; }

    const OUString& getPresetId() const { return maPresetId; }
    const OUString& getSetId() const { return maSetId; }
    const OUString& getSetLabel() const { return maSetLabel; }
    const OUString& getVariantLabel() const { return maVariantLabel; }

private:
    TransitionPreset( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    sal_Int16 mnTransition;
    sal_Int16 mnSubtype;
    bool mbDirection;
    sal_Int32 mnFadeColor;
    OUString maPresetId;
    OUString maSetId;
    OUString maSetLabel;
    OUString maVariantLabel;

    static bool importTransitionsFile( TransitionPresetList& rList,
                                       css::uno::Reference< css::lang::XMultiServiceFactory > const & xServiceFactory,
                                       const OUString& aFilename );
};

}

#endif // INCLUDED_SD_INC_TRANSITIONPRESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
