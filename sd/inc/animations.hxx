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

#ifndef INCLUDED_SD_INC_ANIMATIONS_HXX
#define INCLUDED_SD_INC_ANIMATIONS_HXX

#include "sddllapi.h"

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::animations { class XAnimationNode; }

namespace sd
{

/** stores the link between an after effect node and its master for later insertion
    into the timing hierarchy
*/
struct AfterEffectNode
{
    css::uno::Reference< css::animations::XAnimationNode > mxNode;
    css::uno::Reference< css::animations::XAnimationNode > mxMaster;
    bool const mbOnNextEffect;

    AfterEffectNode( const css::uno::Reference< css::animations::XAnimationNode >& xNode, const css::uno::Reference< css::animations::XAnimationNode >& xMaster, bool bOnNextEffect )
        : mxNode( xNode ), mxMaster( xMaster ), mbOnNextEffect( bOnNextEffect ) {}
};

/** inserts the animation node in the given AfterEffectNode at the correct position
    in the timing hierarchy of its master */
SD_DLLPUBLIC void stl_process_after_effect_node_func(AfterEffectNode const & rNode);

} // namespace sd;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
