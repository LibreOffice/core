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

#ifndef _SD_ANIMATIONS_HXX_
#define _SD_ANIMATIONS_HXX_

#include <sddllapi.h>

namespace sd
{

/** stores the link between an after effect node and its master for later insertion
    into the timing hierarchie
*/
struct AfterEffectNode
{
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxNode;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxMaster;
    bool mbOnNextEffect;

    AfterEffectNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xMaster, bool bOnNextEffect )
        : mxNode( xNode ), mxMaster( xMaster ), mbOnNextEffect( bOnNextEffect ) {}
};

typedef std::list< AfterEffectNode > AfterEffectNodeList;

/** inserts the animation node in the given AfterEffectNode at the correct position
    in the timing hierarchie of its master */
SD_DLLPUBLIC void stl_process_after_effect_node_func(AfterEffectNode& rNode);

} // namespace sd;

#endif
