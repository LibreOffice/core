# MSDOS DMAKE startup file.  Customize to suit your needs.
# Assumes MKS toolkit for the tool commands, and Zortech C.  Change as req'd.
# See the documentation for a description of internally defined macros.
#
# Disable warnings for macros redefined here that were given
# on the command line.
__.SILENT !:= $(.SILENT)
.SILENT   !:= yes

# Configuration parameters for DMAKE startup.mk file
# Set these to NON-NULL if you wish to turn the parameter on.
_HAVE_RCS	:= yes		# yes => RCS  is installed.
_HAVE_SCCS	:= 		# yes => SCCS is installed.

# Applicable suffix definitions
A := .lib	# Libraries
E := .exe	# Executables
F := .for	# Fortran
O := .obj	# Objects
P := .pas	# Pascal
S := .asm	# Assembler sources
V := 		# RCS suffix

# See if these are defined
TMPDIR := $(ROOTDIR)/tmp
.IMPORT .IGNORE : TMPDIR SHELL COMSPEC

# Recipe execution configurations
# First set SHELL, If it is not defined, use COMSPEC, otherwise
# it is assumed to be MKS Korn SHELL.
.IF $(SHELL) == $(NULL)
.IF $(COMSPEC) == $(NULL)
   SHELL := $(ROOTDIR)/bin/sh$E
.ELSE
   SHELL := $(COMSPEC)
.END
.END
GROUPSHELL := $(SHELL)

# Now set remaining arguments depending on which SHELL we
# are going to use.  COMSPEC (assumed to be command.com) or
# MKS Korn Shell.
.IF $(SHELL)==$(COMSPEC)
   SHELLFLAGS  := $(SWITCHAR)c
   GROUPFLAGS  := $(SHELLFLAGS)
   SHELLMETAS  := *"?<>
   GROUPSUFFIX := .bat
   DIRSEPSTR   := \\\
   DIVFILE      = $(TMPFILE:s,/,\)
.ELSE
   SHELLFLAGS  := -c
   GROUPFLAGS  := 
   SHELLMETAS  := *"?<>|()&][$$\#`'
   GROUPSUFFIX := .ksh
   .MKSARGS    := yes
   DIVFILE      = $(TMPFILE:s,/,${DIVSEP_shell_${USESHELL}})
   DIVSEP_shell_yes := \\\
   DIVSEP_shell_no  := \\
.END

# Standard C-language command names and flags
   CC      := ztc		# C-compiler and flags
   CFLAGS  +=

   AS      := masm		# Assembler and flags
   ASFLAGS +=

   LD       = blink		# Loader and flags
   LDFLAGS +=
   LDLIBS   =

# Definition of $(MAKE) macro for recursive makes.
   MAKE = $(MAKECMD) -S $(MFLAGS)

# Language and Parser generation Tools and their flags
   YACC	  := yacc		# standard yacc
   YFLAGS +=
   YTAB	  := ytab		# yacc output files name stem.

   LEX	  := lex		# standard lex
   LFLAGS +=
   LEXYY  := lex_yy		# lex output file

# Other Compilers, Tools and their flags
   PC	:= any_pc		# pascal compiler
   RC	:= anyf77		# ratfor compiler
   FC	:= anyf77		# fortran compiler

   CO	   := co		# check out for RCS
   COFLAGS += -q

   AR     := ar			# archiver
   ARFLAGS+= ruv

   RM	   := rm		# remove a file command
   RMFLAGS +=

# Implicit generation rules for making inferences.
# We don't provide .yr or .ye rules here.  They're obsolete.
# Rules for making *$O
   %$O : %.c ; $(CC) $(CFLAGS) -c $<
   %$O : %.cpp ; $(CC) $(CFLAGS) -c $<
   %$O : %$P ; $(PC) $(PFLAGS) -c $<
   %$O : %$S ; $(AS) $(ASFLAGS) $(<:s,/,\);
   %$O : %.cl ; class -c $<
   %$O :| %.e %.r %.F %$F ; $(FC) $(RFLAGS) $(EFLAGS) $(FFLAGS) -c $<

# Executables
   %$E : %$O ; $(CC) $(LDFLAGS) -o$@ $< $(LDLIBS)

# lex and yacc rules
   %.c : %.y ; $(YACC)  $(YFLAGS) $<; mv $(YTAB).c $@
   %.c : %.l ; $(LEX)   $(LFLAGS) $<; mv $(LEXYY).c $@

# RCS support
.IF $(_HAVE_RCS)
   % : $$(@:d)RCS$$(DIRSEPSTR)$$(@:f)$V;- $(CO) $(COFLAGS) $@
   .NOINFER : $$(@:d)RCS$$(DIRSEPSTR)$$(@:f)$V
.END

# SCCS support
.IF $(_HAVE_SCCS)
   % : s.% ; get $<
   .NOINFER : s.%
.END

# Recipe to make archive files.
%$A .SWAP .GROUP :
    $(AR) $(ARFLAGS) $@ $?
    $(RM) $(RMFLAGS) $?

# DMAKE uses this recipe to remove intermediate targets
.REMOVE :; $(RM) -f $<

# AUGMAKE extensions for SYSV compatibility
"@B" = $(@:b)
"@D" = $(@:d)
"@F" = $(@:f)
"*B" = $(*:b)
"*D" = $(*:d)
"*F" = $(*:f)
"<B" = $(<:b)
"<D" = $(<:d)
"<F" = $(<:f)
"?B" = $(?:b)
"?F" = $(?:f)
"?D" = $(?:d)

# Turn warnings back to previous setting.
.SILENT !:= $(__.SILENT)

# Local init file if any, gets parsed before user makefile
.INCLUDE .IGNORE: "_startup.mk"
