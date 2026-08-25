# Dynamic-array formulas in the OOXML filter

How spilling formulas and the @ and # operators look in an xlsx file
and what the import has to do about it.

## Storage

A spilling formula is an array formula with dynamic-array metadata:

    <c r="A1" cm="1"><f t="array" ref="A1:A3">SEQUENCE(3)</f>...

A non-spilling formula is a plain f element. The same formula text
behaves differently depending on the metadata.

Operator spellings:

| meaning                  | file spelling               |
|--------------------------|-----------------------------|
| @A1 (one value)          | _xlfn.SINGLE(A1)            |
| A1# (the spill range)    | _xlfn.ANCHORARRAY(A1)       |
| A1~B1 (union)            | (A1,B1)                     |
| A1!B1 (intersection)     | A1 B1 (a blank)             |

There is no #SPILL! literal. A blocked spill is saved as a
cached #VALUE! plus the metadata. The import keeps such a cell in
spill state.

## Dropped @

=@A1# is saved as plain _xlfn.ANCHORARRAY(A1) without metadata. The
missing metadata is what means "reduce to one value here".

The export writes no _xlfn.SINGLE call when the operand of the @ is
a reference: the signs in front of it, one cell, range, name or
external reference, and at most the # behind it. Such a cell already
means one value without the call, and a reader that does not know
the call still gets the formula. An operand that computes keeps the
call: =@(A1#*2) is saved as _xlfn.SINGLE((_xlfn.ANCHORARRAY(A1)*2)).

The import puts the @ back for one reference plus the #, nothing
else. A wider reference operand comes back without the @ and reduces
at evaluation to the same value. An @ added anywhere else changes
what the formula computes.

## Import steps

1. Turn _xlfn.ANCHORARRAY(x) into the postfix x# and remove doubled
   parentheses behind _xlfn.SINGLE. The match is on the opcode, not 
   on where the token sits.
2. Compile the tokens still under the OOXML grammar. The blank as
   intersection and the comma as union are baked into the token array
   in this compile. Later recompiles use the document grammar, where
   a blank means nothing.
3. Put the @ back on the dropped-@ shape.

## XLS has no @

The binary format encodes no @, so the import derives it: an RPN
walk adds one when the result would be an array. The format writes
a plain cell reference as a range over one cell, so many cells come
out of an XLS with an @ over nothing but a reference. Those cells
are why the export leaves the _xlfn.SINGLE call out.

