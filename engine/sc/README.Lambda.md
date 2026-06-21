# `LAMBDA`

The `LAMBDA` operator builds a callable function out of a Calc formula.

## Using `LAMBDA`

* `LAMBDA` expects a list of parameter names, followed by an expression.

* All parameter names must be valid identifiers, and must not match any names
already defined by Calc, nor any possible cell references. Identifiers are not
case-sensitive. Identifiers must start with a letter, and may contain only
letters, numbers, and underscores (`_`).

* You may specify no parameters, if you want the function to operate the same
way every time it is called (this may be useful if its expression includes
volatile operators such as `RAND` or `TIME`).

* The result of a `LAMBDA` is a function. Don't expect that to show up in the
spreadsheet in any useful way! You must call the function to use it.

* Call a `LAMBDA` function just like you call a built-in function: put the name
of the function (either a named range, a cell reference, or the `LAMBDA`
expression itself) into your formula before a pair of parentheses containing the
arguments you want to pass to the function.

* If you are nesting one `LAMBDA` inside another, you may use the same parameter
name in both (if you really want - this can easily make things hard to read!).
The innermost `LAMBDA` specifying a parameter of a given name applies at each
level of scope.

* A function can call itself (this is called recursion), but you must be able to
refer to it from within the function. For example, if cell `A1` is a `LAMBDA`
formula, the body of the `LAMBDA` can call itself by referring to `A1`. You can
name your `LAMBDA` cell to make this easier to read.

### Examples

(This form is permitted, but not recommended, as it is difficult to read.)

> `A1`: `=LAMBDA(a,b,a+b)(3*5,7/(6+4))`

`A1` will show `15.7` (=`(3*5)+(7/(6+4))` )

---

> `A1`: `=LAMBDA(a,b,a+b)`
>
> `B1`: `=A1(3*5, 7/(6+4))`

`B1` will show `15.7`

---

If `MY_SUM` is `A1`:

> `A1`: `=LAMBDA(a,b,a+b)`
>
> `B1`: `=MY_SUM(3*5,7/(6+4))`

`B1` will show `15.7`

---

If `FACT` is `A1`:

> `A1`: `=LAMBDA(n,IF(n>1,n*FACT(n-1),1))`
>
> `B1`: `=FACT(5)`

`B1` will show `120` (`FACT` duplicates the functionality of the built-in
`FACTORIAL` function, but is less efficient)

## How it Works

* The `ocLambda` `OpCode` corresponds to the `LAMBDA` operator

* `ocLambda` is set up as a jump `OpCode`, meaning that it has special rules for
what parameters are evaluated, and when.

* The first stage of the compiler tokenizes the elements of the `LAMBDA`
expression, like any other. It also looks for the lambda parameters, and interns
them as identifiers.

* The second stage of the compiler sets up the jump table with the parameters
and body.

* When interpreted, `ocLambda` causes a `ScFormulaFunction` to be built using
the contents of the jump table.

* `ScFormulaFunction`'s constructor clones the lambda body and saves the
interpreter's contextual information, then locates each use of each parameter in
the scope of that identifier in the body and makes note of where it is.

* When a `ScFormulaFunction` is called via `ScInterpreter::ScCall`, it builds a
new interpreter out of the saved context, performs the replacements specified,
and executes the interpreter to obtain a result. This means that the `LAMBDA`
body is executed in the same context that the `LAMBDA` appeared in.

* `ocLambda` is itself nonvolatile, and overrides any volatility in its body,
since the `LAMBDA` never produces a different callable, no matter what functions
appear in the body.

* The presence of an enclosing `LAMBDA` also prevents circular references from
being a problem, as the body of a `LAMBDA` is not executed until the cell
containing it already has a value. This allows recursion.
