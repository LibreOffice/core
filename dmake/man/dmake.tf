.\" Copyright (c) 1990,...,1995 Dennis Vadura, All rights reserved.
.\"
.\" You must use groff to format this man page!!!
.\"
.ds TB "0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.2i +0.5i +0.5i +2.0i
.de Ip
.sp \\n[PD]u
.fi
.nr an-prevailing-indent (n;\w@\\$1 @u)
.it 1 an-trap
.if !\\n[an-div?] .di an-div
.in 0
.nr an-div? 1
.nf
\&\\$1 \\$2
..
.de Is
.nr an-prevailing-indent \w@\\$1@u
..
.de Ii
.fi
.it 1 an-trap
.if !\\n[an-div?] .di an-div
.in 0
.nr an-div? 1
.nf
\&\\$1
..
.TH DMAKE p  "UW" "Version 4.01 PL0" "Unsupported Free Software"
.SH NAME
\fBdmake\fR \- maintain program groups, or interdependent files
.SH SYNOPSIS
.B dmake
[\-P#] [\-{f|C|K} file] [\-{w|W} target ...]
[macro[[!][*][+][:]]=\fIvalue\fP ...]
[\-v{cdfimtw}] [\-ABcdeEghiknpqrsStTuVxX] [target ...]
.SH DESCRIPTION
.PP
.B dmake
is a re-implementation of the UNIX Make utility with significant enhancements.
.B dmake
executes commands found in an external file called a
.I makefile
to update one or more target names.
Each target may depend on zero or more prerequisite targets.
If any of the target's prerequisites is newer than the target or if the target
itself does not exist, then
.B dmake
will attempt to make the target.
.PP
If no
.B \-f
command line option is present then
.B dmake
searches for an existing
.I makefile
from the list of prerequisites specified for the special target \fI.MAKEFILES\fR
(see the STARTUP section for more details).
If "\-" is the name of the file specified to the
.B \-f
flag then \fBdmake\fR uses standard input as the source of the makefile text.
.PP
Any macro definitions (arguments with embedded "="
signs) that appear on the command line are processed first
and supercede definitions for macros of the same name found
within the makefile.  In general it is impossible for definitions found
inside the makefile to redefine a macro defined on the command line, see the
MACROS section for exceptions.
.PP
If no
.I target
names are specified on the command line, then \fBdmake\fR uses the first
non-special target found in the makefile as the default target.
See the
.B "SPECIAL TARGETS"
section for the list of special targets and their function.
Makefiles written for most previous
versions of
.I Make
will be handled correctly by 
.B dmake.
Known differences between \fBdmake\fR and other versions of make
are discussed in the
.B COMPATIBILITY
section found at the end of this document.
.B dmake
returns 0 if no errors were detected and a non-zero result if an error
occurred.
.SH OPTIONS
.IP "\fB\-A\fR"
Enable AUGMAKE special inference rule transformations
(see the "PERCENT(%) RULES" section), these are set to off by default.
.IP "\fB\-B\fR"
Enable the use of spaces instead of <tabs> to begin recipe lines.
This flag equivalent to the .NOTABS special macro and is further described
below.
.IP "\fB\-c\fR"
Use non-standard comment stripping.  If you specify \fB\-c\fP then
.B dmake
will treat any \fB#\fP character as a start of comment character wherever it
may appear unless it is escaped by a \e.
.IP "\fB\-C [+]file\fR"
This option writes to \fIfile\fP a copy of standard output and
standard error from any child processes and from the
.B dmake
process itself.  If you specify a \fB+\fP prior to the file name then
the text is appended to the previous contents of \fIfile\fP.
This option is active in the MSDOS implementation only and is ignored
by non-MSDOS versions of
.B dmake.
.IP "\fB\-d\fR"
Disable the use of the directory cache.  Normally \fBdmake\fP caches directories
as it checks file timestamps.  Giving this flag is equivalent to the
\&.DIRCACHE attribute or macro being set to \fIno\fP.
.IP "\fB\-E\fR"
Read the environment and define all strings of the
form '\fBENV\-VAR\fP=\fIevalue\fP'
defined within as macros whose name is \fBENV\-VAR\fP,
and whose value is '\fIevalue\fP'.
The environment is processed prior to processing the user
specified makefile thereby allowing definitions in the makefile to override
definitions in the environment.
.IP "\fB\-e\fR"
Same as \-E, except that the environment is processed after the
user specified makefile has been processed
(thus definitions in the environment override definitions in the makefile).
The \-e and \-E options are mutually exclusive.
If both are given the latter takes effect.
.IP "\fB\-f file\fR"
Use \fBfile\fR as the source for the makefile text.
Only one \fB\-f\fR option is allowed.
.IP "\fB\-g\fR"
Globally disable group recipe parsing, equivalent to the .IGNOREGROUP
attribute or macro being set to \fIyes\fP at the start of the makefile.
.IP "\fB\-h\fR"
Print the command summary for \fBdmake\fR.
.IP "\fB\-i\fR"
Tells \fBdmake\fR to ignore errors, and continue making other targets.
This is equivalent to the .IGNORE attribute or macro.
.IP "\fB\-K file\fR"
Turns on \fB.KEEP_STATE\fP state tracking and tells \fBdmake\fP to use
\fIfile\fP as the state file.
.IP "\fB\-k\fR"
Causes \fBdmake\fR to ignore errors caused by command execution and to make
all targets not depending on targets that could not be made. 
Ordinarily \fBdmake\fR stops after a command returns a non-zero status,
specifying \fB\-k\fR causes \fBdmake\fR to ignore the error
and continue to make as much as possible.
.IP "\fB\-n\fR"
Causes \fBdmake\fR to print out what it would have executed,
but does not actually execute the commands.  A special check is made for
the string "$(MAKE)" inside a recipe line, if it is found,
the line is expanded and invoked, thereby enabling recursive makes to give a
full description of all that they will do.
This check is disabled inside group recipes.
.IP "\fB\-p\fR"
Print out a version of the digested makefile in human readable form.
(useful for debugging, but cannot be re-read by \fBdmake\fP)
.IP "\fB\-P#\fR"
On systems that support multi-processing cause \fBdmake\fP to use \fI#\fP
concurrent child processes to make targets.
See the "MULTI PROCESSING" section for more information.
.IP "\fB\-q\fR"
Check and see if the target is up to date.  Exits with code 0 if up to date,
1 otherwise.
.IP "\fB\-r\fR"
Tells \fBdmake\fR not to read the initial startup makefile, see STARTUP
section for more details.
.IP "\fB\-s\fR"
Tells \fBdmake\fR to do all its work silently and not echo the commands it is
executing to stdout (also suppresses warnings).
This  is equivalent to the .SILENT attribute or macro.
.IP "\fB\-S\fR"
Force sequential execution of recipes on architectures which support
concurrent makes.  For backward compatibility with old makefiles that have
nasty side-effect prerequisite dependencies.
.IP "\fB\-t\fR"
Causes \fBdmake\fR to touch the targets and bring them up to date
without executing any commands.
Note that targets will not be created if they do not already exist.
.IP "\fB\-T\fR"
Tells \fBdmake\fP to not perform transitive closure on the inference graph.
.IP "\fB\-u\fR"
Force an unconditional update.  (ie. do everything that would
be done if everything that a target depended on was out of date)
.IP "\fB\-v[dfimtw]\fR"
Verbose flag, when making targets print to stdout what we are going to make
and what we think its time stamp is.  The optional flags \fB[dfimt]\fP can be
used to restrict the information that is displayed.  In the absence of any
optional flags all are assumed to be given (ie. \fB\-v\fP is equivalent to
\fB\-vdfimt\fP).  The meanings of the optional flags are:
.RS
.IP "\fBc\fP"
Notify of directory cache operations only.
.IP "\fBd\fP"
Notify of change directory operations only.
.IP "\fBf\fP"
Notify of file I/O operations only.
.IP "\fBi\fP"
Notify of inference algorithm operation only.
.IP "\fBm\fP"
Notify of target update operations only.
.IP "\fBt\fP"
Keep any temporary files created; normally they are automatically deleted.
.IP "\fBw\fP"
Notify of non-essential warnings (these are historical).
.RE
.IP "\fB\-V\fR"
Print the version of \fBdmake\fR, and values of builtin macros.
.IP "\fB\-W target\fR"
Run \fBdmake\fP pretending that \fItarget\fP is out of date.
.IP "\fB\-w target\fR"
\fIWhat if?\fP Show what would be made if \fItarget\fP were out of date.
.IP "\fB\-x\fR"
Upon processing the user makefile export all non-internally defined macros
to the user's environment.  This option together with the \-e option
allows SYSV AUGMAKE recursive makes to function as expected.
.IP "\fB\-X\fR"
Inhibit the execution of \fB#!\fP lines found at the beginning of a makefile.
The use of this flag prevents non-termination of recursive make invocations.
.SH INDEX
Here is a list of the sections that follow and a short description of each.
Perhaps you won't have to read the entire man page to find
what you need.
.IP \fBSTARTUP\fP 1.9i
Describes \fBdmake\fP initialization.
.IP \fBSYNTAX\fP 1.9i
Describes the syntax of makefile expressions.
.IP \fBATTRIBUTES\fP 1.9i
Describes the notion of attributes and how they are used when
making targets.
.IP \fBMACROS\fP 1.9i
Defining and expanding macros.
.IP "\fBRULES AND TARGETS" 1.9i
How to define targets and their prerequisites.
.IP \fBRECIPES\fP 1.9i
How to tell \fBdmake\fP how to make a target.
.IP "\fBTEXT DIVERSIONS\fP" 1.9i
How to use text diversions in recipes and macro expansions.
.IP "\fBSPECIAL TARGETS\fP" 1.9i
Some targets are special.
.IP "\fBSPECIAL MACROS\fP" 1.9i
Macros used by \fBdmake\fP to alter the processing of the makefile,
and those defined by \fBdmake\fP for the user.
.IP "\fBCONTROL MACROS\fP" 1.9i
Itemized list of special control macros.
.IP "\fBRUNTIME MACROS\fP" 1.9i
Discussion of special run-time macros such as $@ and $<.
.IP "\fBFUNCTION MACROS\fP" 1.9i
GNU style function macros, only $(mktmp ...) for now.
.IP "\fBCONDITIONAL MACROS\fP" 1.9i
Target specific conditional macros.
.IP "\fBDYNAMIC PREREQUISITES\fP" 1.9i
Processing of prerequisites which contain macro expansions in their name.
.IP "\fBBINDING TARGETS\fP" 1.9i
The rules that \fBdmake\fP uses to bind
a target to an existing file in the file system.
.IP "\fBPERCENT(%) RULES\fP" 1.9i
Specification of recipes to be used by the inference algorithm.
.IP "\fBMAKING INFERENCES\fP" 1.9i
The rules that \fBdmake\fP uses when inferring how to make a target which
has no explicit recipe.  This and the previous section are really a single
section in the text.
.IP "\fBMAKING TARGETS\fP" 1.9i
How \fBdmake\fP makes targets other than libraries.
.IP "\fBMAKING LIBRARIES\fP" 1.9i
How \fBdmake\fP makes libraries.
.IP "\fBKEEP STATE\fP" 1.9i
A discussion of how .KEEP_STATE works.
.IP "\fBMULTI PROCESSING\fP" 1.9i
Discussion of \fBdmake's\fP parallel make facilities for architectures that
support them.
.IP "\fBCONDITIONALS\fP" 1.9i
Conditional expressions which control the processing of the makefile.
.IP "\fBEXAMPLES\fP" 1.9i
Some hopefully useful examples.
.IP "\fBCOMPATIBILITY\fP" 1.9i
How \fBdmake\fP compares with previous versions of make.
.IP "\fBLIMITS\fP" 1.9i
Limitations of \fBdmake\fP.
.IP \fBPORTABILITY\fP 1.9i
Comments on writing portable makefiles.
.IP \fBFILES\fP 1.9i
Files used by \fBdmake\fP.
.IP "\fBSEE ALSO\fP" 1.9i
Other related programs, and man pages.
.IP "\fBAUTHOR\fP" 1.9i
The guy responsible for this thing.
.IP \fBBUGS\fP 1.9i
Hope not.
.SH STARTUP
When
.B dmake
begins execution it first processes the command line and then processes
an initial startup-makefile.
This is followed by an attempt to locate and process a user supplied makefile.
The startup file defines the default values of all required control macros
and the set of default rules for making targets and inferences.
When searching for the startup makefile,
.B dmake
searches the following locations, in the order specified,
until a startup file is located:
.LP
.RS
.IP 1.
The location given as the value of the macro
MAKESTARTUP defined on the command line.
.IP 2.
The location given as the value of the environment variable MAKESTARTUP
defined in the current environment.
.IP 3.
The location given as the value of the macro
MAKESTARTUP defined internally within \fBdmake\fP.
.RE
.LP
The above search is disabled by specifying the \-r option on the command line.
An error is issued if a startup makefile cannot be found and the \-r
option was not specified.
A user may substitute a custom startup file by defining
the MAKESTARTUP environment variable or by redefining the
MAKESTARTUP macro on the command line.
To determine where
.B dmake
looks for the default startup file, check your environment or issue the command
\fI"dmake \-V"\fP.
.PP
A similar search is performed to locate a default user makefile when no
\fB\-f\fP command line option is specified.
By default, the prerequisite list of the special target .MAKEFILES
specifies the names of possible makefiles and the search order that
\fBdmake\fP should use to determine if one exists.
A typical definition for this target is:
.RS
.sp
\&.MAKEFILES : makefile.mk Makefile makefile
.sp
.RE
\fBdmake\fP will first look for makefile.mk and then the others.
If a prerequisite
cannot be found \fBdmake\fP will try to make it before going on to the next
prerequisite.  For example, makefile.mk can be checked out of an RCS file
if the proper rules for doing so are defined in the startup file.
.PP
If the first line of the user makefile is of the form:
.RS
.sp
\#! command command_args
.sp
.RE
then \fBdmake\fP will expand and run the command prior to reading any
additional input.  If the return code of the command is zero then \fBdmake\fP
will continue on to process the remainder of the user makefile, if the return
code is non-zero then dmake will exit.
.PP
\fBdmake\fP builds the internal dependency graph as it parses a user specified
makefile.  The graph is rooted at the special target \fB.ROOT\fP. .ROOT is the
top level target that dmake builds when it starts to build targets.  All user
specified targets (those from the command line or taken as defaults from
the makefile) are made prerequisites of the special target \fB.TARGETS\fP.
\fBdmake\fP by default creates the relationship that .ROOT depends on .TARGETS
and as a result everything is made.  This approach allows the user to customize, within
their makefile, the order and which, target, is built first.  For example the
default makefiles come with settings for .ROOT that specify:
.sp
.RS
\&.ROOT .PHONY .NOSTATE .SEQUENTIAL : .INIT .TARGETS .DONE
.RE
.sp
with .INIT and .DONE defined as:
.sp
.RS
\&.INIT .DONE .PHONY:;
.RE
.sp
which nicely emulates the behaviour of Sun's make extensions.  The building of
\&.ROOT's prerequisites is always forced to be sequential.  However, this
definition is trivially chaned by supplying the definition:
.sp
.RS
\&.ROOT : .TARGETS
.RE
.sp
which skips the preamble and postamble phases of building .TARGETS.
.SH SYNTAX
This section is a summary of the syntax of makefile statements.
The description is given in a style similar to BNF, where { } enclose
items that may appear zero or more times, and [ ] enclose items that
are optional.  Alternative productions for a left hand side are indicated
by '\(->', and newlines are significant.  All symbols in \fBbold\fP type
are text or names representing text supplied by the user.
.sp 2
.RS
.Ip "Makefile" "\(-> { Statement }"
.Ip "Statement" "\(-> Macro-Definition"
\(-> Conditional-Macro-Definition
\(-> Conditional
\(-> Rule-Definition
\(-> Attribute-Definition
.Ip "Macro-Definition" "\(-> \fBMACRO = LINE\fP"
\(-> \fBMACRO [\fB!\fR]*= LINE\fP
\(-> \fBMACRO [\fB!\fR]:= LINE\fP
\(-> \fBMACRO [\fB!\fR]*:= LINE\fP
\(-> \fBMACRO [\fB!\fR]+= LINE\fP
\(-> \fBMACRO [\fB!\fR]+:= LINE\fP
.Ip "Conditional-Macro-Definition \(-> " "\fBTARGET\fP ?= Macro-Definition"
.Ip "Conditional \(-> " "\fB\&.IF\fR expression"
   Makefile
[ \fB.ELIF\fR expression
   Makefile ]
[ \fB.ELSE\fR
   Makefile ]
\fB\&.END\fR
.Ip "expression" "\(-> \fBLINE\fR"
\(-> \fBSTRING == LINE\fR
\(-> \fBSTRING != LINE\fR
.Ip "Rule-Definition \(-> " "target-definition"
   [ recipe ]
.PP
target-definition \(-> targets [attrs] op { \fBPREREQUISITE\fP } [\fB;\fR rcp-line]
.Ip "targets" "\(-> target { targets }"
\(-> \fB"\fRtarget\fB"\fR { targets }
.Ip "target" "\(-> special-target"
\(-> \fBTARGET\fR
.Ip "attrs" "\(-> attribute { attrs }"
\(-> \fB"\fRattribute\fB"\fR { attrs }
.Ip "op" "\(-> \fB:\fR { modifier }"
.Ip "modifier" "\(-> \fB:\fR"
\(-> \fB^\fR
\(-> \fB!\fR
\(-> \fB\-\fR
\(-> \fB|\fR
.Ip "recipe" "\(-> { \fBTAB\fR rcp-line }"
\(-> [\fB@\fR][\fB%\fR][\fB\-\fR] \fB[
.Is "recipe \(-> "
.Ii " "
   \fR{ \fBLINE\fR }
.Ii " "
\fB]\fR
.Ip "rcp-line" "\(-> [\fB@\fR][\fB%\fR][\fB\-\fR][\fB+\fR] \fBLINE\fR"
.Ip Attribute-Definition "\(-> attrs \fB:\fR targets"
.Ip attribute "\(-> \fB.EPILOG\fR"
\(-> \fB.ERRREMOVE\fR
\(-> \fB.EXECUTE\fR
\(-> \fB.GROUP\fR
\(-> \fB.IGNORE\fR
\(-> \fB.IGNOREGROUP\fR
\(-> \fB.LIBRARY\fR
\(-> \fB.MKSARGS\fR
\(-> \fB.NOINFER\fR
\(-> \fB.NOSTATE\fR
\(-> \fB.PHONY\fR
\(-> \fB.PRECIOUS\fR
\(-> \fB.PROLOG\fR
\(-> \fB.SETDIR=\fIpath\fP\fR
\(-> \fB.SILENT\fR
\(-> \fB.SEQUENTIAL\fR
\(-> \fB.SWAP\fR
\(-> \fB.USESHELL\fR
\(-> \fB.SYMBOL\fR
\(-> \fB.UPDATEALL\fR
.Ip "special-target" "\(-> \fB.ERROR\fR"
\(-> \fB.EXIT\fR
\(-> \fB.EXPORT\fR
\(-> \fB.GROUPEPILOG\fR
\(-> \fB.GROUPPROLOG\fR
\(-> \fB.IMPORT\fR
\(-> \fB.INCLUDE\fR
\(-> \fB.INCLUDEDIRS\fR
\(-> \fB.MAKEFILES\fR
\(-> \fB.REMOVE\fR
\(-> \fB.SOURCE\fR
\(-> \fB.SOURCE.\fIsuffix\fR
\(-> .\fIsuffix1\fR.\fIsuffix2\fR
.fi
.RE
.sp 1
.PP
Where, \fBTAB\fP represents a <tab> character, \fBSTRING\fP represents an
arbitrary sequence of characters, and
\fBLINE\fP represents a
possibly empty sequence of characters terminated by a non-escaped 
(not immediately preceded by a backslash '\e') new-line character.
\fBMACRO\fP, \fBPREREQUISITE\fP,
and \fBTARGET\fP each represent a string of characters not
including space or tab which respectively form the name of a macro,
prerequisite or target.
The name may itself be a macro expansion expression.
A \fBLINE\fP can be continued over several physical lines by terminating it with
a single backslash character.  Comments are initiated by the
pound \fB#\fR character and extend to the end of line.
All comment text is discarded, a '#' may be placed into the makefile text
by escaping it with '\e' (ie. \e# translates to # when it is parsed).
An exception to this occurs when a # is seen inside
a recipe line that begins with a <tab> or is inside a group recipe.
If you specify the \fB\-c\fP command line switch then this behavior is
disabled and
.B dmake
will treat all # characters as start of comment indicators unless they
are escaped by \e.
A set of continued lines may be commented out by placing a single # at the
start of the first line.
A continued line cannot span more than one makefile.
.PP
\fBwhite space\fP is defined to be any combination of
<space>, <tab>, and the sequence \e<nl>
when \e<nl> is used to terminate a LINE.
When processing \fBmacro\fP definition lines,
any amount of white space is allowed on either side of the macro operator
and white space is stripped from both before and after the macro
value string.
The sequence \e<nl> is treated as white space during recipe expansion
and is deleted from the final recipe string.
You must escape the \e<nl> with another \e in order to get a \e at the end
of a recipe line.
The \e<nl> sequence is deleted from macro values when they are expanded.
.PP
When processing \fBtarget\fP definition lines,
the recipe for a target must, in general, follow the first definition
of the target (See the RULES AND TARGETS section for an exception), and
the recipe may not span across multiple makefiles.
Any targets and prerequisites found on a target definition line are taken
to be white space separated tokens.
The rule operator (\fIop\fP in SYNTAX section) is also considered
to be a token but does not require
white space to precede or follow it.  Since the rule operator begins with a `:',
traditional versions of make do not allow the `:' character to
form a valid target name.  \fBdmake\fP allows `:' to be present in
target/prerequisite names as long as the entire target/prerequisite name is
quoted.  For example:
.sp
\ta:fred : test
.sp
would be parsed as TARGET = a, PREREQUISITES={fred, :, test}, which
is not what was intended.  To fix this you must write:
.sp
\t"a:fred" : test
.sp
Which will be parsed as expected.  Quoted target and prerequisite
specifications may also contain white space thereby allowing the use of
complex function macro expressions..
See the EXAMPLES section for how to apply \fB"\fP quoting
to a list of targets.
.SH ATTRIBUTES
.B dmake
defines several target attributes.  Attributes may be
assigned to a single target, a group of targets, or to all targets in the
makefile.  Attributes are used to modify
\fBdmake\fP actions during target update.
The recognized attributes are:
.sp
.IP \fB.EPILOG\fP 1.2i
Insert shell epilog code when executing a group recipe associated with
any target having this attribute set.
.IP \fB.ERRREMOVE\fP 1.2i
Always remove any target having this attribute if an error is encountered
while making them.  Setting this attribute overrides the .PRECIOUS attribute.
.IP \fB.EXECUTE\fP 1.2i
If the \-n flag was given then execute the recipe associated with any
target having this attribute set.
.IP \fB.FIRST\fP 1.2i
Used in conjunction with .INCLUDE.  Terminates the inclusion with the first
successfully included prerequisite.
.IP \fB.GROUP\fP 1.2i
Force execution of a target's recipe as a group recipe.
.IP \fB.IGNORE\fP 1.2i
Ignore an error when trying to make any target with this attribute set.
.IP \fB.IGNOREGROUP\fP 1.2i
Disable the special meaning of '[' to initiate a group recipe.
.IP \fB.LIBRARY\fP 1.2i
Target is a library.
.IP \fB.MKSARGS\fP 1.2i
If running in an MSDOS environment then use MKS extended argument passing
conventions to pass arguments to commands.  Non-MSDOS
environments ignore this attribute.
.IP \fB.NOINFER\fP 1.2i
Any target with this attribute set will not be subjected
to transitive closure if it is inferred as a prerequisite 
of a target whose recipe and prerequisites are being inferred.
(i.e. the inference algorithm will not use any prerequisite with this attribute 
set, as a target)
If specified as '.NOINFER:' (ie. with no prerequisites or targets) then the
effect is equivalent to specifying \fB\-T\fP on the command line.
.IP \fB.NOSTATE\fP 1.2i
Any target with this attribute set will not have command line flag
information stored in the state file if .KEEP_STATE has been enabled.
.IP \fB.PHONY\fP 1.2i
Any target with this attribute set will have its recipe executed
each time the target is made even if a file matching the target name can
be located.  Any targets that have a .PHONY attributed target as a
prerequisite will be made each time the .PHONY attributed prerequisite is
made.
.IP \fB.PRECIOUS\fP 1.2i
Do not remove associated target under any circumstances.
Set by default for any targets whose corresponding files exist in the file
system prior to the execution of \fBdmake\fP.
.IP \fB.PROLOG\fP 1.2i
Insert shell prolog code when executing a group recipe associated with
any target having this attribute set.
.IP \fB.SEQUENTIAL\fP 1.2i
Force a sequential make of the associated target's prerequisites.
.IP \fB.SETDIR\fP 1.2i
Change current working directory to specified directory when making the
associated target.  You must
specify the directory at the time the attribute is specified.  To do this
simply give \fI.SETDIR=path\fP as the attribute.  \fIpath\fP is expanded and
the result is used as the value of the directory to change to.
If \fIpath\fP contains \fB$$@\fP then the name of the target to be built is
used in computing the path to change directory to.
If path is surrounded by single quotes then path is not expanded, and is used
literally as the directory name.
If the \fIpath\fP contains any `:' characters then the entire attribute string
must be quoted using ".
If a target having this attribute set also has the .IGNORE
attribute set then if the change to the specified directory fails it will be
ignored, and no error message will be issued.
.IP \fB.SILENT\fP 1.2i
Do not echo the recipe lines when making any target with this attribute set,
and do not issue any warnings.
.IP \fB.SWAP\fP 1.2i
Under MSDOS
when making a target with this attribute set swap the \fBdmake\fP executable
to disk prior to executing the recipe line.  Also see the '%' recipe line
flag defined in the RECIPES section.
.IP \fB.SYMBOL\fP 1.2i
Target is a library member and is an entry point into a module in the
library.  This attribute is used only when searching a library for a target.
Targets of the form lib((entry)) have this attribute set automatically.
.IP \fB.USESHELL\fP 1.2i
Force each recipe line of a target to be executed using a shell.
Specifying this attribute is equivalent to specifying the '+' character at the
start of each line of a non-group recipe.
.IP \fB.UPDATEALL\fP 1.2i
Indicates that all the targets listed in this rule are updated by the
execution of the accompanying recipe.
A common example is the production of the
.I y.tab.c
and
.I y.tab.h
files by
.B yacc
when it is run on a grammar.  Specifying .UPDATEALL in such a rule
prevents the running of yacc twice, once for the y.tab.c file and once
for the y.tab.h file.  .UPDATEALL targets that are specified in a single rule
are treated as a single target and all timestamps are updated whenever any
target in the set is made.  As a side-effect, \fBdmake\fP internally sorts
such targets in ascending alphabetical order and the value of $@ is always
the first target in the sorted set.
.LP
All attributes are user setable and except for .UPDATEALL, .SETDIR and .MKSARGS
may be used in one of two forms.
The .MKSARGS attribute is restricted to use as a global attribute, and
the use of the .UPDATEALL and .SETDIR attributes is restricted to rules
of the second form only.
.sp
\tATTRIBUTE_LIST : \fItargets\fP
.sp
assigns the attributes specified by ATTRIBUTE_LIST to each target in
.I targets
or
.sp
\t\fItargets\fP ATTRIBUTE_LIST : ...
.sp
assigns the attributes specified by ATTRIBUTE_LIST to each target in
.I targets.
In the first form if
.I targets
is empty (ie. a NULL list), then the
list of attributes will apply to all targets in the makefile
(this is equivalent to the common Make construct of \fI".IGNORE :"\fP
but has been modified to the notion of an attribute instead of
a special target).
Not all of the attributes have global meaning.
In particular, .LIBRARY, .NOSTATE, .PHONY, .SETDIR, .SYMBOL and .UPDATEALL
have no assigned global meaning.
.PP
Any attribute may be used with any target, even with the special targets.
Some combinations are useless (e.g. .INCLUDE .PRECIOUS: ... ),
while others are useful (e.g. .INCLUDE .IGNORE : "file.mk" will not complain
if file.mk cannot be found using the include file search rules,
see the section on SPECIAL TARGETS for a description of .INCLUDE).
If a specified attribute will not be used with the special target a warning
is issued and the attribute is ignored.
.SH MACROS
.B dmake
supports six forms of macro assignment.
.sp
.IP "\fBMACRO = LINE\fP" 1.55i
This is the most common and familiar form of macro assignment.  It assigns
LINE literally as the value of MACRO.
Future expansions of MACRO recursively expand its value.
.IP "\fBMACRO *= LINE\fP" 1.55i
This form behaves exactly as the simple '=' form with the exception that if
MACRO already has a value then the assignment is not performed.
.IP "\fBMACRO := LINE\fP" 1.55i
This form differs from the simple '=' form in that it expands LINE
prior to assigning it as the value of MACRO.
Future expansions of MACRO do not recursively expand its value.
.IP "\fBMACRO *:= LINE\fP" 1.55i
This form behaves exactly as the ':=' form with the exception that if
MACRO already has a value then the assignment and expansion are not performed.
.IP "\fBMACRO += LINE\fP" 1.55i
This form of macro assignment allows macro values to grow.  It takes the
literal value of LINE and appends it to the previous value of MACRO separating
the two by a single space.
Future expansions of MACRO recursively expand its value.
.IP "\fBMACRO +:= LINE\fP" 1.55i
This form is similar to the '+=' form except that the value of LINE is expanded
prior to being added to the value of MACRO.
.PP
Macro expressions specified on the command line allow the macro value
to be redefined within the makefile only if the macro is defined using
the '+=' and '+:=' operators.  Other operators will define a macro that cannot
be further modified.
.PP
Each of the preceeding macro assignment operators may be prefixed by \fB!\fP
to indicate that the assignment should be forced and that no warnings should
be issued.  Thus, specifying \fB!\fP has the effect of silently forcing the
specified macro assignment.
.PP
When \fBdmake\fP defines a non-environment macro it strips leading and
trailing white space from the macro value.
Macros imported from the environment via either the .IMPORT special
target (see the SPECIAL TARGETS section), or the \fB\-e\fP, or \fB\-E\fP flags
are an exception to this rule.  Their values are
always taken literally and white space is never stripped.
In addition, named macros defined using the .IMPORT special target do
not have their values expanded when they are used within a makefile.
In contrast, environment macros that are imported
due to the specification of the \fB\-e\fP or \fB\-E\fP flags
are subject to expansion when used.
.PP
To specify a macro expansion
enclose the name in () or {} and precede it with a dollar sign $.
Thus $(TEST) represents an expansion of the macro variable named TEST.
If TEST is
defined then $(TEST) is replaced by its expanded value.  If TEST is not
defined then $(TEST) expands to the NULL string (this is equivalent to
defining a macro as 'TEST=' ).  A short form may be used for single character
named macros.  In this case the parentheses are optional, and $(I) is
equivalent to $I.
Macro expansion is recursive, hence, if the value string contains an expression
representing a macro expansion, the expansion is performed.  Circular macro
expansions are detected and cause an error to be issued.
.PP
When defining a macro the given macro name is first expanded before being used
to define the macro.  Thus it is possible to define macros whose names
depend on values of other macros.  For example, suppose CWD is defined as
.sp
\tCWD = $(PWD:b)
.sp
then the value of $(CWD) is the name of the current directory.
This can be used to define macros specific to this directory, for
example:
.sp
\t_$(CWD).prt = list of files to print...
.sp
The actual name of the defined macro is a function of the current directory.
A construct such as this is useful when processing a hierarchy of directories
using .SETDIR attributed targets and a collection of small distributed
makefile stubs.
.PP
Macro variables may be defined within the makefile, on the command
line, or imported from the environment.
.PP
.B \fBdmake\fR
supports several non-standard macro expansions:
The first is of the form:
.RS
.IP \fI$(macro_name:modifier_list:modifier_list:...)\fR
.RE
.LP
where
.I modifier_list
is chosen from the set { B or b, D or d, E or e, F or f, I or i, L or l, S or
s, T or t, U or u, ^, +, 1 } and
.RS
.sp
.Is "b  "
.Ii "b "
\- file (not including suffix) portion of path names
.Ii "d"
\- directory portion of all path names
.Ii "e"
\- suffix portion of path names
.Ii "f"
\- file (including suffix) portion of path names
.Ii "i"
\- inferred names of targets
.Ii "l"
\- macro value in lower case
.Ii "s"
\- simple pattern substitution
.Ii "t"
\- tokenization.
.Ii "u"
\- macro value in upper case
.Ii "^"
\- prepend a prefix to each token
.Ii "+"
\- append a suffix to each token
.Ii "1"
\- return the first white space separated token from value
.sp
.RE
.fi
Thus if we have the example:
.RS
test = d1/d2/d3/a.out f.out d1/k.out
.RE
The following macro expansions produce the values on the right of '\(->' after
expansion.
.RS
.sp
.Is "$(test:s/out/in/:f)   "
.Ii "$(test:d)"
\(-> d1/d2/d3/ d1/
.Ii "$(test:b)"
\(-> a f k
.Ii "$(test:f)"
\(-> a.out f.out k.out
.Ii "${test:db}"
\(-> d1/d2/d3/a f d1/k
.Ii "${test:s/out/in/:f}"
\(-> a.in f.in k.in
.Ii $(test:f:t"+")
\(-> a.out+f.out+k.out
.Ii $(test:e)
\(-> .out .out .out
.Ii $(test:u)
\(-> D1/D2/D3/A.OUT F.OUT D1/K.OUT
.Ii $(test:1)
\(-> d1/d2/d3/a.out
.RE
.fi
.PP
If a token ends in a string composed from the value of the macro DIRBRKSTR
(ie. ends in a directory separator string, e.g. '/' in UNIX) and you use the
\fB:d\fP modifier then the expansion returns the directory name less the
final directory separator string.  Thus successive pairs of :d modifiers
each remove a level of directory in the token string.
.PP
The tokenization modifier takes all white space separated tokens from the
macro value and separates them by the quoted separator string.  The separator
string may contain the following escape codes \ea => <bel>,
\&\eb => <backspace>, \ef => <formfeed>, \en => <nl>, \er => <cr>,
\&\et => <tab>, \ev => <vertical tab>, \e" => ", and \exxx => <xxx> where
xxx is the octal representation of a character.  Thus the
expansion:
.LP
.RS
.nf
$(test:f:t"+\en")
.RE
produces:
.RS
a.out+
f.out+
k.out
.fi
.RE
.PP
The prefix operator \fB^\fP takes all white space separated tokens from the
macro value and prepends \fIstring\fP to each.
.LP
.RS
.nf
$(test:f:^mydir/)
.RE
produces:
.RS
mydir/a.out mydir/f.out mydir/k.out
.fi
.RE
.PP
The suffix operator \fB+\fP takes all white space separated tokens from the
macro value and appends \fIstring\fP to each.
.LP
.RS
.nf
$(test:b:+.c)
.RE
produces:
.RS
a.c f.c k.c
.fi
.RE
.PP
The next non-standard form of macro expansion allows for recursive macros.
It is possible to specify a $(\fImacro_name\fR) or ${\fImacro_name\fR} expansion
where \fImacro_name\fR contains more $( ... ) or ${ ... } macro expansions
itself.
.PP
For example $(CC$(_HOST)$(_COMPILER)) will first expand CC$(_HOST)$(_COMPILER)
to get a result and use that result as the name of the macro to expand.
This is useful for writing a makefile for more than one target
environment.  As an example consider the following hypothetical case. 
Suppose that _HOST and _COMPILER are imported from the environment
and are set to represent the host machine type and the host compiler
respectively.
.RS
.sp
.nf
CFLAGS_VAX_CC = \-c \-O  # _HOST == "_VAX", _COMPILER == "_CC"
CFLAGS_PC_MSC = \-c \-ML # _HOST == "_PC",  _COMPILER == "_MSC"
.sp
# redefine CFLAGS macro as:
.sp
CFLAGS := $(CFLAGS$(_HOST)$(_COMPILER))
.fi
.sp
.RE
This causes CFLAGS to take on a value that corresponds to the
environment in which the make is being invoked.
.PP
The final non-standard macro expansion is of the form:
.RS
.sp
string1{token_list}string2
.RE
.LP
where string1, string2 and token_list are expanded.  After expansion,
string1 is prepended to each token found in token_list and
string2 is appended to each resulting token from the previous prepend.
string1 and string2 are not delimited by white space
whereas the tokens in token_list are.
A null token in the token list 
is specified using "".
Thus using another example we have:
.RS
.sp
.Is "test/{f1  f2}.o    "
.Ii "test/{f1 f2}.o"
--> test/f1.o test/f2.o
.Ii "test/ {f1 f2}.o"
--> test/ f1.o f2.o
.Ii "test/{f1 f2} .o"
--> test/f1 test/f2 .o
.Ii "test/{""f1""  """"}.o"
--> test/f1.o test/.o
.sp
.Ii and
.sp
.Is "test/{d1 d2}/{f1 f2}.o --> "
.Ii "test/{d1 d2}/{f1 f2}.o --> "
test/d1/f1.o test/d1/f2.o
test/d2/f1.o test/d2/f2.o
.sp
.RE
.fi
This last expansion is activated only when the first characters of
\fItoken_list\fP
appear immediately after the opening '{' with no intervening white space.
The reason for this restriction is the following incompatibility with
Bourne Shell recipes.  The line
.RS
.sp
{ echo hello;}
.sp
.RE
is valid /bin/sh syntax; while
.RS
.sp
{echo hello;}
.sp
.RE
.fi
is not. Hence the latter triggers the enhanced macro expansion while the former
causes it to be suppressed.
See the SPECIAL MACROS section for a description of the special macros that
\fBdmake\fP defines and understands.
.SH "RULES AND TARGETS"
A makefile contains a series of entries that specify dependencies.
Such entries are called \fItarget/prerequisite\fP or \fIrule\fP definitions.
Each rule definition
is optionally followed by a set of lines that provide a recipe for updating
any targets defined by the rule.
Whenever
.B dmake
attempts to bring a target up to date and an explicit recipe is provided with
a rule defining the target, that recipe is used to update the
target.  A rule definition begins with a line having the following syntax:
.sp
.RS
.nf
\fI<targets>\fP [\fI<attributes>\fP] \fI<ruleop>\fP [\fI<prerequisites>\fP] [;\fI<recipe>\fP]
.fi
.RE
.sp
.I targets
is a non-empty list of targets.  If the target is a
special target (see SPECIAL TARGETS section below) then it must appear alone
on the rule line.  For example:
.sp
.RS
\&.IMPORT .ERROR : ...
.RE
.sp
is not allowed since both .IMPORT and .ERROR are special targets.
Special targets are not used in the construction of the dependency graph and
will not be made.
.PP
.I attributes
is a possibly empty list of attributes.  Any attribute defined in the
ATTRIBUTES section above may be specified.  All attributes will be applied to
the list of named targets in the rule definition.  No other targets will
be affected.
.sp
.IP NOTE: 0.75i
As stated earlier,
if both the target list and prerequisite list are empty but the attributes
list is not, then the specified attributes affect all targets in the makefile.
.sp
.PP
.I ruleop
is a separator which is used to identify the targets from the prerequisites.
Optionally it also provides a facility for modifying the way in which
.B dmake
handles the making of the associated targets.
In its simplest form the operator is a single ':', and need not be separated
by white space from its neighboring tokens.  It may additionally be followed
by any of the modifiers { !, ^, \-, :, | }, where:
.sp
.IP \fB!\fP
says execute the recipe for the associated targets once for each out of date
prerequisite.  Ordinarily the recipe is executed
once for all out of date prerequisites at the same time.
.IP \fB^\fP
says to insert the specified prerequisites, if any, before any
other prerequisites already associated with the specified targets.
In general, it is not useful to specify ^ with an empty
list of prerequisites.
.IP \fB\-\fP
says to clear the previous list of prerequisites before adding
the new prerequisites.  Thus,
.sp
\t.SUFFIXES :
.br
\t.SUFFIXES : .a .b
.sp
can be replaced by
.sp
\t.SUFFIXES :\- .a .b
.sp
however the old form still works as expected.  NOTE:  .SUFFIXES is ignored by
.B dmake
it is used here simply as an example.
.IP \fB:\fP
When the rule operator is not modified by a second ':'
only one set of rules may be specified for making a target.
Multiple definitions may be used to add to the
list of prerequisites that a target depends on.
However, if a target is multiply defined
only one definition may specify a recipe
for making the target.
.sp
When a target's rule operator is modified by a second ':'
(:: for example) then this definition may not be the only
definition with a recipe for the target.  There may be other :: target
definition lines that specify a different set of prerequisites with a
different recipe for updating the target.  
Any such target is made if any of the definitions
find it to be out of date
with respect to the related prerequisites
and the corresponding recipe is used to update the
target.  By definition all '::' recipes that are found to be out of date for
are executed.
.sp 
In the following simple example, each rule has a `::' \fIruleop\fP.  In such an
operator we call the first `:' the operator, and the second `:' the modifier.
.sp
.nf
a.o :: a.c b.h
   first recipe for making a.o

a.o :: a.y b.h
   second recipe for making a.o
.fi
.sp
If a.o is found to be out of date with respect to a.c then the first recipe
is used to make a.o.  If it is found out of date with respect to a.y then
the second recipe is used.  If a.o is out of date with respect to
b.h then both recipes are invoked to make a.o.
In the last case the order of invocation corresponds to the order in which the
rule definitions appear in the makefile.
.IP \fB|\fP
Is defined only for PERCENT rule target definitions.  When specified it
indicates that the following construct should be parsed using the old
semantinc meaning:
.sp
.nf
%.o :| %.c %.r %.f ; some rule
.sp
is equivalent to:
.sp
%.o : %.c ; some rule
%.o : %.r ; some rule
%.o : %.f ; some rule
.fi
.PP
Targets defined using a single `:' operator
with a recipe may be redefined again with a new recipe by using a
`:' operator with a `:' modifier.
This is equivalent to a target having been
initially defined with a rule using a `:' modifier.
Once a target is defined using a `:'
modifier it may not be defined again with a recipe using only the `:' operator
with no `:' modifier.  In both cases the use of a `:' modifier creates a new
list of prerequisites and makes it the current prerequisite list for the target.
The `:' operator with no recipe always modifies the current list
of prerequisites.
Thus assuming each of the following definitions has a recipe attached, then:
.RS
.sp
.nf
joe :  fred ...	(1)
joe :: more ...	(2)
.sp
and
.sp
joe :: fred ...	(3)
joe :: more ...	(4)
.sp
.fi
.RE
are legal and mean:  add the recipe associated with (2), or (4) to the set
of recipes for joe, placing them after existing recipes for
making joe.
The constructs:
.RS
.sp
.nf
joe :: fred ...	(5)
joe : more ...	(6)
.sp
and
.sp
joe : fred ...	(7)
joe : more ...	(8)
.sp
.fi
.RE
are errors since we have two sets of perfectly good recipes for
making the target.
.PP
.I prerequisites
is a possibly empty list of targets that must be brought up to date before
making the current target.
.PP
.I recipe
is a short form and allows the user to specify short rule definitions
on a single line.
It is taken to be the first recipe line in a larger recipe
if additional lines follow the rule definition.
If the semi-colon is present but the recipe line is empty (ie. null string)
then it is taken
to be an empty rule.  Any target so defined causes the
.I "Don't know how to make ..."
error message to be suppressed when
.B dmake
tries to make the target and fails.
This silence is maintained for rules that are terminated
by a semicolon and have no following recipe lines, for targets listed on the
command line, for the first target found in the makefile, and for any target
having no recipe but containing a list of prerequisites (see the COMPATIBILITY
section for an exception to this rule if the AUGMAKE (\fB\-A\fP) flag
was specified.
.SH "RECIPES"
The traditional format used by most versions of Make defines the recipe
lines as arbitrary strings that may contain macro expansions.  They
follow a rule definition line and may be spaced
apart by comment or blank lines.
The list of recipe lines defining the recipe is terminated by a new target
definition, a macro definition, or end-of-file.
Each recipe line
.B MUST
begin with a \fB<TAB>\fP character which
may optionally be followed with one or all
of the characters
.IR "'@%+\-'" "."
The
.I "'\-'"
indicates that non-zero exit values (ie. errors)
are to be ignored when this recipe line is executed, the
.I "'\+'"
indicates that the current recipe line is to be executed using the shell, the
.I "'%'"
indicates that
.B dmake
should swap itself out to secondary storage (MSDOS only) before running the
recipe and the
.I "'@'"
indicates that the recipe line should NOT be echoed to the terminal prior to
being executed.  Each switch is off by default
(ie. by default, errors are significant, commands are echoed, no swapping is
done and a shell is
used only if the recipe line contains a character found in the value of the
SHELLMETAS macro).
Global settings activated via command line options or special attribute or
target names may also affect these settings.
An example recipe:
.sp
.RS
.nf
target :
\tfirst recipe line
\tsecond recipe line, executed independent of first.
\t@a recipe line that is not echoed
\t\-and one that has errors ignored
\t%and one that causes dmake to swap out
\t\+and one that is executed using a shell.
.fi
.RE
.PP
The second and new format of the recipe block begins the block with the
character '[' (the open group character) in the last non-white space
position of a line, and terminates the
block with the character ']' (the close group character)
in the first non-white space position of a line.
In this form each recipe line need not have a leading TAB.  This is
called a recipe group.  Groups so defined are fed intact as a single
unit to a shell for execution whenever the corresponding target needs to
be updated.  If the open group character '[' is preceded
by one or all of \-, @ or %
then they apply to the entire group in the same way that they
apply to single recipe lines.  You may also specify '+' but it is
redundant as a shell is already being used to run the recipe.
See the MAKING TARGETS section for a description of how
.B dmake
invokes recipes.
Here is an example of a group recipe:
.sp
.RS
.nf
target :
[
   first recipe line
   second recipe line
   tall of these recipe lines are fed to a
   single copy of a shell for execution.
]
.fi
.RE
.sp
.SH "TEXT DIVERSIONS"
.B dmake
supports the notion of text diversions.
If a recipe line contains the macro expression
.RS
.sp
$(mktmp[,[\fIfile\fP][,\fItext\fP]] \fIdata\fP)
.sp
.RE
then all text contained in the \fIdata\fP expression is expanded and
is written to a temporary file.  The return
value of the macro is the name of the temporary file.
.PP
.I data
can be any text and must be separated from the 'mktmp' portion of the
macro name by white-space.  The only restriction on the data text is that
it must contain a balanced number of parentheses of the same kind as are
used to initiate the $(mktmp ...) expression.  For example:
.RS
.sp
$(mktmp $(XXX))
.sp
.RE
is legal and works as expected, but:
.RS
.sp
$(mktmp text (to dump to file)
.sp
.RE
is not legal.  You can achieve what you wish by either defining a macro that
expands to '(' or by using {} in the macro expression; like this:
.RS
.sp
${mktmp text (to dump to file}
.sp
.RE
Since the temporary file is opened when the
macro containing the text diversion expression is expanded, diversions may
be nested and any diversions that are created as part of ':=' macro
expansions persist for the duration of the
.B dmake
run.
The diversion text may contain
the same escape codes as those described in the MACROS section.
Thus if the \fIdata\fP text is to contain new lines they must be inserted
using the \en escape sequence.  For example the expression:
.RS
.sp
.nf
all:
	cat $(mktmp this is a\en\e
	test of the text diversion\en)
.fi
.sp
.RE
is replaced by:
.RS
.sp
cat /tmp/mk12294AA
.sp
.RE
where the temporary file contains two lines both of which are terminated
by a new-line.  If the \fIdata\fP text spans multiple lines in the makefile
then each line must be continued via the use of a \e.
A second more illustrative example generates a response file to an MSDOS
link command:
.RS
.sp
.nf
OBJ = fred.obj mary.obj joe.obj
all : $(OBJ)
	link @$(mktmp $(^:t"+\en")\en)
.fi
.sp
.RE
The result of making `all' in the second example is the command:
.RS
.sp
link @/tmp/mk02394AA
.sp
.RE
where the temporary file contains:
.RS
.sp
.nf
fred.obj+
mary.obj+
joe.obj
.fi
.sp
.RE
The last line of the file is terminated by a new-line which is inserted
due to the \en found at the end of the \fIdata\fP string.
.PP
If the optional \fIfile\fP specifier is present then its expanded value
is the name of the temporary file to create.  Whenever a $(mktmp ...) macro
is expanded the macro $(TMPFILE) is set to a new temporary file name.  Thus
the construct:
.RS
.sp
$(mktmp,$(TMPFILE) data)
.sp
.RE
is completely equivalent to not specifying the $(TMPFILE) optional argument.
Another example that would be useful for MSDOS users with a Turbo-C compiler
.RS
.sp
$(mktmp,turboc.cfg $(CFLAGS))
.sp
.RE
will place the contents of CFLAGS into a local \fIturboc.cfg\fP file.
The second optional argument, \fItext\fP, if present alters the name
of the value returned by the $(mktmp ...) macro.
.PP
Under MS-DOS text diversions may be a problem.  Many DOS tools require
that path names which contain directories use the \e character to delimit
the directories.  Some users however wish to use the '/' to delimit pathnames
and use environments that allow them to do so.
The macro USESHELL is set to "yes" if the
current recipe is forced to use a shell via the .USESHELL or '+' directives,
otherwise its value is "no".
The
.B dmake
startup files define the macro DIVFILE whose value is either the
value of TMPFILE or the value of TMPFILE edited to replace any '/' characters
to the appropriate value based on the current shell and whether it will be
used to execute the recipe.
.PP
Previous versions of
.B dmake
defined text diversions using <+, +> strings,
where <+ started a text diversion and +> terminated one.
.B dmake
is backward compatible with this construct only
if the <+ and +> appear literally
on the same recipe line or in the same macro value string.  In such instances
the expression:
.sp
\t<+data+>
.sp
is mapped to:
.sp
\t$(mktmp data)
.sp
which is fully output compatible with the earlier construct.  <+, +>
constructs whose text spans multiple lines must be converted by hand to use
$(mktmp ...).
.PP
If the environment variable TMPDIR is defined then the
temporary file is placed into the directory specified by that variable.
A makefile can modify the location of temporary files by
defining a macro named TMPDIR and exporting it using the .EXPORT special
target.
.SH "SPECIAL TARGETS"
This section describes the special targets that are recognized by \fBdmake\fP.
Some are affected by attributes and others are not.
.IP \fB.ERROR\fP 1.4i
If defined then the recipe associated with this target is executed
whenever an error condition is detected by \fBdmake\fP.  All attributes that
can be used with any other target may be used with this target.  Any
prerequisites of this target will be brought up to date during its processing.
NOTE:  errors will be ignored while making this target, in extreme cases this
may cause some problems.
.IP \fB.EXIT\fP 1.4i
If this target is encountered while parsing a makefile then the parsing of the
makefile is immediately terminated at that point.
.IP \fB.EXPORT\fP 1.4i
All prerequisites associated with this target are assumed to
correspond to macro names and they and their values
are exported to the environment as environment strings at the point in
the makefile at which this target appears.
Any attributes specified with this target are ignored.
Only macros which have been assigned a value in the makefile prior to the
export directive are exported, macros as yet undefined
or macros whose value contains any of the characters "+=:*"
are not exported.
is suppre
.IP \fB.IMPORT\fP 1.4i
Prerequisite names specified for this target are searched for in the
environment and defined as macros with their value taken from the environment.
If the special name \fB.EVERYTHING\fP is used as a prerequisite name then
all environment variables defined in the environment are imported.
The functionality of the \fB\-E\fP flag can be forced by placing the construct
\&\fI.IMPORT : .EVERYTHING\fP at the start of a makefile.  Similarly, by
placing the construct at the end, one can emulate the effect of the \fB\-e\fP
command line flag.
If a prerequisite name cannot be found in the environment
an error message is issued.
\&.IMPORT accepts the .IGNORE attribute.  When given, it causes \fBdmake\fP
to ignore the above error.
See the MACROS section for a description of the processing of imported macro
values.
.IP \fB.INCLUDE\fP 1.4i
Parse another makefile just as if it had been located at the point of the
\&.INCLUDE in the current makefile.
The list of prerequisites gives the list of
makefiles to try to read.  If the list contains multiple makefiles then they
are read in order from left to right.  The following search rules are used
when trying to locate the file.  If the filename is surrounded by " or just
by itself then it is searched for in the current directory.  If it is not
found it is then searched for in each of the directories specified
as prerequisites of the \&.INCLUDEDIRS special target.
If the file name is surrounded by < and >, (ie.
<my_spiffy_new_makefile>) then it is searched for only in the directories
given by the .INCLUDEDIRS special target.  In both cases if the file name is a
fully qualified name starting at the root of the file system then it is only
searched for once, and the .INCLUDEDIRS list is ignored.
If .INCLUDE fails to find the file it invokes the inference engine to
try to infer and hence make the file to be included.  In this way the
file can be checked out of an RCS repository for example.
\&.INCLUDE accepts
the .IGNORE, .SETDIR, and .NOINFER attributes.
If the .IGNORE attribute is given and the file
cannot be found then \fBdmake\fP continues processing,
otherwise an error message is generated.
If the .NOINFER attribute is given and the file
cannot be found then \fBdmake\fP will not attempt to
\fIinfer and make\fP the file.
The .SETDIR attribute causes
.B dmake
to change directories to the specified directory prior to attempting the
include operation.  If all fails \fBdmake\fP attempts to \fImake\fP the file
to be included.  If making the file fails then \fBdmake\fP terminates unless
the .INCLUDE directive also specified the .IGNORE attribute.
If .FIRST is specified along with .INCLUDE then \fBdmake\fP attempts to
include each named prerequisite and will terminate the inclusion with the
first prerequisite that results in a successful inclusion.
.IP \fB.INCLUDEDIRS\fP 1.4i
The list of prerequisites specified for this target defines the set of
directories to search when trying to include a makefile.
.IP \fB.KEEP_STATE\fP 1.4i
This special target is a synonym for the macro definition
.sp
\&\t.KEEP_STATE := _state.mk
.sp
It's effect is to turn on STATE keeping and to define \fI_state.mk\fP
as the state file.
.IP \fB.MAKEFILES\fP 1.4i
The list of prerequisites is the set of files to try to read as the default
makefile.  By default this target is defined as: 
.sp
\t\&.MAKEFILES : makefile.mk Makefile makefile
.sp
.IP \fB.SOURCE\fP 1.4i
The prerequisite list of this target defines a set of directories to check
when trying to locate a target file name.  See the section on BINDING of
targets for more information.
.IP \fB.SOURCE.suff\fP 1.4i
The same as .SOURCE, except that the .SOURCE.suff list is searched first when
trying to locate a file matching the a target whose name ends in the suffix
\&.suff.
.IP \fB.REMOVE\fP 1.4i
The recipe of this target is used whenever \fBdmake\fP needs to remove
intermediate targets that were made but do not need to be kept around.
Such targets result from the application of transitive closure on the
dependency graph.
.PP
In addition to the special targets above,
several other forms of targets are recognized and are considered special,
their exact form and use is defined in the sections that follow.
.SH "SPECIAL MACROS"
.B dmake
defines a number of special macros.  They are divided into three classes:
control macros, run-time macros, and function macros.
The control macros are used by
.B dmake
to configure its actions, and are the preferred method of doing so.
In the case when a control macro has the same function as a special 
target or attribute they share the same name as the special target or
attribute.
The run-time macros are defined when
.B dmake
makes targets and may be used by the user inside recipes.
The function macros provide higher level functions dealing with macro
expansion and diversion file processing.
.SH "CONTROL MACROS"
To use the control macros simply assign them a value just like any other
macro.  The control macros are divided into three groups:
string valued macros, character valued macros, and boolean valued macros.
.PP
The following are all of the string valued macros.
This list is divided into two groups.  The first group gives the string
valued macros that are defined internally and cannot be directly set by the
user.
.IP \fBINCDEPTH\fP 1.6i
This macro's value is a string of digits representing
the current depth of makefile inclusion.
In the first makefile level this value is zero.
.IP \fBMFLAGS\fP 1.6i
Is the list of flags
that were given on the command line including a leading switch character.
The \-f flag is not included in this list.
.IP \fBMAKECMD\fP 1.6i
Is the name with which \fBdmake\fP was invoked.
.IP \fBMAKEDIR\fP 1.6i
Is the full path to the initial directory in which
.B dmake
was invoked.
.IP \fBMAKEFILE\fP 1.6i
Contains the string "\-f \fImakefile\fP" where, \fImakefile\fP is the name
of initial user makefile that was first read.
.IP \fBMAKEFLAGS\fP 1.6i
Is the same as $(MFLAGS) but has no leading switch
character. (ie. MFLAGS = \-$(MAKEFLAGS))
.IP \fBMAKEMACROS\fP 1.6i
Contains the complete list of macro expressions that were specified on the
command line.
.IP \fBMAKETARGETS\fP 1.6i
Contains the name(s) of the target(s), if any, that were
specified on the command line.
.IP \fBMAKEVERSION\fP 1.6i
Contains a string indicating the current \fBdmake\fP version number.
.IP \fBMAXPROCESSLIMIT\fP 1.6i
Is a numeric string representing the maximum number of processes that 
\fBdmake\fP can use when making targets using parallel mode.
.IP \fBNULL\fP 1.6i
Is permanently defined to be the NULL string.
This is useful when comparing a conditional expression to an NULL value.
.IP \fBPWD\fP 1.6i
Is the full path to the
current directory in which make is executing.
.IP \fBTMPFILE\fP 1.6i
Is set to the name of the most recent temporary file opened by \fBdmake\fP.
Temporary files are used for text diversions and for group recipe processing.
.IP \fBTMD\fP 1.6i
Stands for "To Make Dir", and
is the path from the present directory (value of $(PWD)) to the directory
that \fBdmake\fP was started up in (value of $(MAKEDIR)).
This macro is modified when .SETDIR attributes are processed.
.IP \fBUSESHELL\fP 1.6i
The value of this macro is set to "yes" if the current recipe is forced to
use a shell for its execution via the .USESHELL or '+' directives, its value
is "no" otherwise.
.sp
.PP
The second group of string valued macros control
.B dmake
behavior and may be set by the user.
.IP \fB.DIRCACHE\fP 1.6i
If set to "yes" enables the directory cache (this is the default).  If set to
"no" disables the directory cache (equivalent to -d command-line flag).
.IP \fB.DIRCACHERESPCASE\fP 1.6i
If set to "yes" causes the directory cache, if enabled, to respect
file case, if set to "no" facilities of the native OS are used to
match file case.
.IP \fB.NAMEMAX\fP 1.6i
Defines the maximum length of a filename component.  The value of the variable
is initialized at startup to the value of the compiled macro NAME_MAX.  On
some systems the value of NAME_MAX is too short by default.  Setting a new
value for .NAMEMAX will override the compiled value.
.IP \fB.NOTABS\fP 1.6i
When set to "yes" enables the use of spaces as well as <tabs> to begin
recipe lines.
By default a non\-group recipe is terminated by a line without any leading
white\-space or by a line not beggining with a <tab> character.
Enabling this mode modifies the first condition of
the above termination rule to terminate a
non\-group recipe with a line that contains only white\-space.
This mode does not effect the parsing of group recipes bracketed by [].
.IP \fBAUGMAKE\fP 1.6i
If set to "yes" value will enable the transformation of special
meta targets to support special AUGMAKE inferences (See the COMPATIBILITY
section).
.IP \fBDIRBRKSTR\fP 1.6i
Contains the string of chars used to terminate
the name of a directory in a pathname.
Under UNIX its value is "/", under MSDOS its value is "/\e:".
.IP \fBDIRSEPSTR\fP 1.6i
Contains the string that is used to separate directory components when
path names are constructed.  It is defined with a default value at startup.
.IP \fBDIVFILE\fP 1.6i
Is defined in the startup file and gives the name that should be returned for
the diversion file name when used in
$(mktmp ...) expansions, see the TEXT DIVERSION section for details.
.IP \fBDYNAMICNESTINGLEVEL\fP 1.6i
Specifies the maximum number of recursive dynamic macro expansions.  Its
initial value is 100.
.IP \fB.KEEP_STATE\fP 1.6i
Assigning this macro a value tells
.B dmake
the name of the state file to use and turns on the keeping of state
information for any targets that are brought up to date by the make.
.IP \fBGROUPFLAGS\fP 1.6i
This macro gives the set of flags to pass to the shell when
invoking it to execute a group recipe.  The value of the macro is the
list of flags with a leading switch indicator.  (ie. `\-' under UNIX)
.IP \fBGROUPSHELL\fP 1.6i
This macro defines the full
path to the executable image to be used as the shell when
processing group recipes.  This macro must be defined if group recipes are
used.  It is assigned a default value in the startup makefile.  Under UNIX
this value is /bin/sh.
.IP \fBGROUPSUFFIX\fP 1.6i
If defined, this macro gives the string to use as a suffix
when creating group recipe files to be handed to the command interpreter.
For example, if it is defined as .sh, then all
temporary files created by \fBdmake\fP will end in the suffix .sh.
Under MSDOS if you are using command.com as your GROUPSHELL, then this suffix
must be set to .bat in order for group recipes to function correctly.
The setting of GROUPSUFFIX and GROUPSHELL is done automatically for
command.com in the startup.mk files.
.IP \fBMAKE\fP 1.6i
Is defined in the startup file by default.
Initially this macro is defined to have the value "$(MAKECMD) $(MFLAGS)".
The string $(MAKE) is recognized when using the \-n switch.
.IP \fBMAKESTARTUP\fP 1.6i
This macro defines the full path to the initial startup
makefile.  Use the \fB\-V\fP command line option to discover its initial
value.
.IP \fBMAXLINELENGTH\fP 1.6i
This macro defines the maximum size of a single line of
makefile input text.  The size is specified as a number, the default value
is defined internally and is shown via the \fB\-V\fP option.
A buffer of this size plus 2 is allocated for reading makefile text.  The
buffer is freed before any targets are made, thereby allowing files containing
long input lines to be processed without consuming memory during the actual
make.
This macro can only be used to extend the line length beyond it's default
minimum value.
.IP \fBMAXPROCESS\fP 1.6i
Specify the maximum number of child processes to use when making targets.
The default value of this macro is "1" and its value cannot exceed the value
of the macro MAXPROCESSLIMIT.  Setting the value of MAXPROCESS on the command
line or in the makefile is equivalent to supplying a corresponding value to
the -P flag on the command line.
.IP \fBPREP\fP 1.6i
This macro defines the number of iterations to be expanded
automatically when processing % rule definitions of the form:
.sp
% : %.suff
.sp
See the sections on PERCENT(%) RULES for details on how PREP is used.
.IP \fBSHELL\fP 1.6i
This macro defines the full path to the executable
image to be used as the shell when
processing single line recipes.  This macro must be defined if recipes
requiring the shell for execution are to be used.
It is assigned a default value in the startup makefile.
Under UNIX this value is /bin/sh.
.IP \fBSHELLFLAGS\fP 1.6i
This macro gives the set of flags to pass to the shell when
invoking it to execute a single line recipe.  The value of the macro is the
list of flags with a leading switch indicator.  (ie. `\-' under UNIX)
.IP \fBSHELLMETAS\fP 1.6i
Each time
.B dmake
executes a single recipe line (not a group recipe) the line is
searched for any occurrence of a character defined in the value of SHELLMETAS.
If such a character is found the recipe line is defined to require a shell
to ensure its correct execution.  In such instances
a shell is used to invoke the recipe line.
If no match is found the recipe line is executed without the use of a shell.
.sp
.PP
There is only one character valued macro defined by \fBdmake\fP:
\fBSWITCHAR\fP contains the switch character used
to introduce options on command lines.  For UNIX its value is `\-', and for
MSDOS its value may be `/' or `\-'.
The macro is internally defined and is not user setable.
The MSDOS version of \fBdmake\fP attempts to first extract SWITCHAR from an
environment variable of the same name.  If that fails it then attempts to
use the undocumented getswitchar system call, and returns the result of
that.  Under MSDOS version 4.0 you must set the value of the environment
macro SWITCHAR to '/' to obtain predictable behavior.
.PP
All boolean macros currently understood by 
.B dmake
correspond directly to the previously defined attributes.
These macros provide
a second way to apply global attributes, and represent the
preferred method of doing so.  They are used by assigning them a
value.  If the value is not a NULL string then the boolean condition
is set to on.
If the value is a NULL string then the condition is set to off.
There are five conditions defined and they correspond directly to the
attributes of the same name.  Their meanings are defined in the ATTRIBUTES
section above.
The macros are:
\&\fB.EPILOG\fP,
\&\fB.IGNORE\fP,
\&\fB.MKSARGS\fP,
\&\fB.NOINFER\fP,
\&\fB.PRECIOUS\fP,
\&\fB.PROLOG\fP,
\&\fB.SEQUENTIAL\fP,
\&\fB.SILENT\fP,
\&\fB.SWAP\fP, and
\&\fB.USESHELL\fP.
Assigning any of these a non NULL value will globally set
the corresponding attribute to on.
.SH "RUNTIME MACROS"
These macros are defined
when \fBdmake\fP is making targets, and may take on different values for each
target.  \fB$@\fP is defined to be the full target name, \fB$?\fP is the
list of all out of date prerequisites, \fB$&\fP is the list of all
prerequisites, \fB$>\fP is the name of the library if the current target is a
library member, and
\fB$<\fP is the list of prerequisites specified in the current rule.
If the current target had a recipe inferred then \fB$<\fP is the name of the
inferred prerequisite even if the target had a list of prerequisites supplied
using an explicit rule that did not provide a recipe.  In such situations
\fB$&\fP gives the full list of prerequisites.
.PP
\fB$*\fP is defined as
\fB$(@:db)\fP when making targets with explicit recipes and is defined as the
value of % when making targets whose recipe is the result of an inference.
In the first case \fB$*\fP is the target name with no suffix,
and in the second case, is the value of the matched % pattern from
the associated %-rule.
\fB$^\fP expands to the set of out of date prerequisites taken from the
current value of \fB$<\fP.
In addition to these,
\fB$$\fP expands to $, \fB{{\fP expands to {, \fB}}\fP expands to }, and the
strings \fB<+\fP and \fB+>\fP are recognized
as respectively starting and terminating a text diversion when they appear
literally together in the same input line.
.PP
The difference between $? and $^ can best be illustrated by an example,
consider:
.RS
.sp
.nf
fred.out : joe amy hello
\trules for making fred

fred.out : my.c your.h his.h her.h	  # more prerequisites
.fi
.sp
.RE
Assume joe, amy, and my.c are newer then fred.out.  When
.B dmake
executes the recipe for making fred.out the values of the following macros
will be:
.RS
.sp
.nf
.Is "$@ "
.Ii "$@"
--> fred.out
.Ii "$*"
--> fred
.Ii "$?"
--> joe amy my.c  # note output of $? vs $^
.Ii "$^"
--> joe amy
.Ii "$<"
--> joe amy hello
.Ii "$&"
--> joe amy hello my.c your.h his.h her.h
.fi
.sp
.RE
.SH "FUNCTION MACROS"
.B dmake
supports a full set of functional macros.  One of these, the $(mktmp ...)
macro, is discussed in detail in the TEXT DIVERSION section and is not
covered here.
.RS
.sp
.IP "$(\fBand\fP \fBmacroterm ...\fP)"
expands each \fBmacroterm\fP in turn until there are no more or one of
them returns an empty string.  If all expand to non-empty strings the
macro returs the string "t" otherwise it returns an empty string.
.sp
.IP "$(\fBassign\fP \fBexpression\fP)"
Causes \fIexpression\fP to be parsed as a macro assignment expression and results
in the specified assignment being made.  An error is issued if the assignment
is not syntatically correct.  \fIexpression\fP may contain white space.  This is
in effect a dynamic macro assignment facility and may appear anywhere any
other macro may appear.  The result of the expanding a dynamic macro
assignment expression is the name of the macro that was assigned and $(NULL)
if the \fIexpression\fP is not a valid macro assignment expression.
Some examples are:
.RS
.sp
.nf
$(assign foo := fred)
$(assign $(ind_macro_name) +:= $(morejunk))
.fi
.RE
.IP "$(\fBecho\fP \fBlist\fP)"
Echo's the value of \fIlist\fP.  \fIlist\fP is not expanded.
.IP "$(\fBeq\fP,\fItext_a\fP,\fItext_b\fP \fBtrue\fP \fBfalse\fP)"
expands
.I text_a
and
.I text_b
and compares their results.  If equal it returns the result of the expansion
of the 
.B true
term, otherwise it returns the expansion of the
.B false
term.
.IP "$(\fB!eq\fP,\fItext_a\fP,\fItext_b\fP \fBtrue\fP \fBfalse\fP)"
Behaves identically to the previous macro except that the 
.B true
string is chosen if the expansions of the two strings are not equal
.IP "$(\fBforeach\fP,\fIvar\fP,\fIlist\fP \fBdata\fP)"
Implements iterative macro expansion over \fIdata\fP using \fBvar\fP as the
iterator taking on values from \fIlist\fP. \fIvar\fP and \fIlist\fP are
expanded and the result is the concatenation of expanding \fIdata\fP with
\fIvar\fP being set to each whitespace separated token from \fIlist\fP.
For example:
.RS
.RS
.sp
.nf
list = a b c
all :; echo [$(foreach,i,$(list) [$i])]
.fi
.sp
.RE
will output
.RS
.sp
.nf
[[a] [b] [c]]
.fi
.sp
.RE
The iterator variable is defined as a local variable to this foreach
instance.  The following expression illustrates this:
.RS
.sp
.nf
$(foreach,i,$(foreach,i,$(sort c a b) root/$i) [$i/f.h])
.fi
.sp
.RE
when evaluated the result is:
.RS
.sp
.nf
[root/a/f.h] [root/b/f.h] [root/c/f.h]
.fi
.sp
.RE
The specification of list must be a valid macro expression, such as:
.RS
.sp
.nf
$($(assign list=a b c))
$(sort d a b c)
$(echo a b c)
.fi
.sp
.RE
and cannot just be the list itself.  That is, the following foreach
expression:
.RS
.sp
.nf
$(foreach,i,a b c [$i])
.fi
.sp
.RE
yields:
.RS
.sp
.nf	
"b c [a]"
.fi
.sp
.RE
when evaluated.
.RE
.IP "$(\fBnil\fP \fBexpression\fP)"
Always returns the value of $(NULL) regardless of what \fIexpression\fP is.
This function macro can be used to discard results of expanding
macro expressions.
.IP "$(\fBnot\fP \fBmacroterm\fP)"
expands \fBmacroterm\fP and returs the string "t" if the result of the
expansion is the empty string; otherwise, it returns the empty string.
.IP "$(\fBnull\fP,\fItext\fP \fBtrue\fP \fBfalse\fP)"
expands the value of
.I text.
If it is NULL then the macro returns the value of the expansion of \fBtrue\fP
and the expansion of \fBfalse\fP otherwise.  The terms \fBtrue\fP, and
\fBfalse\fP must be strings containing no white\-space.
.IP "$(\fB!null\fP,\fItext\fP \fBtrue\fP \fBfalse\fP)"
Behaves identically to the previous macro except that the 
.B true
string is chosen if the expansion of
.I text
is not NULL.
.IP "$(\fBor\fP \fBmacroterm ...\fP)"
expands each \fBmacroterm\fP in turn and returs the empty string if
each term expands to the empty string; otherwise, it returs the string
"t".
.IP "$(\fBshell\fP \fBcommand\fP)"
Runs \fIcommand\fP as if it were part of a recipe and returns,
separated by a single space, all the non-white
space terms written to stdout by the command.
For example:
.RS
.RS
.sp
$(shell ls *.c)
.sp
.RE
will return \fI"a.c b.c c.c d.c"\fP if the files exist in the current
directory.  The recipe modification flags \fB[+@%\-]\fP are honored if they
appear as the first characters in the command.  For example:
.RS
.sp
$(shell +ls *.c)
.sp
.RE
will run the command using the current shell.
.RE
.IP "$(\fBshell,expand\fP \fBcommand\fP)"
Is an extension to the \fB$(shell...\fP function macro that expands the result
of running \fBcommand\fP.
.IP "$(\fBsort\fP \fBlist\fP)"
Will take all white\-space separated tokens in \fIlist\fP and will
return their sorted equivalent list.
.IP "$(\fBstrip\fP \fBdata\fP)"
Will replace all strings of white\-space in data by a single space.
.IP "$(\fBsubst\fP,\fIpat\fP,\fIreplacement\fP \fBdata\fP)"
Will search for \fIpat\fP in
.B data
and will replace any occurrence of
.I pat
with the
.I replacement
string.  The expansion
.RS
.sp
$(subst,.o,.c $(OBJECTS))
.sp
.RE
is equivalent to:
.RS
.sp
$(OBJECTS:s/.o/.c/)
.sp
.RE
.IP "$(\fBuniq\fP \fBlist\fP)"
Will take all white\-space separated tokens in \fIlist\fP and will
return their sorted equivalent list containing no duplicates.
.RE
.SH "CONDITIONAL MACROS"
.B dmake
supports conditional macros.  These allow the definition of target specific
macro values.  You can now say the following:
.RS
.sp
\fBtarget\fP ?= \fIMacroName MacroOp Value\fP
.sp
.RE
This creates a definition for \fIMacroName\fP whose value is \fIValue\fP
only when \fBtarget\fP is being made.  You may use a conditional macro
assignment anywhere that a regular macro assignment may appear, including
as the value of a $(assign ...) macro.
.LP
The new definition is associated with the most recent cell definition
for \fBtarget\fP.  If no prior definition exists then one is created.  The
implications of this are immediately evident in the following example:
.sp
.RS
.nf
foo := hello
.sp
all : cond;@echo "all done, foo=[$(foo)] bar=[$(bar)]"
.sp
cond ?= bar := global decl
.sp
cond .SETDIR=unix::;@echo $(foo) $(bar)
cond ?= foo := hi
.sp
cond .SETDIR=msdos::;@echo $(foo) $(bar)
	cond ?= foo := hihi
.fi
.RE
.sp
The first conditional assignment creates a binding for 'bar' that is
activated when 'cond' is made.  The bindings following the :: definitions are
activated when their respective recipe rules are used.  Thus the
first binding serves to provide a global value for 'bar' while any of the
cond :: rules are processed, and the local bindings for 'foo' come into
effect when their associated :: rule is processed.
.LP
Conditionals for targets of .UPDATEALL are all activated before the
target group is made.  Assignments are processed in order.  Note that
the value of a conditional macro assignment is NOT AVAILABLE until the
associated target is made, thus the construct
.sp
.RS
.nf
mytarget ?= bar := hello
mytarget ?= foo := $(bar)
.fi
.RE
.sp
results in $(foo) expanding to "", if you want the result to be "hello"
you must use:
.sp
.RS
.nf
mytarget ?= bar := hello
mytarget ?= foo  = $(bar)
.fi
.RE
.sp
Once a target is made any associated conditional macros are deactivated
and their values are no longer available.  Activation occurrs after all
inference, and .SETDIR directives have been processed and after $@ is
assigned, but before prerequisites are processed; thereby making the values of
conditional macro definitions available during construction of prerequisites.
.LP
If a %-meta rule target has associated conditional macro assignments,
and the rule is chosen by the inference algorithm then the conditional
macro assignments are inferred together with the associated recipe.
.SH "DYNAMIC PREREQUISITES"
.B dmake
looks for prerequisites whose names contain macro expansions during target
processing.  Any such prerequisites are expanded and the result of the
expansion is used as the prerequisite name.  As an example the line:
.sp
\tfred : $$@.c
.sp
causes the $$@ to be expanded when \fBdmake\fP is making fred, and it resolves
to the target \fIfred\fP.
This enables dynamic prerequisites to be generated.  The value
of @ may be modified by any of the valid macro modifiers.  So you can say for
example:
.sp
\tfred.out : $$(@:b).c
.sp
where the $$(@:b) expands to \fIfred\fP.
Note the use of $$ instead of $ to indicate the dynamic expansion, this
is due to the fact that the rule line is expanded when it is initially parsed,
and $$ then returns $ which later triggers the dynamic prerequisite expansion.
If you really want a $ to be part of a prerequisite name you must use $$$$.
Dynamic macro expansion is performed in all user defined rules,
and the special targets .SOURCE*, and .INCLUDEDIRS.
.PP
If dynamic macro expansion results in multiple white space separated tokens
then these are inserted into the prerequisite list inplace of the dynamic
prerequisite.  If the new list contains additional dynamic prerequisites they
will be expanded when they are processed.  The level of recursion in this
expansion is controlled by the value of the variable \fBDYNAMICNESTINGLEVEL\fP
and is set to 100 by default.
.SH "BINDING TARGETS"
This operation takes a target name and binds it to an existing file, if
possible.
.B dmake
makes a distinction between the internal target name of a target and its
associated external file name.
Thus it is possible for a target's internal name and its external
file name to differ.
To perform the binding, the following set of rules is used.
Assume that we are
trying to bind a target whose name is of the form \fIX.suff\fP,
where \fI.suff\fP is the suffix and \fIX\fP is the stem portion
(ie. that part which contains the directory and the basename).
.B dmake
takes this target name and performs a series of search operations that try to
find a suitably named file in the external file system.
The search operation is user controlled
via the settings of the various .SOURCE targets.
.RS
.IP 1.
If target has the .SYMBOL attribute set then look for it in the library.
If found, replace the target name with the library member name and continue
with step 2.  If the name is not found then return.
.IP 2.
Extract the suffix portion (that following the `.') of the target name.
If the suffix is not null, look up the special target .SOURCE.<suff>
(<suff> is the suffix).  
If the special target exists then search each directory given in
the .SOURCE.<suff> prerequisite list for the target.
If the target's suffix was null (ie. \fI.suff\fP was empty) then 
perform the above search but use the special target .SOURCE.NULL instead.
If at any point a match is found then terminate the search.
If a directory in the prerequisite list is the special name `.NULL ' perform
a search for the full target name without prepending any directory portion
(ie. prepend the NULL directory).
.IP 3.
The search in step 2. failed.  Repeat the same search but this time
use the special target .SOURCE.
(a default target of '.SOURCE : .NULL' is defined by \fBdmake\fP at startup,
and is user redefinable)
.IP 4.
The search in step 3. failed.
If the target has the library member attribute (.LIBMEMBER)
set then try to find the target in the library which was passed along
with the .LIBMEMBER attribute (see the MAKING LIBRARIES section).
The bound file name assigned to a target which is successfully
located in a library is the same name that would be assigned had the search
failed (see 5.).
.IP 5.
The search failed.  Either the target was not found in any of the search
directories or no applicable .SOURCE special targets exist.
If applicable .SOURCE special targets exist, but the target was not found,
then \fBdmake\fP assigns the first name searched as the bound file name.
If no applicable .SOURCE special targets exist,
then the full original target name becomes the bound file name.
.RE
.PP
There is potential here for a lot of search operations.  The trick is to
define .SOURCE.x special targets with short search lists and leave .SOURCE
as short as possible.
The search algorithm has the following useful side effect.
When a target having the .LIBMEMBER (library member) attribute is searched for,
it is first searched for as an ordinary file.
When a number of library members require updating it is desirable to compile
all of them first and to update the library at the end in a single operation.
If one of the members does not compile and \fBdmake\fP stops, then
the user may fix the error and make again.  \fBdmake\fP will not remake any
of the targets whose object files have already been generated as long as
none of their prerequisite files have been modified as a result of the fix.
.PP
When \fBdmake\fP constructs target pathnames './' substrings are removed and
substrings of the form 'foo/..' are eliminated.  This may result in somewhat
unexpected values of the macro expansion \fB$@\fP, but is infact the corect
result.
.PP
When defining .SOURCE and .SOURCE.x targets the construct
.RS
.sp
.SOURCE :
.br
.SOURCE : fred gery
.sp
.RE
is equivalent to
.RS
.sp
.SOURCE :\- fred gery
.RE
.PP
\fBdmake\fP correctly handles the UNIX Make variable VPATH.  By definition VPATH
contains a list of ':' separated directories to search when looking for a
target.  \fBdmake\fP maps VPATH to the following special rule:
.RS
.sp
.SOURCE :^ $(VPATH:s/:/ /)
.sp
.RE
Which takes the value of VPATH and sets .SOURCE to the same set of directories
as specified in VPATH.
.SH "PERCENT(%) RULES AND MAKING INFERENCES"
When \fBdmake\fP makes a target, the target's set of prerequisites (if any)
must exist and the target must have a recipe which \fBdmake\fP
can use to make it.
If the makefile does not specify an explicit recipe for the target then
.B dmake
uses special rules to try to infer a recipe which it can use
to make the target.  Previous versions of Make perform this task by using
rules that are defined by targets of the form .<suffix>.<suffix> and by
using the .SUFFIXES list of suffixes.  The exact workings of this mechanism
were sometimes difficult to understand and often limiting in their usefulness.
Instead, \fBdmake\fP supports the concept of \fI%-meta\fP rules.  
The syntax and semantics of these rules differ from standard rule lines as
follows:
.sp
.nf
.RS
\fI<%-target>\fP [\fI<attributes>\fP] \fI<ruleop>\fP [\fI<%-prerequisites>\fP] [;\fI<recipe>\fP]
.RE
.fi
.sp
where \fI%-target\fP is a target containing exactly a single `%' sign,
.I attributes
is a list (possibly empty) of attributes,
.I ruleop
is the standard set of rule operators,
.I "%-prerequisites"
\&, if present, is a list of prerequisites containing zero or more `%' signs,
and
.I recipe,
if present, is the first line of the recipe.
.PP
The
.I %-target
defines a pattern against which a target whose recipe is
being inferred gets matched.  The pattern match goes as follows:  all chars are
matched exactly from left to right up to but not including the % sign in the
pattern, % then matches the longest string from the actual target name
not ending in
the suffix given after the % sign in the pattern.
Consider the following examples:
.RS
.sp
.nf
.Is "dir/%.c   "
.Ii "%.c"
matches fred.c but not joe.c.Z
.Ii "dir/%.c"
matches dir/fred.c but not dd/fred.c
.Ii "fred/%"
matches fred/joe.c but not f/joe.c
.Ii "%"
matches anything
.fi
.sp
.RE
In each case the part of the target name that matched the % sign is retained
and is substituted for any % signs in the prerequisite list of the %-meta rule
when the rule is selected during inference and
.B dmake
constructs the new dependency.
As an example the following %-meta rules describe the following:
.RS
.sp
%.c : %.y ; recipe...
.sp
.RE
describes how to make any file ending in .c if a corresponding file ending
in .y can be found.
.RS
.sp
foo%.o : fee%.k ; recipe...
.sp
.RE
is used to describe how to make fooxxxx.o from feexxxx.k.
.RS
.sp
%.a :; recipe...
.sp
.RE
describes how to make a file whose suffix is .a without inferring any
prerequisites.
.RS
.sp
%.c : %.y yaccsrc/%.y ; recipe...
.sp
.RE
is a short form for the construct:
.RS
.sp
%.c : %.y ; recipe...
.br
%.c : yaccsrc/%.y ; recipe...
.sp
.RE
ie. It is possible to specify the same recipe for two %-rules by giving
more than one prerequisite in the prerequisite list.
A more interesting example is:
.RS
.sp
% : RCS/%,v ; co $<
.sp
.RE
which describes how to take any target and check it out of
the RCS directory if the corresponding file exists in the RCS directory.
The equivalent SCCS rule would be:
.RS
.sp
% : s.% ; get $<
.sp
.RE
.PP
The previous RCS example defines an infinite rule, because it says how to make
.I anything
from RCS/%,v, and
.I anything
also includes RCS/fred.c,v.
To limit the size of the graph that results from such rules
.B dmake
uses the macro variable PREP (stands for % repetition).  By default the value
of this variable is 0, which says that no repetitions of a %-rule are to be
generated.  If it is set to something greater than 0, then that many
repetitions of any infinite %-rule are allowed.  If in the above
example PREP was set to 1, then \fBdmake\fP would generate the dependency
graph:
.RS
.sp
% --> RCS/%,v --> RCS/RCS/%,v,v
.sp
.RE
Where each link is assigned the same recipe as the first link.
PREP should be used only in special cases, since it may result in
a large increase in the number of possible prerequisites tested.
.B dmake
further assumes that any target that has no suffix can be made from
a prerequisite that has at least one suffix.
.PP
.B dmake
supports dynamic prerequisite generation for prerequisites of %-meta rules.
This is best illustrated by an example.  The RCS rule shown above can infer
how to check out a file from a corresponding RCS file only if the target
is a simple file name with no directory information.  That is, the above rule
can infer how to find \fIRCS/fred.c,v\fP from the target \fIfred.c\fP,
but cannot infer how to find \fIsrcdir/RCS/fred.c,v\fP from \fIsrcdir/fred.c\fP
because the above rule will cause \fBdmake\fP to look for RCS/srcdir/fred.c,v;
which does not exist (assume that srcdir has its own RCS directory as is the
common case).
.PP
A more versatile formulation of the above RCS check out rule is the following:
.RS
.sp
% :  $$(@:d)RCS/$$(@:f),v : co $@
.sp
.RE
This rule uses the dynamic macro $@ to specify the prerequisite to try to
infer.  During inference of this rule the macro $@ is set to the value of
the target of the %-meta rule and the appropriate prerequisite is generated by
extracting the directory portion of the target name (if any), appending the
string \fIRCS/\fP to it, and appending the target file name with a trailing
\fI,v\fP attached to the previous result.
.PP
.B dmake
can also infer indirect prerequisites.
An inferred target can have a list of prerequisites added that will not
show up in the value of $< but will show up in the value of $? and $&.
Indirect prerequisites are specified in an inference rule by quoting the
prerequisite with single quotes.  For example, if you had the explicit
dependency:
.RS
.sp
.nf
fred.o : fred.c ; rule to make fred.o
fred.o : local.h
.fi
.sp
.RE
then this can be inferred for fred.o from the following inference rule:
.RS
.sp
%.o : %.c 'local.h' ; makes a .o from a .c
.sp
.RE
You may infer indirect prerequisites that are a function of the value of '%'
in the current rule.  The meta-rule:
.RS
.sp
%.o : %.c '$(INC)/%.h' ; rule to make a .o from a .c
.sp
.RE
infers an indirect prerequisite found in the INC directory whose name is the
same as the expansion of $(INC), and the prerequisite name depends on the
base name of the current target.
The set of indirect prerequisites is attached to the meta rule in which they
are specified and are inferred only if the rule is used to infer a recipe
for a target.  They do not play an active role in driving the inference
algorithm.
The construct:
.RS
.sp
%.o : %.c %.f 'local.h'; recipe
.sp
.RE
is equivalent to:
.RS
.sp
.nf
%.o : %.c 'local.h' : recipe
.fi
.sp
.RE
while:
.RS
.sp
%.o :| %.c %.f 'local.h'; recipe
.sp
.RE
is equivalent to:
.RS
.sp
.nf
%.o : %.c 'local.h' : recipe
%.o : %.f 'local.h' : recipe
.fi
.sp
.RE
.PP
If any of the attributes .SETDIR, .EPILOG, .PROLOG, .SILENT,
\&.USESHELL, .SWAP, .PRECIOUS, .LIBRARY, .NOSTATE and .IGNORE
are given for a %-rule then when that rule is bound to a target
as the result of an inference, the target's set of attributes is augmented by
the attributes from the above set that are specified in the bound %-rule.
Other attributes specified for %-meta rules are not inherited by the target.
The .SETDIR attribute is treated in a special way.
If the target already had a .SETDIR attribute set then
.B dmake
changes to that directory prior to performing the inference.
During inference any .SETDIR attributes for the inferred prerequisite
are honored.
The directories must exist for a %-meta rule to be selected as a possible
inference path.  If the directories do not exist no error message is issued,
instead the corresponding path in the inference graph is rejected.
.PP
.B dmake
also supports the old format special target .<suffix>.<suffix>
by identifying any rules
of this form and mapping them to the appropriate %-rule.  So for example if
an old makefile contains the construct:
.RS
.sp
\&.c.o :; cc \-c $< \-o $@
.sp
.RE
.B dmake
maps this into the following %-rule:
.RS
.sp
%.o : %.c; cc \-c $< \-o $@
.sp
.RE
Furthermore,
.B dmake
understands several SYSV AUGMAKE special targets and maps them into
corresponding %-meta rules.  These transformation must be enabled by providing
the \-A flag on the command line or by setting the value of AUGMAKE to
non\-NULL.
The construct
.RS
.sp
\&.suff :; recipe
.sp
.RE
gets mapped into:
.RS
.sp
% : %.suff; recipe
.sp
.RE
and the construct
.RS
.sp
\&.c~.o :; recipe
.sp
.RE
gets mapped into:
.RS
.sp
%.o : s.%.c ; recipe
.sp
.RE
In general, a special target of the form .<str>~ is replaced by the %-rule
construct s.%.<str>, thereby providing support for the syntax used by SYSV
AUGMAKE for providing SCCS support.
When enabled, these mappings allow processing of existing SYSV
makefiles without modifications.
.PP
.B dmake
bases all of its inferences on the inference graph constructed from the
%-rules defined in the makefile.
It knows exactly which targets can be made from which prerequisites by
making queries on the inference graph.  For this reason .SUFFIXES is not
needed and is completely ignored.
.PP
For a %-meta rule to be inferred as the
rule whose recipe will be used to make a target, the target's name must match
the %-target pattern, and any inferred %-prerequisite must already exist or
have an explicit recipe so that the prerequisite can be made.
Without \fItransitive closure\fP on the inference graph the above rule
describes precisely when an inference match terminates the search.
If transitive closure is enabled (the usual case), and a prerequisite does
not exist or cannot be made, then
.B dmake
invokes the inference algorithm recursively on the prerequisite to see if
there is some way the prerequisite can be manufactured.  For, if the
prerequisite can be made then the current target can also be made using the
current %-meta rule.
This means that there is no longer a need to give a rule
for making a .o from a .y if you have already given a rule for making a .o
from a .c and a .c from a .y.  In such cases
.B dmake
can infer how to make the
\&.o from the .y via the intermediary .c and will remove the .c when the .o is
made.  Transitive closure can be disabled by giving the \-T switch on the
command line.
.PP
A word of caution.
.B dmake
bases its transitive closure on the %-meta rule targets.
When it performs transitive closure it infers how to make a target from a
prerequisite by performing a pattern match as if the potential prerequisite
were a new target.
The set of rules:
.RS
.nf
.sp
%.o : %.c :; rule for making .o from .c
%.c : %.y :; rule for making .c from .y
% : RCS/%,v :; check out of RCS file
.fi
.sp
.RE
will, by performing transitive closure, allow \fBdmake\fP to infer how to make
a .o from a .y using a .c as an intermediate temporary file.  Additionally
it will be able to infer how to make a .y from an RCS file, as long as that
RCS file is in the RCS directory and has a name which ends in .y,v.
The transitivity computation is performed dynamically for each target that
does not have a recipe.  This has potential to be costly if the %-meta
rules are not carefully specified.  The .NOINFER attribute is used to mark
a %-meta node as being a final target during inference.  Any node with this
attribute set will not be used for subsequent inferences.  As an example
the node RCS/%,v is marked as a final node since we know that if the RCS file
does not exist there likely is no other way to make it.  Thus the standard
startup makefile contains an entry similar to:
.RS
.nf
\&.NOINFER : RCS/%,v
.fi
.RE
Thereby indicating that the RCS file is the end of the inference chain.
Whenever the inference algorithm determines that a target can be made from
more than one prerequisite and the inference chains for the two methods
are the same length the algorithm reports an ambiguity and prints the
ambiguous inference chains.
.PP
.B dmake
tries to
remove intermediate files resulting from transitive closure if the file
is not marked as being PRECIOUS, or the \fB\-u\fP flag was not given on the
command line, and if the inferred intermediate did not previously exist.
Intermediate targets that existed prior to being made are never removed.
This is in keeping with the philosophy that
.B dmake
should never remove things from the file system that it did not add.
If the special target .REMOVE is defined and has a recipe then
.B dmake
constructs a list of the intermediate files to be removed and makes them
prerequisites of .REMOVE.  It then makes .REMOVE thereby removing the
prerequisites if the recipe of .REMOVE says to.  Typically .REMOVE is defined
in the startup file as:
.RS
.sp
\&.REMOVE :; $(RM) $<
.RE
.SH "MAKING TARGETS"
In order to update a target \fBdmake\fP must execute a recipe.
When a recipe needs to be executed it is first expanded so that any macros
in the recipe text are expanded, and it is then either executed directly or
passed to a shell.
.B dmake
supports two types of recipes.  The regular recipes and group recipes.
.PP
When a regular recipe is invoked \fBdmake\fP executes each line of the recipe
separately using a new copy of a shell if a shell is required.
Thus effects of commands do not generally persist across recipe lines
(e.g. cd requests in a recipe line do not carry over to the next recipe line).
This is true even in environments such as \fBMSDOS\fP, where dmake internally
sets the current working director to match the directory it was in before
the command was executed.
.PP
The decision on whether a shell is required to execute a command is based on
the value of the macro SHELLMETAS or on the specification of '+' or .USESHELL
for the current recipe or target respectively.
If any character in the value of
SHELLMETAS is found in the expanded recipe text-line or the use of a shell
is requested explicitly via '+' or .USESHELL then the command is
executed using a shell, otherwise the command is executed directly.
The shell that is used for execution is given by the value of the macro SHELL.
The flags that are passed to the shell are given by the value of SHELLFLAGS.
Thus \fBdmake\fP constructs the command line:
.sp
\t$(SHELL) $(SHELLFLAGS) $(expanded_recipe_command)
.sp
Normally
.B dmake
writes the command line that it is about to invoke to standard output.
If the .SILENT attribute is set for the target or for
the recipe line (via @), then the recipe line is not echoed.
.PP
Group recipe processing is similar to that of regular recipes, except that
a shell is always invoked.  The shell that is invoked is given by the value of
the macro GROUPSHELL, and its flags are taken from the value of the macro
GROUPFLAGS.  If a target has the .PROLOG attribute set then
.B dmake
prepends to the shell script the recipe associated with the special target
\&.GROUPPROLOG, and if the attribute .EPILOG is set as well, then the recipe
associated with the special target .GROUPEPILOG is appended to the script
file.
This facility can be used to always prepend a common header and common trailer
to group recipes.
Group recipes are echoed to standard output just like standard recipes, but
are enclosed by lines beginning with [ and ].
.PP
The recipe flags [+,\-,%,@] are recognized at the start of a recipe line
even if they appear in a macro.  For example:
.RS
.sp
.nf
SH = +
all:
\t$(SH)echo hi
.fi
.sp
.RE
is completely equivalent to writing
.RS
.sp
.nf
SH = +
all:
\t+echo hi
.fi
.sp
.RE
.PP
The last step performed by
.B dmake
prior to running a recipe is to set the macro CMNDNAME to the name of the
command to execute (determined by finding the first white\-space ending token
in the command line).  It then sets the macro CMNDARGS to be the remainder
of the line.
.B dmake
then expands the macro COMMAND which by default is set to
.RS
.sp
COMMAND = $(CMNDNAME) $(CMNDARGS)
.sp
.RE
The result of this final expansion is the command that will be executed.
The reason for this expansion is to allow for a different interface to
the argument passing facilities (esp. under DOS) than that provided by
.B dmake\fR.\fP
You can for example define COMMAND to be
.RS
.sp
COMMAND = $(CMNDNAME) @$(mktmp $(CMNDARGS))
.sp
.RE
which dumps the arguments into a temporary file and runs the command
.RS
.sp
$(CMNDNAME) @/tmp/ASAD23043
.sp
.RE
which has a much shorter argument list.  It is now up to the command to
use the supplied argument as the source for all other arguments.
As an optimization, if COMMAND is not defined
.B dmake
does not perform the above expansion.  On systems, such as UNIX, that
handle long command lines this provides a slight saving in processing the
makefiles.
.SH "MAKING LIBRARIES"
Libraries are easy to maintain using \fBdmake\fP.  A library is a file
containing a collection of object files.
Thus to make a library you simply specify it as a target with the .LIBRARY
attribute set and specify its list of prerequisites.  The prerequisites should
be the object members that are to go into the library.  When
.B dmake
makes the library target it uses the .LIBRARY attribute to pass to the
prerequisites the .LIBMEMBER attribute and the name of the library.  This
enables the file binding mechanism to look for the member in the library if an
appropriate object file cannot be found. 
.B dmake
now supports \fBElf\fP libraries on systems that support \fBElf\fP and
hence supports, on those systems, long member file names.
A small example best illustrates this.
.RS
.nf
.sp
mylib.a .LIBRARY : mem1.o mem2.o mem3.o
\trules for making library...
\t# remember to remove .o's when lib is made
.sp
# equivalent to:  '%.o : %.c ; ...'
\&.c.o :; rules for making .o from .c say
.sp
.fi
.RE
.B dmake
will use the .c.o rule for making the library members if appropriate .c files
can be found using the search rules.  NOTE:  this is not specific in any way
to C programs, they are simply used as an example.
.PP
.B dmake
tries to handle the old library construct format in a sensible way.
The construct 
.I lib(member.o)
is separated and the \fIlib\fP portion is declared
as a library target.
The new target is defined
with the .LIBRARY attribute set and the \fImember.o\fP portion of the
construct is
declared as a prerequisite of the lib target.
If the construct \fIlib(member.o)\fP
appears as a prerequisite of a target in the
makefile, that target has the new name of the lib assigned as its
prerequisite.  Thus the following example:
.RS
.sp
.nf
a.out : ml.a(a.o) ml.a(b.o); $(CC) \-o $@  $<

\&.c.o :; $(CC) \-c $(CFLAGS) \-o $@  $<
%.a:
.RS
ar rv $@ $?
ranlib $@
rm \-rf $?
.RE
.sp
.fi
.RE
constructs the following dependency
graph.
.RS
.sp
.nf
a.out : ml.a; $(CC) \-o $@  $<
ml.a .LIBRARY : a.o b.o

%.o : %.c ; $(CC) -c $(CFLAGS) \-o $@  $<
%.a :
.RS
ar rv $@ $?
ranlib $@
rm -rf $?
.RE
.sp
.fi
.RE
and making a.out then works as expected.
.PP
The same thing happens for any target of the form \fIlib((entry))\fP.
These targets have an
additional feature in that the \fIentry\fP target has the .SYMBOL attribute
set automatically.
.PP
NOTE:  If the notion of entry points is supported by the archive and by
\fBdmake\fP (currently not the case) then
.B dmake
will search the archive for the entry point and return not only the
modification time of the member which defines the entry but also the name of
the member file.  This name will then replace \fIentry\fP and will be used for
making the member file.  Once bound to an archive member the .SYMBOL
attribute is removed from the target.
This feature is presently disabled as there is little standardization
among archive formats, and we have yet to find a makefile utilizing this
feature (possibly due to the fact that it is unimplemented in most versions
of UNIX Make).
.PP
Finally, when
.B dmake
looks for a library member it must first locate the library file.
It does so by first looking for the library relative to the current directory
and if it is not found it then looks relative to the current value of
$(TMD).  This allows commonly used libraries to be kept near the root of
a source tree and to be easily found by
.B dmake\fR.\fP
.SH "KEEP STATE"
.B dmake
supports the keeping of state information for targets that it makes whenever
the macro .KEEP_STATE is assigned a value.  The value of the macro should be
the name of a state file that will contain the state information.  If state
keeping is enabled then each target that does not poses the .NOSTATE
attribute will have a record written into the state file indicating the
target's name, the current directory, the command used to update the target,
and which, if any, :: rule is being used.  When you make this target again
if any of this information does not match the previous settings and the
target is not out dated it will still be re\-made.  The assumption is that one
of the conditions above has changed and that we wish to remake the target.
For example,
state keeping is used in the maintenance of
.B dmake
to test compile different versions of the source using different compilers.
Changing the compiler causes the compilation flags to be modified and hence
all sources to be recompiled.
.PP
The state file is an ascii file and is portable, however it is
not in human readable form as the entries represent hash keys of the above
information.
.PP
The Sun Microsystem's Make construct
.RS
.sp
\&.KEEP_STATE :
.sp
.RE
is recognized and is mapped to \fB.KEEP_STATE:=_state.mk\fP.
The
.B dmake
version of state keeping does not include scanning C source files for
dependencies like Sun Make.  This is specific to C programs and it was
felt that it does not belong in make.
.B dmake
instead provides the tool, \fBcdepend\fP, to scan C source files and to produce
depedency information.  Users are free to modify cdepend to produce other
dependency files.  (NOTE:
.B cdepend
does not come with the distribution at this time, but will be available in
a patch in the near future)
.SH "MULTI PROCESSING"
If the architecture supports it then \fBdmake\fP is capable of making a target's
prerequisites in parallel.  \fBdmake\fP will make as much in parallel as it
can and use a number of child processes up to the maximum specified by
MAXPROCESS or by the value supplied to the \-P command line flag.
A parallel make is enabled by setting the value of MAXPROCESS (either directly
or via \-P option) to a value which is > 1.
\fBdmake\fP guarantees that all dependencies as specified in the makefile are
honored.  A target will not be made until all of its prerequisites have been
made.  Note that when you specify \fB-P 4\fP then four child processes are
run concurrently but \fBdmake\fP actually displays the fifth command it will
run immediately upon a child process becomming free.  This is an artifact of
the method used to traverse the dependency graph and cannot be removed.
If a parallel make is being performed then the following restrictions on
parallelism are enforced.
.RS
.IP 1.
Individual recipe lines in a non-group recipe are performed sequentially in
the order in which they are specified within the makefile and in parallel with
the recipes of other targets.
.IP 2.
If a target contains multiple recipe definitions (cf. :: rules) then these are
performed sequentially in the order in which the :: rules are specified within
the makefile and in parallel with the recipes of other targets.
.IP 3.
If a target rule contains the `!' modifier, then the recipe is performed
sequentially for the list of outdated prerequisites and in parallel with the recipes of other targets.
.IP 4.
If a target has the .SEQUENTIAL attribute set then all of its prerequisites
are made sequentially relative to one another (as if MAXPROCESS=1), but in
parallel with other targets in the makefile.
.RE
.PP
Note:  If you specify a parallel make then
the order of target update and the order in which the associated recipes are
invoked will not correspond to that displayed by the \-n flag.
.SH "CONDITIONALS"
.B dmake
supports a makefile construct called a \fIconditional\fR.  It allows
the user
to conditionally select portions of makefile text for input processing
and to discard other portions.  This becomes useful for
writing makefiles that are intended to function for more than one target
host and environment.  The conditional expression is specified as follows:
.sp
.RS
.nf
\&.IF  \fIexpression\fR
   ... if text ...
\&.ELIF  \fIexpression\fR
   ... if text ...
\&.ELSE
   ... else text ...
\&.END
.RE
.fi
.sp
The .ELSE and .ELIF portions are optional, and the conditionals may be
nested (ie.  the text may contain another conditional).
\&.IF, .ELSE, and .END
may appear anywhere in the makefile, but a single conditional expression
may not span multiple makefiles.
.PP
\fIexpression\fR can be one of the following three forms:
.sp
\t<text> | <text> == <text> | <text> != <text>
.sp
where \fItext\fR is either text or a macro expression.  In any case,
before the comparison is made, the expression is expanded.  The text
portions are then selected and compared.  White space at the start and
end of the text portion is discarded before the comparison.  This means
that a macro that evaluates to nothing but white space is considered a
NULL value for the purpose of the comparison.
In the first case the expression evaluates TRUE if the text is not NULL
otherwise it evaluates FALSE.  The remaining two cases both evaluate the
expression on the basis of a string comparison.
If a macro expression needs to be equated to a NULL string then compare it to
the value of the macro $(NULL).
You can use the $(shell ...) macro to construct more complex test expressions.
.SH "EXAMPLES"
.RS
.nf
.sp
# A simple example showing how to use make
#
prgm : a.o b.o
	cc a.o b.o \-o prgm
a.o : a.c g.h
	cc a.c \-o $@
b.o : b.c g.h
	cc b.c \-o $@
.fi
.RE
.sp
In the previous
example prgm is remade only if a.o and/or b.o is out of date with
respect to prgm.
These dependencies can be stated more concisely
by using the inference rules defined in the standard startup file.
The default rule for making .o's from .c's looks something like this:
.sp
\&\t%.o : %.c; cc \-c $(CFLAGS) \-o $@ $<
.sp
Since there exists a rule (defined in the startup file)
for making .o's from .c's
\fBdmake\fR will use that rule
for manufacturing a .o from a .c and we can specify our dependencies
more concisely.
.sp
.RS
.nf
prgm : a.o b.o
	cc \-o prgm $<
a.o b.o : g.h
.fi
.RE
.sp
A more general way to say the above using the new macro expansions
would be:
.sp
.RS
.nf
SRC = a b
OBJ = {$(SRC)}.o
.sp
prgm : $(OBJ)
	cc \-o $@ $<
.sp
$(OBJ) : g.h
.fi
.RE
.sp
If we want to keep the objects in a separate directory, called
objdir, then we would write
something like this.
.sp
.RS
.nf
SRC = a b
OBJ = {$(SRC)}.o
.sp
prgm : $(OBJ)
	cc $< \-o $@
.sp
$(OBJ) : g.h
\&%.o : %.c
	$(CC) \-c $(CFLAGS) \-o $(@:f) $<
	mv $(@:f) objdir

\&.SOURCE.o : objdir   # tell dmake to look here for .o's
.fi
.RE
.sp
An example of building library members would go something like this:
(NOTE:  The same rules as above will be used to produce .o's from .c's)
.sp
.RS
.nf
SRC\t= a b
LIB\t= lib
LIBm\t= { $(SRC) }.o
.sp
prgm: $(LIB)
	cc \-o $@ $(LIB)
.sp
$(LIB) .LIBRARY : $(LIBm)
	ar rv $@ $<
	rm $<
.fi
.RE
.sp
Finally, suppose that each of the source files in the previous example had
the `:' character in their target name.  Then we would write the above example
as:
.sp
.RS
.nf
SRC\t= f:a f:b
LIB\t= lib
LIBm\t= "{ $(SRC) }.o"	 # put quotes around each token
.sp
prgm: $(LIB)
	cc \-o $@ $(LIB)
.sp
$(LIB) .LIBRARY : $(LIBm)
	ar rv $@ $<
	rm $<
.fi
.RE
.SH "COMPATIBILITY"
There are two notable differences between 
.B \fBdmake\fR
and the standard version of BSD UNIX 4.2/4.3 Make.
.RS
.IP 1. .3i
BSD UNIX 4.2/4.3 Make supports wild card filename expansion for
prerequisite names.  Thus if a directory contains a.h, b.h and c.h, then a
line like
.sp
\ttarget: *.h
.sp
will cause UNIX make to expand the *.h into "a.h b.h c.h".  \fBdmake\fR
does not support this type of filename expansion.
.IP 2. .3i
Unlike UNIX make, touching a library member causes \fBdmake\fR
to search the library for the member name and to update the library time stamp.
This is only implemented in the UNIX version.
MSDOS and other versions may not have librarians that keep file time stamps,
as a result \fBdmake\fR touches the library file itself, and prints a warning.
.RE
.PP
\fBdmake\fP is not compatible with GNU Make.  In particular it does not
understand GNU Make's macro expansions that query the file system.
.PP
.B dmake
is fully compatible with SYSV AUGMAKE, and supports the following AUGMAKE
features:
.RS
.IP 1. .3i
GNU Make style \fBinclude\fP, and \fBif/else/endif\fP directives are allowed
in non-group recipes.
Thus, the word \fBinclude\fP appearing at
the start of a line that is not part of a gruop recipe will be mapped
to the ".INCLUDE" directive that \fBdamke\fP uses.
Similarly, the words \fBifeq\fP,\fBifneq\fP,\fBelif\fP,\fBelse\fP,
and \fBendif\fP are mapped to their corresponding \fBdmake\fP equivalents.
.IP 2. .3i
The macro modifier expression $(macro:str=sub) is understood and is equivalent
to the expression $(macro:s/str/sub), with the restriction that str must match
the following regular expression:
.sp
\tstr[ |\et][ |\et]*
.sp
(ie. str only matches at the end of a token where str is a suffix and is
terminated by a space, a tab, or end of line)
Normally \fIsub\fP is expanded before the substitution is made, if you specify
\-A on the command line then sub is not expanded.
.IP 3.
The macro % is defined to be $@ (ie. $% expands to the same value as $@).
.IP 4.
The AUGMAKE notion of libraries is handled correctly.
.IP 5.
When defining special targets for the inference rules and the AUGMAKE special
target handling is enabled then the special target
\&.X is equivalent to the %-rule "% : %.X".
.IP 6.
Directories are always made if you specify \fB\-A\fP.  This is consistent
with other UNIX versions of Make.
.IP 7.
Makefiles that utilize virtual targets to force making of other targets work
as expected if AUGMAKE special target handling is enabled.  For example:
.sp
.nf
\tFRC:
\tmyprog.o : myprog.c $(FRC) ; ...
.fi
.sp
Works as expected if you issue the command
.sp
\t'\fBdmake\fP \-A FRC=FRC'
.sp
but fails with a 'don't know how to make FRC'
error message if you do not specify AUGMAKE special target handling via
the \-A flag (or by setting AUGMAKE:=yes internally).
.IP 8.
The \fBMSDOS\fP version of \fBdmake\fP now supports a single buitin runtime
command \fBnoop\fP, which returns success if requested and does nothing.
.RE
.SH "LIMITS"
In some environments the length of an argument string is restricted.
(e.g. MSDOS command line arguments cannot be longer than 128 bytes if you are
using the standard command.com command interpreter as your shell,
.B dmake
text diversions may help in these situations.)
.SH "PORTABILITY"
To write makefiles that can be moved from one environment to another requires
some forethought.  In particular you must define as macros all those things
that may be different in the new environment.
.B dmake
has two facilities that help to support writing portable makefiles, recursive
macros and conditional expressions.  The recursive macros, allow one to define
environment configurations that allow different environments for similar types
of operating systems.  For example the same make script can be used for SYSV and
BSD but with different macro definitions.
.PP
To write a makefile that is portable between UNIX and MSDOS requires both
features since in almost all cases you will need to define new recipes for
making targets.  The recipes will probably be quite different since the
capabilities of the tools on each machine are different.  Different
macros will be needed to help handle the smaller differences in the two
environments.
.SH FILES
Makefile, makefile, startup.mk (use dmake \-V to tell you where the startup
file is)
.SH "SEE ALSO"
sh(1), csh(1), touch(1), f77(1), pc(1), cc(1)
.br
S.I. Feldman  \fIMake - A Program for Maintaining Computer Programs\fP
.SH "AUTHOR"
Dennis Vadura, dvadura@wticorp.com
.br
Many thanks to Carl Seger for his helpful suggestions,
and to Trevor John Thompson for his many excellent ideas and
informative bug reports.  Many thanks also go to those on the
NET that have helped in making \fBdmake\fP one of the best Make tools
available.
.SH BUGS
Some system commands return non-zero status inappropriately.
Use
.B \-i
(`\-' within the makefile) to overcome the difficulty.
.PP
Some systems do not have easily accessible
time stamps for library members (MSDOS, AMIGA, etc)
for these \fBdmake\fR uses the time stamp of the library instead and prints
a warning the first time it does so.  This is almost always ok, except when
multiple makefiles update a single library file.  In these instances it is
possible to miss an update if one is not careful.
.PP
This man page is way too long.
.SH WARNINGS
Rules supported by make(1) may not work if transitive closure is turned off
(-T, .NOINFER).
.PP
PWD from csh/ksh will cause problems if a cd operation is performed and
-e or -E option is used.
.PP
Using internal macros such as COMMAND, may wreak havoc if you don't understand
their functionality.
