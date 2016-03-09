Contains non-graphical helper code for office applications.

Specifically this module does not depend on or use includes from module
vcl. Originally all code in svtools that did not depend on vcl was split
off into this svl ("svtools light") module.

In particular the SfxItemSet is a property-bag like container that
stores arbitrary sets of properties for everything from text run
formats, to Chart regression line properties.

There are lots of other useful helpers in here for various office
tasks; much of this code was originally moved from svx/sfx2.

== Items, Pools and Sets ==

=== SfxPoolItem ===

A small reference counted piece of data.  Many subclasses, each with a
unique integer to identify its type (WhichId).  Can be compared for equality
(operator==), Clone()d, and converted to/from uno::Any (QueryValue/PutValue).

A pool item may have value semantics ("poolable"), meaning that
there will generally be only one instance that compares equal per item pool,
or not, in which case the item will be Clone()d quite a bit.

=== SfxItemPool ===

Usually there is one item pool per document, with a range of valid WhichIds
that is specific to the type of document.

The item pool owns all instances of SfxPoolItem or its subclasses that have
ever been added to an item set.  It also contains a default item for
every WhichId, which will be (depending on parameters) returned from item
sets if the set does not contain an item at this WhichId.

=== SfxItemSet ===

The item set can be created with a user-supplied range of WhichIds; it
will accept SfxPoolItems with matching WhichIds and ignore attempts to
insert items with non-matching WhichIds.

Items that are successfully inserted into the set will be stored in the
set's SfxItemPool, and for poolable items only a single instance that
compares equal under the predicate operator== will be stored in the pool,
regardless of how many sets contain it, thus conserving memory.

There are members m_pWhichRanges for the valid ranges (as pairs of WhichIds),
m_nCount for the number of items contained, and m_pItems for the pointers to
the actual items.

