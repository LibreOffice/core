.IF "$(TESTOPT)"==""
    TESTOPT="-onlyerrors"
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST1LIB=$(SHL1TARGETN)

.IF "$(SHL1TARGET)"!=""
test : test1
test1: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#1 on library $(TEST1LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST1LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST2LIB=$(SHL2TARGETN)

.IF "$(SHL2TARGET)"!=""
test : test2
test2: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#2 on library $(TEST2LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST2LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST3LIB=$(SHL3TARGETN)

.IF "$(SHL3TARGET)"!=""
test : test3
test3: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#3 on library $(TEST3LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST3LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST4LIB=$(SHL4TARGETN)

.IF "$(SHL4TARGET)"!=""
test : test4
test4: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#4 on library $(TEST4LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST4LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST5LIB=$(SHL5TARGETN)

.IF "$(SHL5TARGET)"!=""
test : test5
test5: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#5 on library $(TEST5LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST5LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST6LIB=$(SHL6TARGETN)

.IF "$(SHL6TARGET)"!=""
test : test6
test6: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#6 on library $(TEST6LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST6LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST7LIB=$(SHL7TARGETN)

.IF "$(SHL7TARGET)"!=""
test : test7
test7: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#7 on library $(TEST7LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST7LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST8LIB=$(SHL8TARGETN)

.IF "$(SHL8TARGET)"!=""
test : test8
test8: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#8 on library $(TEST8LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST8LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST9LIB=$(SHL9TARGETN)

.IF "$(SHL9TARGET)"!=""
test : test9
test9: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#9 on library $(TEST9LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST9LIB) $(TESTOPT)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

TEST10LIB=$(SHL10TARGETN)

.IF "$(SHL10TARGET)"!=""
test : test10
test10: ALLTAR
        @+echo ----------------------------------------------------------
        @+echo - start cppunit test \#10 on library $(TEST10LIB)
        @+echo ----------------------------------------------------------
        testshl2 $(TEST10LIB) $(TESTOPT)
.ENDIF

