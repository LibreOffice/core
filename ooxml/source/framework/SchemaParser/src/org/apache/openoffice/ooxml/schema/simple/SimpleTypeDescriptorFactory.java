package org.apache.openoffice.ooxml.schema.simple;

import java.util.HashMap;
import java.util.Map;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;
import org.apache.openoffice.ooxml.schema.model.simple.Union;

public class SimpleTypeDescriptorFactory
    extends NodeVisitorAdapter
{
    class TypeCounter
        extends NodeVisitorAdapter
    {
        public final Map<NodeType,Integer> Counters;
        TypeCounter ()
        {
            Counters = new HashMap<>();
            for (final NodeType eType : NodeType.values())
                Counters.put(eType, 0);
        }
        @Override
        public void Default (final INode aNode)
        {
            Counters.put(aNode.GetNodeType(), Counters.get(aNode.GetNodeType())+1);
        }
    }




    public static SimpleTypeDescriptor CreateSimpleTypeDescriptor(
        final SimpleType aSimpleType,
        final SchemaBase aSchemaBase,
        final Log aLog)
    {
        final SimpleTypeDescriptorFactory aFactory = new SimpleTypeDescriptorFactory(
            aSchemaBase,
            aLog);

        aLog.AddComment("Simple type %s, defined at %s",
            aSimpleType.GetName(),
            aSimpleType.GetLocation().toString());

        final ISimpleTypeNode aSubType = aFactory.ProcessSimpleType(aSimpleType);
        final SimpleTypeDescriptor aDescriptor = new SimpleTypeDescriptor(aSimpleType.GetName());
        aSubType.AcceptVisitor(new SimpleTypeNodeVisitorAdapter ()
        {
            @Override public void Visit (final UnionNode aType)
            {
                aDescriptor.SetSubTypes(aType.GetChildren());
            }

            @Override public void Default (final ISimpleTypeNode aType)
            {
                aDescriptor.SetSubTypes(new ISimpleTypeNode[]{aSubType});
            }
        });

        aDescriptor.Print(aLog);
        aLog.printf("\n");

        return aDescriptor;
    }




    private SimpleTypeDescriptorFactory (
        final SchemaBase aSchemaBase,
        final Log aLog)
    {
        maSchemaBase = aSchemaBase;
        maLog = aLog;
        maResult = null;
    }




    ISimpleTypeNode ProcessSimpleType (final SimpleType aSimpleType)
    {
        return ApplyVisitor(aSimpleType);
    }




    @Override
    public void Visit (final BuiltIn aNode)
    {
        assert(aNode.GetChildCount() == 0);
        assert(maResult == null);

        maLog.AddComment("builtin %s", aNode.toString());

        switch(aNode.GetBuiltInType())
        {
            case Double:
                maResult = new NumberNode<Double>(aNode.GetBuiltInType());
                break;
            case Float:
                maResult = new NumberNode<Float>(aNode.GetBuiltInType());
                break;

            case Boolean:
                maResult = new NumberNode<Boolean>(aNode.GetBuiltInType());
            case Integer:
                maResult = new NumberNode<Long>(aNode.GetBuiltInType());
                break;
            case Byte:
                maResult = new NumberNode<Byte>(aNode.GetBuiltInType());
            case Int:
                maResult = new NumberNode<Integer>(aNode.GetBuiltInType());
            case Long:
                maResult = new NumberNode<Long>(aNode.GetBuiltInType());
            case Short:
                maResult = new NumberNode<Short>(aNode.GetBuiltInType());
            case UnsignedByte:
                maResult = new NumberNode<Integer>(aNode.GetBuiltInType());
            case UnsignedInt:
                maResult = new NumberNode<Long>(aNode.GetBuiltInType());
            case UnsignedLong:
                maResult = new NumberNode<Long>(aNode.GetBuiltInType());
            case UnsignedShort:
                maResult = new NumberNode<Integer>(aNode.GetBuiltInType());
                break;

            case AnyURI:
            case ID:
            case NcName:
            case String:
            case Token:
                maResult = new StringNode(aNode.GetBuiltInType());
                break;

            case Base64Binary:
            case HexBinary:
                maResult = new BlobNode(aNode.GetBuiltInType());
                break;

            case DateTime:
                maResult = new DateTimeNode(aNode.GetBuiltInType());
                break;

            default:
                throw new RuntimeException(aNode.toString()+" is not supported");
        }
    }




    @Override
    public void Visit (final List aNode)
    {
        maLog.AddComment("list of type %s", aNode.GetItemType().toString());
        maLog.StartBlock();
        final ISimpleTypeNode aItemType = ApplyVisitor(maSchemaBase.GetSimpleTypeForName(aNode.GetItemType()));
        maLog.EndBlock();

        aItemType.SetIsList();
        maResult = aItemType;
    }




    @Override
    public void Visit (final Restriction aNode)
    {
        assert(aNode.GetChildCount() == 0);

        maLog.AddComment("%s", aNode.toString());

        final INode aBaseType = maSchemaBase.GetSimpleTypeForName(aNode.GetBaseType());
        if (aBaseType == null)
            throw new RuntimeException("got no type for name "+aNode.GetBaseType());
        maLog.StartBlock();
        maResult = ApplyVisitor(aBaseType);
        maLog.EndBlock();
        maResult.ApplyRestriction(
            aNode,
            maSchemaBase.AttributeValueToIdMap);
    }




    @Override
    public void Visit (final SimpleType aNode)
    {
        maLog.AddComment(aNode.toString());

        assert(aNode.GetChildCount() <= 1);
        switch(aNode.GetChildCount())
        {
            case 0:
                maResult = null;
                break;
            case 1:
                maLog.StartBlock();
                maResult = ApplyVisitor(aNode.GetOnlyChild());
                maLog.EndBlock();
                break;
            default:
                throw new RuntimeException();
        }
    }




    @Override
    public void Visit (final SimpleTypeReference aNode)
    {
        maLog.AddComment("reference to %s", aNode.GetReferencedTypeName());

        maLog.StartBlock();
        maResult = ApplyVisitor(aNode.GetReferencedNode(maSchemaBase));
        maLog.EndBlock();
    }




    @Override
    public void Visit (final Union aNode)
    {
        maLog.AddComment("union");

        final UnionNode aUnion = new UnionNode();

        // Make sure that all children have compatible types and value sets.
        maLog.StartBlock();

        for (final INode aChild : aNode.GetChildren())
        {
            aUnion.AddNode(ApplyVisitor(aChild));
        }

        maLog.EndBlock();

        maResult = aUnion;
    }




    @Override
    public void Default (final INode aNode)
    {
        switch(aNode.GetNodeType())
        {
            default:
                throw new RuntimeException(aNode.GetNodeType() +" is not yet supported");
        }
    }




    ISimpleTypeNode ApplyVisitor (final INode aNode)
    {
        aNode.AcceptVisitor(this);
        final ISimpleTypeNode aResult = maResult;
        maResult = null;
        return aResult;
    }




    private final SchemaBase maSchemaBase;
    private final Log maLog;
    private ISimpleTypeNode maResult;
}
