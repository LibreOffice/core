#ifndef _SD_ANIMATIONS_HXX_
#define _SD_ANIMATIONS_HXX_

namespace sd
{

/** stores the link between an after effect node and its master for later insertion
    into the timing hierarchie
*/
struct AfterEffectNode
{
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxNode;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxMaster;
    sal_Int32 mnMasterRel;

    AfterEffectNode( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode, const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xMaster, sal_Int32 nMasterRel )
        : mxNode( xNode ), mxMaster( xMaster ), mnMasterRel( nMasterRel ) {}
};

typedef std::list< AfterEffectNode > AfterEffectNodeList;

/** inserts the animation node in the given AfterEffectNode at the correct position
    in the timing hierarchie of its master */
void stl_process_after_effect_node_func(AfterEffectNode& rNode);

}; // namespace sd;

#endif
