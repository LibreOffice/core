/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
