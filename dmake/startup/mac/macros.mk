# Define MPW MAC specific macros.
# Assumes CodeWarrior for Mac 5.0 C, change as needed.
#

A            *:= .lib
S	         *:= .s
V            *:= v
TMPDIR       *:= $(TempFolder)

# import library definitions
.IMPORT .IGNORE : CLibraries Libraries

# Set arguments for the SHELL.  Since we can't execute sub-processes,
# these variables are not important, except for some makefiles that check
# for some values to determine the platform.
SHELL       *:= "{MPW}MPW Shell"
SHELLFLAGS  *:=
GROUPFLAGS  *:=
SHELLMETAS  *:=

# Define toolkit macros
CC          *:= MWCPPC
AS          *:= PPCAsm
LD          *:= MWLinkPPC
AR          *:=
ARFLAGS     *:=
RM          *:= delete
RMFLAGS     *:= 
MV          *:= rename
YTAB        *:=
LEXYY       *:=

LDLIBS      *=  "{SharedLibraries}StdCLib" "{SharedLibraries}InterfaceLib" \
                "{PPCLibraries}StdCRuntime.o" "{PPCLibraries}PPCCRuntime.o" \
                "{Libraries}MathLib.o" "{PPCLibraries}PPCToolLibs.o"

# Disable the print command
PRINT       *=

# Make certain to disable defining how to make executables.
__.EXECS !:=
