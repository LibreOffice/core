# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := test_climaker
PACKAGE = cliversion

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = sandbox.jar ridl.jar unoil.jar jurt.jar juh.jar jut.jar java_uno.jar OOoRunner.jar
JAVAFILES = VersionTestCase.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

CSCFLAGS = -incr
.IF "$(debug)" != ""
CSCFLAGS += -checked+ -define:DEBUG -define:TRACE -debug+
.ELSE
CSCFLAGS += -optimize+
.ENDIF



OUTDIR=$(BIN)$/qa$/versioning
EXETARGET2=$(OUTDIR)$/runtests.exe 

.IF "$(name)" != ""
TESTLIB=$(OUTDIR)$/$(name)
.ELSE
TESTLIB=$(OUTDIR)$/version_current.dll
.ENDIF
VERSIONLIBS=version_libs

.IF "$(GUI)"=="WNT"
ALLTAR: $(EXETARGET2) \
    $(TESTLIB) \
    COPYVERSIONLIBS \
    RUNINSTRUCTIONS
.ELSE
ALLTAR:
.ENDIF


MAKEOUTDIR:
     $(MKDIR) $(OUTDIR)

COPYVERSIONLIBS: MAKEOUTDIR
        -$(GNUCOPY) -p $(VERSIONLIBS)$/* $(OUTDIR)

CSFILES2 = runtests.cs
$(EXETARGET2): $(CSFILES2) MAKEOUTDIR
    +$(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET2) \
        $(CSFILES2)


CSFILESLIB = version.cs
$(TESTLIB): $(CSFILESLIB) MAKEOUTDIR
    +$(CSC) $(CSCFLAGS) -target:library -out:$(TESTLIB) \
        -reference:$(BIN)$/cli_ure.dll \
         -reference:$(BIN)$/cli_types.dll \
         -reference:$(BIN)$/cli_basetypes.dll \
        -reference:$(BIN)$/cli_cppuhelper.dll \
        $(CSFILESLIB)



#-----------------------------------------------------------------------------
CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF


.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk


RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo "To run the test you have to provide the path to the  office location."
    @echo Example:
    @echo dmake run office="d:\myOffice"
    @echo .
    @echo "To build a test library with a particular name run"
    @echo "dmake name=name_of_library.dll"	
    @echo ###########################   N O T E  ######################################
    @echo .
    @echo .	

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN:
    +java -cp $(CLASSPATH) -Dcli_test_program=$(EXETARGET2) -Duno_path=$(office)\program $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST)

run: RUN

