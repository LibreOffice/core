# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL1TARGETN)"!="" && "$(SHL1CPPUNITTEST)"!="FALSE"
ALLTAR : test1
test1 .PHONY : $(SHL1TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#1 on library $(SHL1TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL1TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL2TARGETN)"!="" && "$(SHL2CPPUNITTEST)"!="FALSE"
ALLTAR : test2
test2 .PHONY : $(SHL2TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#2 on library $(SHL2TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL2TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL3TARGETN)"!="" && "$(SHL3CPPUNITTEST)"!="FALSE"
ALLTAR : test3
test3 .PHONY : $(SHL3TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#3 on library $(SHL3TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL3TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL4TARGETN)"!="" && "$(SHL4CPPUNITTEST)"!="FALSE"
ALLTAR : test4
test4 .PHONY : $(SHL4TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#4 on library $(SHL4TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL4TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL5TARGETN)"!="" && "$(SHL5CPPUNITTEST)"!="FALSE"
ALLTAR : test5
test5 .PHONY : $(SHL5TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#5 on library $(SHL5TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL5TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL6TARGETN)"!="" && "$(SHL6CPPUNITTEST)"!="FALSE"
ALLTAR : test6
test6 .PHONY : $(SHL6TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#6 on library $(SHL6TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL6TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL7TARGETN)"!="" && "$(SHL7CPPUNITTEST)"!="FALSE"
ALLTAR : test7
test7 .PHONY : $(SHL7TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#7 on library $(SHL7TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL7TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL8TARGETN)"!="" && "$(SHL8CPPUNITTEST)"!="FALSE"
ALLTAR : test8
test8 .PHONY : $(SHL8TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#8 on library $(SHL8TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL8TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL9TARGETN)"!="" && "$(SHL9CPPUNITTEST)"!="FALSE"
ALLTAR : test9
test9 .PHONY : $(SHL9TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#9 on library $(SHL9TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL9TARGETN)
.ENDIF

# unroll begin
# ---- create the test libraries ------------------------------------

.IF "$(SHL10TARGETN)"!="" && "$(SHL10CPPUNITTEST)"!="FALSE"
ALLTAR : test10
test10 .PHONY : $(SHL10TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test \#10 on library $(SHL10TARGETN)
        @echo ----------------------------------------------------------
        $(CPPUNITTESTER) $(SHL10TARGETN)
.ENDIF

