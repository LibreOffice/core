# unroll begin

.IF "$(SCP1TARGETN)"!=""

ALLSCP1FILES=$(foreach,i,$(SCP1FILES) $(foreach,j,$(SCP1LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP1TARGETN): $(ALLSCP1FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP1FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP2TARGETN)"!=""

ALLSCP2FILES=$(foreach,i,$(SCP2FILES) $(foreach,j,$(SCP2LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP2TARGETN): $(ALLSCP2FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP2FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP3TARGETN)"!=""

ALLSCP3FILES=$(foreach,i,$(SCP3FILES) $(foreach,j,$(SCP3LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP3TARGETN): $(ALLSCP3FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP3FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP4TARGETN)"!=""

ALLSCP4FILES=$(foreach,i,$(SCP4FILES) $(foreach,j,$(SCP4LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP4TARGETN): $(ALLSCP4FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP4FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP5TARGETN)"!=""

ALLSCP5FILES=$(foreach,i,$(SCP5FILES) $(foreach,j,$(SCP5LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP5TARGETN): $(ALLSCP5FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP5FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP6TARGETN)"!=""

ALLSCP6FILES=$(foreach,i,$(SCP6FILES) $(foreach,j,$(SCP6LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP6TARGETN): $(ALLSCP6FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP6FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP7TARGETN)"!=""

ALLSCP7FILES=$(foreach,i,$(SCP7FILES) $(foreach,j,$(SCP7LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP7TARGETN): $(ALLSCP7FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP7FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP8TARGETN)"!=""

ALLSCP8FILES=$(foreach,i,$(SCP8FILES) $(foreach,j,$(SCP8LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP8TARGETN): $(ALLSCP8FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP8FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP9TARGETN)"!=""

ALLSCP9FILES=$(foreach,i,$(SCP9FILES) $(foreach,j,$(SCP9LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP9TARGETN): $(ALLSCP9FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP9FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(SCP10TARGETN)"!=""

ALLSCP10FILES=$(foreach,i,$(SCP10FILES) $(foreach,j,$(SCP10LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP10TARGETN): $(ALLSCP10FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP10FILES) $(PAR)$/{$(subst,$(@:d:d:d), $(@:d:d))}$/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
#next Target
