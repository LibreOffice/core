package org.apache.openoffice.ooxml.schema.model.optimize;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import org.apache.openoffice.ooxml.schema.iterator.NodeIterator;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;

public class SchemaOptimizer
{
    public SchemaOptimizer (final Schema aOriginalSchema)
    {
        maOriginalSchema = aOriginalSchema;
        maOptimizedSchema = new Schema();
        maTodoList = new LinkedList<>();
        maProcessedTypes = new HashSet<>();

        maRequestVisitor = new RequestVisitor(maOriginalSchema, this);
    }




    public Schema Run ()
    {
        // Copy over the top-level elements.  They are the seeds in the use chain of types.
        for (final Element aElement : maOriginalSchema.TopLevelElements.GetUnsorted())
            maOptimizedSchema.TopLevelElements.Add(aElement);

        // Seed the todo list with the types to which the top-level elements lead.
        for (final Element aElement : maOriginalSchema.TopLevelElements.GetUnsorted())
            RequestType(aElement.GetTypeName());

        ProcessTodoList();

        return maOptimizedSchema;
    }




    void RequestType (final QualifiedName aName)
    {
        final Node aNode = maOriginalSchema.GetTypeForName(aName);
        if (aNode == null)
            throw new RuntimeException("there is no type named '"+aName+"' in the schema");
        else
            RequestType(aNode);
    }




    void RequestType (final INode aNode)
    {
        if (aNode.GetNodeType() == NodeType.SimpleTypeReference)
            System.out.println(aNode);
        if ( ! maProcessedTypes.contains(aNode))
        {
            maProcessedTypes.add(aNode);
            maTodoList.add(aNode);
        }
    }




    /** Process each entry in the todo list until it is empty.
     *  Each type in it is used and inserted into the optimized schema.
     */
    private void ProcessTodoList ()
    {
        final INodeVisitor aVisitor = new ProcessTypeVisitor(
            maOriginalSchema,
            maOptimizedSchema,
            this);
        while ( ! maTodoList.isEmpty())
        {
            final INode aNode = maTodoList.poll();
            aNode.AcceptVisitor(aVisitor);

            // Request used namespaces.
            final QualifiedName aName = aNode.GetName();
            if (aName != null)
                maOptimizedSchema.Namespaces.ProvideNamespace(aName.GetNamespaceURI(), aName.GetNamespacePrefix());
        }
    }




    /** Iterate over all nodes in the given type definition.
     *  References to types are inserted into the todo list.
     */
    void RequestReferencedTypes (final Node aRoot)
    {
        for (final INode aNode : new NodeIterator(aRoot))
        {
            aNode.AcceptVisitor(maRequestVisitor);
        }
    }




    private final Schema maOriginalSchema;
    private final Schema maOptimizedSchema;
    private final Queue<INode> maTodoList;
    private final Set<INode> maProcessedTypes;
    private final RequestVisitor maRequestVisitor;
}
