# This is the root DMAKE startup file.
#
# Definitions common to all environments are given at the root.
# Definitions parameterized at the root have their parameters specified
# in sub-makefiles which are included based on the values of the three
# make variables:
#
#	OS		- core operating system flavour
#	OSRELEASE	- specific release of the operating system
#	OSENVIRONMENT	- software construction environment in use
# 
# See the file 'summary', found in this directory for a list of
# environments supported by this release.

# Disable warnings for macros given on the command line but redefined here.
__.silent !:= $(.SILENT)	# Preserve user's .SILENT flag
.SILENT   !:= yes

# startup.mk configuration parameters, for each, set it to non-null if you wish
# to enable the named facility.
__.HAVE_RCS  !:= yes 		# yes => RCS  is installed.
__.HAVE_SCCS !:=    		# yes => SCCS is installed.
__.DEFAULTS  !:= yes		# yes => define default construction rules.
__.EXECS     !:= yes            # yes => define how to build executables.

# Grab key definitions from the environment
.IMPORT .IGNORE : OS OSRELEASE OSENVIRONMENT TMPDIR SHELL

# Default DMAKE configuration, if not overriden by environment
.INCLUDE .NOINFER $(!null,$(OS) .IGNORE) : $(INCFILENAME:d)config.mk

# Look for a local defaults configuration
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)local.mk

# Define the directory separator string.
/ *=  $(DIRSEPSTR)

# Customize macro definitions based on setings of OS, OSRELEASE and
# OSENVIRONMENT, this must come before the default macro definitions which
# follow.
.INCLUDE .NOINFER .IGNORE : $(INCFILENAME:d)$(OS)$/macros.mk

# ----------------- Default Control Macro definitions -----------------------
# Select appropriate defaults for basic macros
   MAKE          *=  $(MAKECMD) -S $(MFLAGS)
   TMPDIR        *:= $/tmp
   DIVFILE       *=  $(TMPFILE)
   AUGMAKE       *:= no

