package org.apache.openoffice.ooxml.schema.simple;

import java.util.Map;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

/** Represent a single node in the node tree of a simple type.
 */
public interface ISimpleTypeNode
{
    void ApplyRestriction (
        final Restriction aNode,
        final Map<String,Integer> aValueToIdMap);
    void Print (final Log aLog);

    /** List elements are not represented by their own node (type).
     *  There is only this flag that makes this node a list of the item type
     *  which is represented by the node.
     */
    boolean IsList ();

    /** Set the IsList flag.
     */
    void SetIsList ();
    void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor);
}
