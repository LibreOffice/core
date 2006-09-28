
PRJ=..
PRJNAME=eggtray
TARGET=eggtray


# --- Settings ----------------------------------

.INCLUDE : settings.mk
PKGCONFIG_MODULES=gtk+-2.0 gdk-2.0
.INCLUDE: pkg_config.mk
CFLAGS+=$(PKGCONFIG_CFLAGS)

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/eggtrayicon.obj

SHL1STDLIBS+=$(PKGCONFIG_LIBS) -lX11

SHL1TARGET= 	eggtray$(UPD)$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/eggtray.lib

# --- Targets ----------------------------------

.INCLUDE : target.mk
