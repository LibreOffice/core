# Reference lists, intersections and spilled ranges

What the interpreter answers for the union, intersection and
dynamic-array operators.

## Evaluation

| formula                  | answer  | because                       |
|--------------------------|---------|-------------------------------|
| =SUM((A1,B1))            | the sum | SUM reads the list whole      |
| =(A1~B1)                 | #VALUE! | a bare list has no value      |
| =(A1~B1)%, =(A1~B1)&"x"  | #VALUE! | no single value to work on    |
| =(A1, B1%)               | #VALUE! | a value part joined the list  |
| =(A1, #REF!)             | #REF!   | an error part is the answer   |
| =A1:B1 (C1:D1)           | #NULL!  | the ranges do not overlap     |
| =(A1:B1)#                | #REF!   | # over more than one cell     |

A list part joins the union when it starts with a reference or with
something that returns one: a nested list, IF, CHOOSE, IFS, SWITCH,
XLOOKUP, INDEX, INDIRECT, OFFSET, @, or an error constant. What the
part then does to the reference is settled at calculation time, as in
the table. A part starting with anything else does not parse. The
producing application refuses the same shapes.

## Spilled value position

=A1# in a plain cell shows the spill value of its own row,
and #VALUE! outside the spill. =@(A1#*2) shows the top-left value
everywhere: once an operator computes over the spill the position is
gone. The producing application behaves the same, so this is not a
bug.

The # operator pushes the spill values as a matrix that remembers the
range it came from (ScMatrixRangeToken). Elementwise operators keep
the range on a same-shaped result. Functions like SORT do not, they
may move values around. The token answers two questions
separately: where the values sit, and whether they still are the
cells' own, which is what ISREF asks.

