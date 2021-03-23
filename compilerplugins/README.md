Compiler plugins.


== Overview ==

This directory contains code for compiler plugins. These are used to perform
additional actions during compilation (such as additional warnings) and
also to perform mass code refactoring.

Currently only the Clang compiler is supported (http://wiki.documentfoundation.org/Development/Clang).


== Usage ==

Compiler plugins are enabled automatically by --enable-dbgutil if Clang headers
are found or explicitly using --enable-compiler-plugins.


== Functionality ==

There are two kinds of plugin actions:
- compile checks - these are run during normal compilation
- rewriters - these must be run manually and modify source files

Each source has a comment saying whether it's compile check or a rewriter
and description of functionality.

=== Compile checks ===

Used during normal compilation to perform additional checks.
All warnings and errors are marked '[loplugin]' in the message.


=== Rewriters ===

Rewriters analyse and possibly modify given source files.
Usage: make COMPILER_PLUGIN_TOOL=<rewriter_name>
Additional optional make arguments:
- it is possible to also pass FORCE_COMPILE_ALL=1 to make to trigger rebuild of all source files,
    even those that are up to date.
- UPDATE_FILES=<scope> - limits which modified files will be actually written back with the changes
    - mainfile - only the main .cxx file will be modified (default)
    - all - all source files involved will be modified (possibly even header files from other LO modules),
        3rd party header files are however never modified
    - <module> - only files in the given LO module (toplevel directory) will be modified (including headers)

Modifications will be written directly to the source files.

Some rewriter plugins are dual-mode and can also be used in a non-rewriting mode
in which they emit warnings for problematic code that they would otherwise
automatically rewrite.  When any rewriter is enabled explicitly via "make
COMPILER_PLUGIN_TOOL=<rewriter_name>" it works in rewriting mode (and all other
plugins are disabled), but when no rewriter is explicitly enabled (i.e., just
"make"), all dual-mode rewriters are enabled in non-rewriting mode (along with
all non-rewriter plugins; and all non--dual-mode plugins are disabled).  The
typical process to use such a dual-mode rewriter X in rewriting mode is

  make COMPILER_PLUGIN_WARNINGS_ONLY=X \
  && make COMPILER_PLUGIN_TOOL=X FORCE_COMPILE_ALL=1 UPDATE_FILES=all

which first generates a full build without failing due to warnings from plugin
X in non-rewriting mode (in case of --enable-werror) and then repeats the build
in rewriting mode (during which no object files are generate).


== Code documentation / howtos ==

http://wiki.documentfoundation.org/Clang_plugins

