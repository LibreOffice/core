# Callable

A callable is a formula data type that represents code that can be called on a
(possibly empty) set of arguments to produce a result.

## Using Callables

The following elements are involved in using callables:

* The `StackVar` enum provides `svCallable` to identify callable objects.

* The `FormulaCallableToken` class allows you to create callable tokens.
Pass a `FormulaCallableRef` to the constructor. Use `GetCallable` to
retrieve the reference.

* The `FormulaCallable` class contains the data for a callable object. It is an
abstract base class; you should not instantiate it directly. Use
`FormulaCallableRef` to refer to these objects.

* Three subclasses of `FormulaCallable` have been implemented:

    1. The `FormulaBuiltInFunction` represents a function that is provided by
Calc.

        * The `FormulaBuiltInFunction` class stores a table of references to
functions that have been accessed. The class should not be instantiated
directly.

        * If you have an `OpCode`, you can retrieve the corresponding
`FormulaBuiltInFunction` reference by calling `FormulaBuiltInFunction::Get()`.

    2. The `FormulaExternalFunction` class represents a function that is
external to Calc. These are provided by means of add-ins.

        * Pass the name of the external function to the constructor to
instantiate an object.

    4. The `ScMacroFunction` class represents a function that is defined as a
macro.
        * Pass the `ScInterpreter` and the name of the macro to the constructor
to instantiate an object.

* The `OpCode` enum provides `ocCall` as a left-associative binary operator.
This operator is automatically inserted by `ScCompiler` between the callable
name (or expression) and the opening parenthesis of the argument list. The next
phase of the compiler (in `FormulaCompiler`) acts on that `OpCode` to properly
build the call in RPN form.

* To execute a `FormulaCallable`, use `ScInterpreter`'s `ScCall` method. Provide
a reference to the `FormulaCallable`, along with either a vector of arguments
(`FormulaConstTokenRef`) or the number of arguments to extract from the top of
the interpreter's stack. The result is placed on the top of the stack.

## How it Works

### Constructing

* `ScCompiler::CompileString` identifies the use of a built-in operator without
a following parenthesis, and inserts a `FormulaBuiltInFunction` there instead.
If a (left) parenthesis does follow the reference to a built-in operator, its
`OpCode` is used instead. This makes things more efficient during
interpretation.

* `ScCompiler::CompileString` also identifies the use of an external function
without a following parenthesis, and inserts a `FormulaExternalFunction` there
instead. As with a built-in function, if an external function is called
immediately, no callable is created.

* Macro references produce `ocMacro` tokens (in `ScCompiler::CompileString`),
which generate `ScMacroFunction` objects when interpreted.
`ScInterpreter::ScCall` can call these.

### Compiling

1. `ScCompiler::CompileString` identifies when an open parenthesis appears after
either a closing parenthesis, a reference, or a name that doesn't match a
built-in operator, and injects `ocCall` into the output token stream before
continuing.

2. `FormulaCompiler::CompileTokenArray` includes `CallLine` in its binary
operator precedence rules. `CallLine` detects `ocCall` and builds the necessary
RPN syntax for a call.

### Interpreting

1. `ScInterpreter::ScCall` collects the arguments provided and the
`FormulaCallable` reference, and passes them on to the other
`ScInterpreter::ScCall` methods.

2. There are two versions of `ScInterpreter::ScCall` which, combined, are
capable of calling the different types of functions. The first accepts a
`FormulaCallable` and an integer, which indicates how many arguments are being
passed to it on the stack. The other accepts a vector of `FormulaConstTokenRef`
objects, which supplies the argument list directly. The idea is to be able to
call whichever method corresponds to the most convenient means of passing the
arguments, and let the two versions of `ScInterpreter::ScCall` figure out how
to actually make the call.

3. If a `FormulaCallable` needs to be called by any other means, one of the
versions of the `ScInterpereter::ScCall` method that accepts arguments may be
used. The result appears on the top of the stack.

### Subclassing

A subclass must implement a `GetOpCode` method, whose result uniquely determines
which subclass it is. `FormulaBuiltInFunction` returns an `OpCode` corresponding
to one of the built-in operators. `FormulaExternalFunction` returns ocExternal,
and `ScMacroFunction` returns `ocMacro`. The versions of `ScInterpreter::ScCall`
use this information to determine which subclass is being called, and perform
the actions necessary to implement the call. Note that each subclass has a
preferred means of receiving arguments; the other version of
`ScInterpreter::ScCall` just sets up the arguments and calls the preferred
version.
