# This is the root DMAKE startup file.
#
# Definitions common to all environments are given at the root.
# Definitions parameterized at the root have their parameters specified
# in sub-makefiles which are included based on the values of the three
# make variables:
#
#	OS		- core operating system flavour
#
# See the file 'summary', found in this directory for a list of
# environments supported by this release.

# Disable warnings for macros given on the command line but redefined here.
__.silent !:= $(.SILENT)	# Preserve user's .SILENT flag
.SILENT   !:= yes
.NOTABS	  !:= yes
.ERROR :
    @echo '---* $(MKFILENAME) *---'

# Define the directory separator string.
/ *=  $(DIRSEPSTR)

.IMPORT .IGNORE : .EVERYTHING

.IF "$(TNR)" == ""

CALLMACROS:=$(MAKEMACROS)
.EXPORT : CALLMACROS

CALLTARGETS=$(MAKETARGETS)
.ENDIF			# "$(TNR)" == ""

.INCLUDEDIRS : $(DMAKE_INC) $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT) $(SOLARENV)$/inc

# Grab key definitions from the environment
# .IMPORT .IGNORE : OS TMPDIR UPD INPATH OUTPATH GUI COM EX CLASSPATH PRODUCT GUIBASE SOLARENV SOLARVER GUIENV CPU CPUNAME GVER COMPATH LIB INCLUDE TR SORT UNIQ ROOTDIR SED AWK TOUCH IENV ILIB PRODUCT_ENV COMEX UPD BUILD DEVROOT VCL SO3 ENVCFLAGS

.IF "$(OS)" == "" || "$(OS)" == "Windows_NT"
.ERROR : ; @echo Forced error: Environment variable OS has to be set for OOo build!
OS_variable_needed
.ENDIF

# Customize macro definitions based on seting of OS.
# This must come before the default macro definitions which follow.
.IF "$(OS)" == "WNT"
.INCLUDE: $(INCFILENAME:d)wnt/macros.mk
.END

# ----------------- Default Control Macro definitions -----------------------
# Select appropriate defaults for basic macros
MAKE          *=  $(MAKECMD) $(MFLAGS)
TMPDIR        *:= $/tmp
DIVFILE       *=  $(TMPFILE)

MAXLINELENGTH	= 65530

# Recipe execution configuration
SHELL := $(OOO_SHELL)
SHELLFLAGS := -c
SHELLMETAS := !"\#$$%&'()*;<=>?[\]`{{|}}~
    # the colon (":") utility is typically only available as a shell built-in,
    # so it should be included in SHELLMETAS; however, this would result in very
    # many false positives on Windows where ":" is used in drive letter notation
GROUPSHELL := $(SHELL)
GROUPFLAGS :=
GROUPSUFFIX :=

# Intermediate target removal configuration
RM            *:= rm
RMFLAGS       *=  -f
RMTARGET      *=  $<

# Default recipe that is used to remove intermediate targets.
.REMOVE :; #echo dummy remove #$(RM) $(RMFLAGS) $(RMTARGET)
#.REMOVE :; echo $(RM) $(RMFLAGS) $(RMTARGET)

# Directory caching configuration.
.DIRCACHE         *:= yes
.DIRCACHERESPCASE *:= yes

# Define the special NULL Prerequisite
NULLPRQ *:= __.NULLPRQ

# Primary suffixes in common use
E *:= # Executables

# Other Compilers, Tools and their flags
MV *:= mv     # File rename command


# Finally, define the default construction strategy
.ROOT .PHONY .NOSTATE .SEQUENTIAL :- .INIT .TARGETS .DONE;
.INIT .DONE .PHONY: $(NULLPRQ);

# Define the NULL Prerequisite as having no recipe.
$(NULLPRQ) .PHONY :;

# Reset warnings back to previous setting.
.SILENT !:= $(__.silent)

# Check for a Local project file, gets parsed before user makefile.
.INCLUDE .IGNORE .NOINFER: "project.mk"

.MAKEFILES :- makefile.mk MAKEFILE.MK $(CONFIG_PROJECT)$/makefile.rc $(SOLARENV)$/inc$/found_no.mk

LANG=C
.EXPORT : LANG
