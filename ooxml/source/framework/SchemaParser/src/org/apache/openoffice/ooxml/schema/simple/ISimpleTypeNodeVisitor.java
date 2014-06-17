package org.apache.openoffice.ooxml.schema.simple;

public interface ISimpleTypeNodeVisitor
{
    void Visit (final BlobNode aType);
    void Visit (final DateTimeNode aType);
    void Visit (final NumberNode<?> aType);
    void Visit (final StringNode aType);
    void Visit (final UnionNode aType);
}
