# unroll begin
# ---- create the test libraries ------------------------------------

TEST1LIB=$(SHL1TARGETN)
.IF "$(TESTOPT)"==""
    TEST1OPT="-jobexclude"
    TEST1OPT+=$(SHL1TARGET).xsce
    TEST1OPT+="-sf"
    TEST1OPT+=$(mktmp "foo")
    TEST1OPT+="-onlyerrors"
    TEST1OPT+=" "
    TEST1OPT+=$(TESTOPTADD)
.ELSE
    TEST1OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL1TARGET)"!=""
test : test1
test1: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#1 on library $(TEST1LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST1LIB) $(TEST1OPT)
.ELSE
        testshl2 $(TEST1LIB) $(TEST1OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST2LIB=$(SHL2TARGETN)
.IF "$(TESTOPT)"==""
    TEST2OPT="-jobexclude"
    TEST2OPT+=$(SHL2TARGET).xsce
    TEST2OPT+="-sf"
    TEST2OPT+=$(mktmp "foo")
    TEST2OPT+="-onlyerrors"
    TEST2OPT+=" "
    TEST2OPT+=$(TESTOPTADD)
.ELSE
    TEST2OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL2TARGET)"!=""
test : test2
test2: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#2 on library $(TEST2LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST2LIB) $(TEST2OPT)
.ELSE
        testshl2 $(TEST2LIB) $(TEST2OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST3LIB=$(SHL3TARGETN)
.IF "$(TESTOPT)"==""
    TEST3OPT="-jobexclude"
    TEST3OPT+=$(SHL3TARGET).xsce
    TEST3OPT+="-sf"
    TEST3OPT+=$(mktmp "foo")
    TEST3OPT+="-onlyerrors"
    TEST3OPT+=" "
    TEST3OPT+=$(TESTOPTADD)
.ELSE
    TEST3OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL3TARGET)"!=""
test : test3
test3: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#3 on library $(TEST3LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST3LIB) $(TEST3OPT)
.ELSE
        testshl2 $(TEST3LIB) $(TEST3OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST4LIB=$(SHL4TARGETN)
.IF "$(TESTOPT)"==""
    TEST4OPT="-jobexclude"
    TEST4OPT+=$(SHL4TARGET).xsce
    TEST4OPT+="-sf"
    TEST4OPT+=$(mktmp "foo")
    TEST4OPT+="-onlyerrors"
    TEST4OPT+=" "
    TEST4OPT+=$(TESTOPTADD)
.ELSE
    TEST4OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL4TARGET)"!=""
test : test4
test4: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#4 on library $(TEST4LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST4LIB) $(TEST4OPT)
.ELSE
        testshl2 $(TEST4LIB) $(TEST4OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST5LIB=$(SHL5TARGETN)
.IF "$(TESTOPT)"==""
    TEST5OPT="-jobexclude"
    TEST5OPT+=$(SHL5TARGET).xsce
    TEST5OPT+="-sf"
    TEST5OPT+=$(mktmp "foo")
    TEST5OPT+="-onlyerrors"
    TEST5OPT+=" "
    TEST5OPT+=$(TESTOPTADD)
.ELSE
    TEST5OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL5TARGET)"!=""
test : test5
test5: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#5 on library $(TEST5LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST5LIB) $(TEST5OPT)
.ELSE
        testshl2 $(TEST5LIB) $(TEST5OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST6LIB=$(SHL6TARGETN)
.IF "$(TESTOPT)"==""
    TEST6OPT="-jobexclude"
    TEST6OPT+=$(SHL6TARGET).xsce
    TEST6OPT+="-sf"
    TEST6OPT+=$(mktmp "foo")
    TEST6OPT+="-onlyerrors"
    TEST6OPT+=" "
    TEST6OPT+=$(TESTOPTADD)
.ELSE
    TEST6OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL6TARGET)"!=""
test : test6
test6: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#6 on library $(TEST6LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST6LIB) $(TEST6OPT)
.ELSE
        testshl2 $(TEST6LIB) $(TEST6OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST7LIB=$(SHL7TARGETN)
.IF "$(TESTOPT)"==""
    TEST7OPT="-jobexclude"
    TEST7OPT+=$(SHL7TARGET).xsce
    TEST7OPT+="-sf"
    TEST7OPT+=$(mktmp "foo")
    TEST7OPT+="-onlyerrors"
    TEST7OPT+=" "
    TEST7OPT+=$(TESTOPTADD)
.ELSE
    TEST7OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL7TARGET)"!=""
test : test7
test7: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#7 on library $(TEST7LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST7LIB) $(TEST7OPT)
.ELSE
        testshl2 $(TEST7LIB) $(TEST7OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST8LIB=$(SHL8TARGETN)
.IF "$(TESTOPT)"==""
    TEST8OPT="-jobexclude"
    TEST8OPT+=$(SHL8TARGET).xsce
    TEST8OPT+="-sf"
    TEST8OPT+=$(mktmp "foo")
    TEST8OPT+="-onlyerrors"
    TEST8OPT+=" "
    TEST8OPT+=$(TESTOPTADD)
.ELSE
    TEST8OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL8TARGET)"!=""
test : test8
test8: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#8 on library $(TEST8LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST8LIB) $(TEST8OPT)
.ELSE
        testshl2 $(TEST8LIB) $(TEST8OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST9LIB=$(SHL9TARGETN)
.IF "$(TESTOPT)"==""
    TEST9OPT="-jobexclude"
    TEST9OPT+=$(SHL9TARGET).xsce
    TEST9OPT+="-sf"
    TEST9OPT+=$(mktmp "foo")
    TEST9OPT+="-onlyerrors"
    TEST9OPT+=" "
    TEST9OPT+=$(TESTOPTADD)
.ELSE
    TEST9OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL9TARGET)"!=""
test : test9
test9: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#9 on library $(TEST9LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST9LIB) $(TEST9OPT)
.ELSE
        testshl2 $(TEST9LIB) $(TEST9OPT)
.ENDIF
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST10LIB=$(SHL10TARGETN)
.IF "$(TESTOPT)"==""
    TEST10OPT="-jobexclude"
    TEST10OPT+=$(SHL10TARGET).xsce
    TEST10OPT+="-sf"
    TEST10OPT+=$(mktmp "foo")
    TEST10OPT+="-onlyerrors"
    TEST10OPT+=" "
    TEST10OPT+=$(TESTOPTADD)
.ELSE
    TEST10OPT=$(TESTOPT)
.ENDIF

.IF "$(SHL10TARGET)"!=""
test : test10
test10: ALLTAR
        @echo ----------------------------------------------------------
        @echo - start unit test \#10 on library $(TEST10LIB)
        @echo ----------------------------------------------------------
.IF "$(OS)" == "LINUX" # full path needed by osl_getModuleURLFromAddress:
        `which testshl2` $(TEST10LIB) $(TEST10OPT)
.ELSE
        testshl2 $(TEST10LIB) $(TEST10OPT)
.ENDIF
.ENDIF

