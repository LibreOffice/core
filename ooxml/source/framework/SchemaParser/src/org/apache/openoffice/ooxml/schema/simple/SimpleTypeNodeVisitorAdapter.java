package org.apache.openoffice.ooxml.schema.simple;

public class SimpleTypeNodeVisitorAdapter
    implements ISimpleTypeNodeVisitor
{
    @Override
    public void Visit(BlobNode aType)
    {
        Default(aType);
    }

    @Override
    public void Visit(DateTimeNode aType)
    {
        Default(aType);
    }

    @Override
    public void Visit(NumberNode<?> aType)
    {
        Default(aType);
    }

    @Override
    public void Visit(StringNode aType)
    {
        Default(aType);
    }

    @Override
    public void Visit(UnionNode aType)
    {
        Default(aType);
    }

    public void Default (final ISimpleTypeNode aType)
    {
    }
}
