PRJ=..$/..$/..
PRJNAME=ucb
# Version
UCPGVFS_MAJOR=1
TARGET=ucpgvfs
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE


.INCLUDE: settings.mk

.IF "$(ENABLE_GNOMEVFS)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gnome-vfs-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS:s/ -llinc//:s/ -lbonobo-activation//:s/ -lgconf-2//:s/ -lORBit-2//:s/ -lIDL-2//:s/ -lgmodule-2.0//:s/ -lgobject-2.0//:s/ -lgthread-2.0//)
.ENDIF          # "$(OS)" == "LINUX"

# no "lib" prefix
DLLPRE =

SLOFILES=\
    $(SLO)$/content.obj	\
    $(SLO)$/directory.obj	\
    $(SLO)$/stream.obj	\
    $(SLO)$/provider.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)$(UCPGVFS_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(VOSLIB) \
    $(UCBHELPERLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)

SHL1STDLIBS+=$(PKGCONFIG_LIBS)

.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""

.INCLUDE: target.mk

