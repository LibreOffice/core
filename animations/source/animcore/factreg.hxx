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
#include <rtl/unload.h>

namespace animcore {

extern rtl_StandardModuleCount g_moduleCount;

#define DECL_NODE_FACTORY(N)\
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance_##N( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rSMgr ) throw (::com::sun::star::uno::Exception);\
extern OUString getImplementationName_##N();\
extern ::com::sun::star::uno::Sequence< OUString> getSupportedServiceNames_##N(void)

DECL_NODE_FACTORY( PAR );
DECL_NODE_FACTORY( SEQ );
DECL_NODE_FACTORY( ITERATE );
DECL_NODE_FACTORY( ANIMATE );
DECL_NODE_FACTORY( SET );
DECL_NODE_FACTORY( ANIMATECOLOR );
DECL_NODE_FACTORY( ANIMATEMOTION );
DECL_NODE_FACTORY( ANIMATETRANSFORM );
DECL_NODE_FACTORY( TRANSITIONFILTER );
DECL_NODE_FACTORY( AUDIO );
DECL_NODE_FACTORY( COMMAND );
DECL_NODE_FACTORY( TargetPropertiesCreator );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
