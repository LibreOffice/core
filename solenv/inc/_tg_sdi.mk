# unroll begin

.IF "$(HIDSID1PARTICLE)"!=""
$(HIDSID1PARTICLE): $(SDI1TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI1NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID1PARTICLE)"!=""

.IF "$(SDI1TARGET)"!=""
$(SDI1TARGET): $(SVSDI1DEPEND) $(SDI1NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI1NAME).hxx	\
    -fd$(INCCOMX)/$(SDI1NAME).ilb	\
    -fm$(MISC)/$(SDI1NAME).don	\
    -fl$(MISC)/$(SDI1NAME).lst         \
    -fx$(SDI1EXPORT).sdi		\
    -fz$(MISC)/$(SDI1NAME).sid	\
    $(SDI1NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI1TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID2PARTICLE)"!=""
$(HIDSID2PARTICLE): $(SDI2TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI2NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID2PARTICLE)"!=""

.IF "$(SDI2TARGET)"!=""
$(SDI2TARGET): $(SVSDI2DEPEND) $(SDI2NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI2NAME).hxx	\
    -fd$(INCCOMX)/$(SDI2NAME).ilb	\
    -fm$(MISC)/$(SDI2NAME).don	\
    -fl$(MISC)/$(SDI2NAME).lst         \
    -fx$(SDI2EXPORT).sdi		\
    -fz$(MISC)/$(SDI2NAME).sid	\
    $(SDI2NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI2TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID3PARTICLE)"!=""
$(HIDSID3PARTICLE): $(SDI3TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI3NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID3PARTICLE)"!=""

.IF "$(SDI3TARGET)"!=""
$(SDI3TARGET): $(SVSDI3DEPEND) $(SDI3NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI3NAME).hxx	\
    -fd$(INCCOMX)/$(SDI3NAME).ilb	\
    -fm$(MISC)/$(SDI3NAME).don	\
    -fl$(MISC)/$(SDI3NAME).lst         \
    -fx$(SDI3EXPORT).sdi		\
    -fz$(MISC)/$(SDI3NAME).sid	\
    $(SDI3NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI3TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID4PARTICLE)"!=""
$(HIDSID4PARTICLE): $(SDI4TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI4NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID4PARTICLE)"!=""

.IF "$(SDI4TARGET)"!=""
$(SDI4TARGET): $(SVSDI4DEPEND) $(SDI4NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI4NAME).hxx	\
    -fd$(INCCOMX)/$(SDI4NAME).ilb	\
    -fm$(MISC)/$(SDI4NAME).don	\
    -fl$(MISC)/$(SDI4NAME).lst         \
    -fx$(SDI4EXPORT).sdi		\
    -fz$(MISC)/$(SDI4NAME).sid	\
    $(SDI4NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI4TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID5PARTICLE)"!=""
$(HIDSID5PARTICLE): $(SDI5TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI5NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID5PARTICLE)"!=""

.IF "$(SDI5TARGET)"!=""
$(SDI5TARGET): $(SVSDI5DEPEND) $(SDI5NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI5NAME).hxx	\
    -fd$(INCCOMX)/$(SDI5NAME).ilb	\
    -fm$(MISC)/$(SDI5NAME).don	\
    -fl$(MISC)/$(SDI5NAME).lst         \
    -fx$(SDI5EXPORT).sdi		\
    -fz$(MISC)/$(SDI5NAME).sid	\
    $(SDI5NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI5TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID6PARTICLE)"!=""
$(HIDSID6PARTICLE): $(SDI6TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI6NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID6PARTICLE)"!=""

.IF "$(SDI6TARGET)"!=""
$(SDI6TARGET): $(SVSDI6DEPEND) $(SDI6NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI6NAME).hxx	\
    -fd$(INCCOMX)/$(SDI6NAME).ilb	\
    -fm$(MISC)/$(SDI6NAME).don	\
    -fl$(MISC)/$(SDI6NAME).lst         \
    -fx$(SDI6EXPORT).sdi		\
    -fz$(MISC)/$(SDI6NAME).sid	\
    $(SDI6NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI6TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID7PARTICLE)"!=""
$(HIDSID7PARTICLE): $(SDI7TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI7NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID7PARTICLE)"!=""

.IF "$(SDI7TARGET)"!=""
$(SDI7TARGET): $(SVSDI7DEPEND) $(SDI7NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI7NAME).hxx	\
    -fd$(INCCOMX)/$(SDI7NAME).ilb	\
    -fm$(MISC)/$(SDI7NAME).don	\
    -fl$(MISC)/$(SDI7NAME).lst         \
    -fx$(SDI7EXPORT).sdi		\
    -fz$(MISC)/$(SDI7NAME).sid	\
    $(SDI7NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI7TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID8PARTICLE)"!=""
$(HIDSID8PARTICLE): $(SDI8TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI8NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID8PARTICLE)"!=""

.IF "$(SDI8TARGET)"!=""
$(SDI8TARGET): $(SVSDI8DEPEND) $(SDI8NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI8NAME).hxx	\
    -fd$(INCCOMX)/$(SDI8NAME).ilb	\
    -fm$(MISC)/$(SDI8NAME).don	\
    -fl$(MISC)/$(SDI8NAME).lst         \
    -fx$(SDI8EXPORT).sdi		\
    -fz$(MISC)/$(SDI8NAME).sid	\
    $(SDI8NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI8TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID9PARTICLE)"!=""
$(HIDSID9PARTICLE): $(SDI9TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI9NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID9PARTICLE)"!=""

.IF "$(SDI9TARGET)"!=""
$(SDI9TARGET): $(SVSDI9DEPEND) $(SDI9NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI9NAME).hxx	\
    -fd$(INCCOMX)/$(SDI9NAME).ilb	\
    -fm$(MISC)/$(SDI9NAME).don	\
    -fl$(MISC)/$(SDI9NAME).lst         \
    -fx$(SDI9EXPORT).sdi		\
    -fz$(MISC)/$(SDI9NAME).sid	\
    $(SDI9NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI9TARGET)"!=""

# Instruction for ???
# unroll begin

.IF "$(HIDSID10PARTICLE)"!=""
$(HIDSID10PARTICLE): $(SDI10TARGET)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@.$(ROUT).tmp $@
    @$(TYPE) $(MISC)/$(SDI10NAME).sid | $(AWK) '$$1=="#define" { print $$2, $$3 }' > $@.$(ROUT).tmp
    @-$(RM) $@
    @$(RENAME) $@.$(ROUT).tmp $@
.ENDIF # "$(HIDSID10PARTICLE)"!=""

.IF "$(SDI10TARGET)"!=""
$(SDI10TARGET): $(SVSDI10DEPEND) $(SDI10NAME).sdi
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)/$(SDI10NAME).hxx	\
    -fd$(INCCOMX)/$(SDI10NAME).ilb	\
    -fm$(MISC)/$(SDI10NAME).don	\
    -fl$(MISC)/$(SDI10NAME).lst         \
    -fx$(SDI10EXPORT).sdi		\
    -fz$(MISC)/$(SDI10NAME).sid	\
    $(SDI10NAME).sdi -I$(MISC) -I$(SVSDIINC) -I$(INC) $(INCLUDE))
.ENDIF # "$(SDI10TARGET)"!=""

# Instruction for ???
