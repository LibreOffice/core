/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include <rtl/unload.h>

namespace animcore {

extern rtl_StandardModuleCount g_moduleCount;

#define DECL_NODE_FACTORY(N)\
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance_##N( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rSMgr ) throw (::com::sun::star::uno::Exception);\
extern ::rtl::OUString getImplementationName_##N();\
extern ::com::sun::star::uno::Sequence< ::rtl::OUString> getSupportedServiceNames_##N(void)

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
