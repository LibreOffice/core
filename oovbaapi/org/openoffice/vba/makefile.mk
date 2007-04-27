PRJ=..$/..$/..

PRJNAME=oovbapi

TARGET=vba
PACKAGE=org$/openoffice$/vba

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(ENABLE_VBA)"!="YES"
dummy:
        @echo "not building vba..."
.ENDIF

# ------------------------------------------------------------------------

IDLFILES=\
    VbMsgBoxResult.idl \
    VbMsgBoxStyle.idl \
    XErrObject.idl \
    XGlobals.idl \
    XCollection.idl\
    XVBAToOOEventDescGen.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