# Recipe execution configuration
   SHELL	 *:= $/bin$/sh
   SHELLFLAGS	 *:= -ce
   GROUPSHELL	 *:= $(SHELL)
   GROUPFLAGS	 *:= 
   SHELLMETAS	 *:= |();&<>?*][$$:\\#`'"
   GROUPSUFFIX	 *:=

# Intermediate target removal configuration
   RM            *:= $/bin$/rm
   RMFLAGS       *=  -f
   RMTARGET      *=  $<

# Default recipe that is used to remove intermediate targets.
.REMOVE :; $(RM) $(RMFLAGS) $(RMTARGET)

# Check and enable AUGMAKE extensions for SYSV compatibility
.IF $(AUGMAKE)
   "@B" != $(@:b)
   "@D" != $(@:d)
   "@F" != $(@:f)
   "*B" != $(*:b)
   "*D" != $(*:d)
   "*F" != $(*:f)
   "<B" != $(<:b)
   "<D" != $(<:d)
   "<F" != $(<:f)
   "?B" != $(?:b)
   "?F" != $(?:f)
   "?D" != $(?:d)
.ENDIF

# Directory caching configuration.
  .DIRCACHE         *:= yes
  .DIRCACHERESPCASE *:= yes

# Define the special NULL Prerequisite
NULLPRQ *:= __.NULLPRQ

# ---------- Default Construction Macro and Rule definitions --------------
# The construction rules may be customized further in subsequent recipes.mk
# files.
.IF $(__.DEFAULTS)
   # Primary suffixes in common use
   A          *:= .a		# Libraries
   E          *:= 		# Executables
   F          *:= .f		# Fortran
   O          *:= .o		# Objects
   P          *:= .p		# Pascal
   S          *:= .s		# Assembler sources
   V          *:= ,v		# RCS suffix
   YTAB	      *:= y.tab		# name-stem for yacc output files.
   LEXYY      *:= lex.yy	# lex output file

   # Standard C-language command names and flags
   CPP	      *:= $/lib$/cpp	# C-preprocessor
   CC         *:= cc		# C   compiler 
   CFLAGS     *=		# C   compiler flags
   "C++"      *:= CC		# C++ Compiler
   "C++FLAGS" *=                # C++ Compiler flags

   AS         *:= as		# Assembler and flags
   ASFLAGS    *= 

   LD         *= $(CC)		# Loader and flags
   LDFLAGS    *=
   LDLIBS     *=		# Default libraries

   AR         *:= ar		# archiver
   ARFLAGS    *= -rv

   # Definition of Print command for this system.
   PRINT      *= lp

   # Language and Parser generation Tools and their flags
   YACC	      *:= yacc		# standard yacc
   YFLAGS     *=
   LEX	      *:= lex		# standard lex
   LFLAGS     *=

   # Other Compilers, Tools and their flags
   PC	      *:= pc		# pascal compiler
   RC	      *:= f77		# ratfor compiler
   FC	      *:= f77		# fortran compiler
   MV	      *:= $/bin$/mv     # File rename command

   # Implicit generation rules for making inferences.
   # lex and yacc rules
   %.c : %.y %.Y
    $(YACC) $(YFLAGS) $<
    $(MV) $(YTAB).c $@

   %.c : %.l %.L
    $(LEX) $(LFLAGS) $<
    $(MV) $(LEXYY).c $@

   # Rules for making *$O
   %$O : %.c ; $(CC) $(CFLAGS) -c $<
   %$O : %$P ; $(PC) $(PFLAGS) -c $<
   %$O : %$S ; $(AS) $(ASFLAGS) -o $@ $<
   %$O : %.cl ; class -c $<
   %$O :| %.e %.r %.F %$F
    $(FC) $(RFLAGS) $(EFLAGS) $(FFLAGS) -c $<

   # Defibe how to build simple executables
   .IF $(__.EXECS)
      %$E : %$O ; $(CC) $(LDFLAGS) -o $@ $< $(LDLIBS)
   .ENDIF

   # Recipe to make archive files, defined only if we have
   # an archiver defined.
   .IF $(AR)
      %$A .SWAP .GROUP :
    $(AR) $(ARFLAGS) $@ $?
    $(RM) $(RMFLAGS) $?
   .ENDIF

   # RCS support
   .IF $(__.HAVE_RCS)
      CO      *:= co		# check out for RCS
      COFLAGS !+= -q

      % : $$(@:d)RCS$$/$$(@:f)$V
    -$(CO) $(COFLAGS) $(null,$(@:d) $@ $(<:d:s/RCS/)$@)
      .NOINFER : $$(@:d)RCS$$/$$(@:f)$V

      .IF $V
         % : %$V
        -$(CO) $(COFLAGS) $(null,$(@:d) $@ $(<:d:s/RCS/)$@)
         .NOINFER : %$V
      .ENDIF
   .END

   # SCCS support
   .IF $(__.HAVE_SCCS)
      GET     *:= get
      GFLAGS  !+=

      % : "$$(null,$$(@:d) s.$$@ $$(@:d)s.$$(@:f))"
    -$(GET) $(GFLAGS) $@
      .NOINFER : "$$(null,$$(@:d) s.$$@ $$(@:d)s.$$(@:f))"
   .END

   # Customize default recipe definitions for OS, OSRELEASE, etc. settings.
   .INCLUDE .NOINFER .IGNORE: $(INCFILENAME:d)$(OS)$/recipes.mk
.ENDIF


# Finally, define the default construction strategy
.ROOT .PHONY .NOSTATE .SEQUENTIAL :- .INIT .TARGETS .DONE;
.INIT .DONE .PHONY: $(NULLPRQ);

# Define the NULL Prerequisite as having no recipe.
$(NULLPRQ) .PHONY :;

# Reset warnings back to previous setting.
.SILENT !:= $(__.silent)

# Check for a Local project file, gets parsed before user makefile.
.INCLUDE .IGNORE .NOINFER: "project.mk"
