# OOXML Formula Rewriter

OOXML spells some operators differently than Calc:

| Calc    | OOXML                   |
|---------|-------------------------|
| @A1     | _xlfn.SINGLE(A1)        |
| A1#     | _xlfn.ANCHORARRAY(A1)   |
| A1~B1   | (A1,B1)                 |

Each needs a parenthesis pair around the operand, so the rewriter has
to know where the operand begins and ends.

The # also changes place: behind its operand in Calc, in front of it
in OOXML. So it has two opcodes. ocSpill is the postfix operator and
has no OOXML spelling, ocAnchorArray is the call and has no native
one. Neither can be written into the wrong grammar.

## Operator priority

From the highest priority to the lowest, so 1 takes its operand
before 2 does:

1. # operator: only the factor directly before it. 
2. Reference operators: range (:), intersection (! or a blank),
   union (~), calls.
3. @ operator: the whole reference expression under it, so group 2
   stays inside its operand.
4. Percent (%) sign.
5. Prefix plus and minus: -A1# negates the spill range.
6. Power, multiplication, division, addition, subtraction,
   concatenation, comparisons.

## How it works

The rewriter runs on a bare token array, without a parse and
sometimes without compiled code, so it re-derives the operator
priority from the token order. It is a second statement of the
grammar: when the binding of an operator changes in FormulaCompiler,
the rewriter has to change too.

