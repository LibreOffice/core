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
