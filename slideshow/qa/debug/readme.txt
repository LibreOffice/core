Recognized preprocessor switches
================================

 - VERBOSE=t: enable extra verbose trace output (debug=t builds only)
 - TIMELOG=t: enable time logging (also for product builds)


Debug tools overview
====================

nodetree.sh:
 
 Generates a number of tree graphs from a trace output file, showing
 the tree of AnimationNodes at various instances of the
 slideshow. Each node has a fill color corresponding to its current
 state, a legend for the color values can be obtained by the colors.sh
 script.
 To generate output at other instances, use the DEBUG_NODES_SHOWTREE
 macro in animationnodefactory.cxx.
 