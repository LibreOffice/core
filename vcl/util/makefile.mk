#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..

PRJNAME=vcl
TARGET=vcl
TARGETTYPE=GUI
USE_DEFFILE=TRUE
GEN_HID_OTHER=TRUE

.IF "$(SNDFILE_LIBS)"!=""
SNDFILELIB=$(SNDFILE_LIBS)
.ENDIF

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  makefile.pmk
.INCLUDE :  makefile2.pmk

.IF "$(OS)" == "SOLARIS"
.IF "$(CPUNAME)" == "SPARC" && "$(CPU)" == "U"
LINKFLAGSRUNPATH_OOO := -R/usr/sfw/lib/64 $(LINKFLAGSRUNPATH_OOO)
.ELSE
LINKFLAGSRUNPATH_OOO := -R/usr/sfw/lib $(LINKFLAGSRUNPATH_OOO)
.ENDIF
.ENDIF

# --- Allgemein ----------------------------------------------------------

HXXDEPNLST= $(INC)$/vcl$/accel.hxx       \
            $(INC)$/vcl$/animate.hxx     \
            $(INC)$/vcl$/apptypes.hxx    \
            $(INC)$/vcl$/bitmap.hxx      \
            $(INC)$/vcl$/bitmapex.hxx    \
            $(INC)$/vcl$/bmpacc.hxx      \
            $(INC)$/vcl$/btndlg.hxx      \
            $(INC)$/vcl$/button.hxx      \
            $(INC)$/vcl$/ctrl.hxx        \
            $(INC)$/vcl$/cursor.hxx      \
            $(INC)$/vcl$/cmdevt.hxx      \
            $(INC)$/vcl$/decoview.hxx    \
            $(INC)$/vcl$/dialog.hxx      \
            $(INC)$/vcl$/dllapi.h        \
            $(INC)$/vcl$/dockwin.hxx     \
            $(INC)$/vcl$/edit.hxx        \
            $(INC)$/vcl$/event.hxx       \
            $(INC)$/vcl$/field.hxx       \
            $(INC)$/vcl$/fixed.hxx       \
            $(INC)$/vcl$/floatwin.hxx    \
            $(INC)$/vcl$/font.hxx        \
            $(INC)$/vcl$/floatwin.hxx    \
            $(INC)$/vcl$/graph.hxx       \
            $(INC)$/vcl$/group.hxx       \
            $(INC)$/vcl$/help.hxx        \
            $(INC)$/vcl$/jobset.hxx      \
            $(INC)$/vcl$/keycodes.hxx    \
            $(INC)$/vcl$/keycod.hxx      \
            $(INC)$/vcl$/image.hxx       \
            $(INC)$/vcl$/lstbox.h             \
            $(INC)$/vcl$/lstbox.hxx      \
            $(INC)$/vcl$/mapmod.hxx      \
            $(INC)$/vcl$/metaact.hxx     \
            $(INC)$/vcl$/menu.hxx        \
            $(INC)$/vcl$/menubtn.hxx     \
            $(INC)$/vcl$/metric.hxx      \
            $(INC)$/vcl$/morebtn.hxx     \
            $(INC)$/vcl$/msgbox.hxx      \
            $(INC)$/vcl$/octree.hxx      \
            $(INC)$/vcl$/outdev.hxx      \
            $(INC)$/vcl$/pointr.hxx      \
            $(INC)$/vcl$/ptrstyle.hxx    \
            $(INC)$/vcl$/prntypes.hxx    \
            $(INC)$/vcl$/print.hxx       \
            $(INC)$/vcl$/prndlg.hxx      \
            $(INC)$/vcl$/region.hxx      \
            $(INC)$/vcl$/rendergraphic.hxx		\
            $(INC)$/vcl$/rendergraphicrasterizer.hxx	\
            $(INC)$/vcl$/salbtype.hxx    \
            $(INC)$/vcl$/scrbar.hxx      \
            $(INC)$/vcl$/slider.hxx      \
            $(INC)$/vcl$/seleng.hxx      \
            $(INC)$/vcl$/settings.hxx    \
            $(INC)$/vcl$/sound.hxx       \
            $(INC)$/vcl$/sndstyle.hxx    \
            $(INC)$/vcl$/split.hxx       \
            $(INC)$/vcl$/splitwin.hxx    \
            $(INC)$/vcl$/spin.hxx        \
            $(INC)$/vcl$/spinfld.hxx     \
            $(INC)$/vcl$/status.hxx      \
            $(INC)$/vcl$/stdtext.hxx     \
            $(INC)$/vcl$/sv.h            \
            $(INC)$/vcl$/svapp.hxx       \
            $(INC)$/vcl$/syschild.hxx    \
            $(INC)$/vcl$/sysdata.hxx     \
            $(INC)$/vcl$/syswin.hxx      \
            $(INC)$/vcl$/tabctrl.hxx     \
            $(INC)$/vcl$/tabdlg.hxx      \
            $(INC)$/vcl$/tabpage.hxx     \
            $(INC)$/vcl$/toolbox.hxx     \
            $(INC)$/vcl$/timer.hxx       \
            $(INC)$/vcl$/virdev.hxx      \
            $(INC)$/vcl$/wall.hxx        \
            $(INC)$/vcl$/waitobj.hxx     \
            $(INC)$/vcl$/window.hxx      \
            $(INC)$/vcl$/wrkwin.hxx

.IF "$(linkinc)" != ""
SHL11FILE=  $(MISC)$/app.slo
SHL12FILE=  $(MISC)$/gdi.slo
SHL13FILE=  $(MISC)$/win.slo
SHL14FILE=  $(MISC)$/ctrl.slo
#SHL15FILE=  $(MISC)$/ex.slo
SHL16FILE=  $(MISC)$/salapp.slo
SHL17FILE=  $(MISC)$/salwin.slo
SHL18FILE=  $(MISC)$/salgdi.slo
.ENDIF

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  $(SLB)$/app.lib         \
            $(SLB)$/gdi.lib         \
            $(SLB)$/win.lib         \
            $(SLB)$/ctrl.lib        \
            $(SLB)$/helper.lib	    \
            $(SLB)$/fontsubset.lib  \
            $(SLB)$/components.lib

.IF "$(GUI)" == "UNX" && "$(GUIBASE)"!="aqua"
LIB1FILES+= $(SLB)$/salplug.lib  \
            $(SLB)$/fontman.lib  \
            $(SLB)$/printer.lib
.ELSE
LIB1FILES+= \
            $(SLB)$/salwin.lib  \
            $(SLB)$/salgdi.lib  \
            $(SLB)$/salapp.lib
.IF "$(GUIBASE)" == "aqua"
LIB1FILES+= $(SLB)$/dtransaqua.lib
.ENDIF
.ENDIF

SHL1TARGET= vcl$(DLLPOSTFIX)
SHL1IMPLIB= ivcl
SHL1STDLIBS+=\
            $(SOTLIB)           \
            $(UNOTOOLSLIB)      \
            $(TOOLSLIB)         \
            $(I18NPAPERLIB)     \
            $(I18NISOLANGLIB)   \
            $(I18NUTILLIB)      \
            $(COMPHELPERLIB)	\
            $(UCBHELPERLIB)     \
            $(CPPUHELPERLIB)    \
            $(CPPULIB)          \
            $(VOSLIB)           \
            $(SALLIB)			\
            $(BASEGFXLIB)		\
            $(ICUUCLIB)			\
            $(ICUDATALIB)		\
            $(ICULELIB)			\
            $(JVMACCESSLIB)

.IF "$(GUI)" == "UNX"
.IF "$(ENABLE_GRAPHITE)" != ""
.IF "$(SYSTEM_GRAPHITE)" == "YES"
SHL1STDLIBS+= $(GRAPHITE_LIBS)
.ELSE
SHL1STDLIBS+= $(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT)/libgraphite.a
.ENDIF
.ENDIF
.ENDIF
SHL1USE_EXPORTS=name

.IF "$(GUIBASE)"=="aqua"
SHL1STDLIBS+= \
    $(BASEBMPLIB) \
    -lAppleRemote$(DLLPOSTFIX) \
    -framework QuickTime

LIB1FILES+= \
            $(SLB)$/sala11y.lib
.ENDIF

.IF "$(USE_BUILTIN_RASTERIZER)"!=""
    LIB1FILES +=    $(SLB)$/glyphs.lib
    SHL1STDLIBS+=   $(FREETYPELIB)
.ELSE
.IF "$(ENABLE_GRAPHITE)" == "TRUE"
    LIB1FILES +=    $(SLB)$/glyphs.lib
.ENDIF
.ENDIF # USE_BUILTIN_RASTERIZER

SHL1LIBS=   $(LIB1TARGET)
.IF "$(GUI)"!="UNX"
.IF "$(COM)"!="GCC"
#SHL1OBJS=   $(SLO)$/salshl.obj
.ENDIF
.ENDIF

.IF "$(GUI)" != "UNX"
SHL1RES=    $(RES)$/salsrc.res
.ENDIF

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =   $(HXXDEPNLST) \
                $(LIB1TARGET)
DEF1DES     =VCL
DEFLIB1NAME =vcl

# --- W32 ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

.IF "$(ENABLE_GRAPHITE)" == "TRUE"
.IF "$(COM)" == "GCC"
SHL1STDLIBS += -lgraphite
.ELSE
SHL1STDLIBS += graphite_dll.lib
.ENDIF
.ENDIF

SHL1STDLIBS += $(UWINAPILIB)      \
               $(GDI32LIB)        \
               $(GDIPLUSLIB)	  \
               $(MSIMG32LIB)      \
               $(WINSPOOLLIB)     \
               $(OLE32LIB)        \
               $(SHELL32LIB)      \
               $(ADVAPI32LIB)

SHL1STDLIBS += $(IMM32LIB)

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
LINKFLAGSSHL += /ENTRY:LibMain@12
.ENDIF
.ENDIF

# --- OS2 ----------------------------------------------------------------

.IF "$(GUI)" == "OS2"
STDSHL1 += ft2lib.lib
.ENDIF

# --- UNX ----------------------------------------------------------------

# UNX sal plugins
.IF "$(GUI)" == "UNX" && "$(GUIBASE)" != "aqua"

# desktop detector
LIB7TARGET=$(SLB)$/idet
LIB7FILES=$(SLB)$/dtdetect.lib
SHL7TARGET=desktop_detector$(DLLPOSTFIX)
SHL7STDLIBS=\
            $(SALLIB) \
            $(X11LINK_DYNAMIC)
SHL7IMPLIB=idet
SHL7LIBS=$(LIB7TARGET)

# basic pure X11 plugin
LIB2TARGET=$(SLB)$/ipure_x
LIB2FILES= \
            $(SLB)$/dtransX11.lib  \
            $(SLB)$/printergfx.lib  \
            $(SLB)$/salwin.lib  \
            $(SLB)$/salgdi.lib  \
            $(SLB)$/salapp.lib
SHL2TARGET=vclplug_gen$(DLLPOSTFIX)
SHL2IMPLIB=ipure_x
SHL2LIBS=$(LIB2TARGET)
SHL2DEPN=$(SHL1IMPLIBN) $(SHL1TARGETN)

# libs for generic plugin
SHL2STDLIBS=\
            $(VCLLIB)\
            $(I18NPAPERLIB)     \
            $(I18NISOLANGLIB)     \
            $(TOOLSLIB)         \
            $(VOSLIB)           \
            $(BASEGFXLIB)	\
            $(UNOTOOLSLIB) \
            $(COMPHELPERLIB)	\
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

# prepare linking of Xinerama
.IF "$(USE_XINERAMA)" != "NO"

.IF "$(OS)"=="MACOSX" || "$(OS)$(CPU)" == "LINUXX"
XINERAMALIBS=-lXinerama
.ELSE
.IF "$(OS)" != "SOLARIS" || "$(USE_XINERAMA_VERSION)" == "Xorg"
.IF "$(XINERAMA_LINK)" == "dynamic"
XINERAMALIBS= -lXinerama
.ELSE
XINERAMALIBS= -Wl,-Bstatic -lXinerama -Wl,-Bdynamic 
.ENDIF # XINERAMA_LINK == dynamic
.ENDIF # OS == SOLARIS
.ENDIF # OS == MACOSX

SHL2STDLIBS += $(XINERAMALIBS)
.ENDIF # USE_XINERAMA != NO

.IF "$(XRENDER_LINK)" == "YES"
SHL2STDLIBS+=`pkg-config --libs xrender`
.ENDIF

.IF "$(GUIBASE)"=="unx"

SHL2STDLIBS += -lXext -lSM -lICE -lX11
.IF "$(OS)"!="MACOSX" && "$(OS)"!="FREEBSD" && "$(OS)"!="NETBSD"
# needed by salprnpsp.cxx
SHL2STDLIBS+= -ldl
.ENDIF

.IF "$(ENABLE_RANDR)" != ""
.IF "$(XRANDR_DLOPEN)" == "FALSE"
SHL2STDLIBS+= $(XRANDR_LIBS)
.ENDIF
.ENDIF

.ENDIF          # "$(GUIBASE)"=="unx"

# gtk plugin
.IF "$(ENABLE_GTK)" != ""
PKGCONFIG_MODULES=gtk+-2.0 gthread-2.0
.IF "$(ENABLE_DBUS)" != ""
PKGCONFIG_MODULES+= dbus-glib-1
.ENDIF
.INCLUDE: pkg_config.mk

LIB4TARGET=$(SLB)$/igtk_plug_
LIB4FILES=\
            $(SLB)$/gtkapp.lib\
            $(SLB)$/gtka11y.lib \
            $(SLB)$/gtkgdi.lib\
            $(SLB)$/gtkwin.lib

SHL4TARGET=vclplug_gtk$(DLLPOSTFIX)
SHL4IMPLIB=igtk_plug_
SHL4LIBS=$(LIB4TARGET)
SHL4DEPN=$(SHL1IMPLIBN) $(SHL1TARGETN) $(SHL2IMPLIBN) $(SHL2TARGETN)
# libs for gtk plugin
SHL4STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
SHL4SONAME+=-z nodefs
SHL4NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""


SHL4STDLIBS+=-l$(SHL2TARGET)
SHL4STDLIBS+=\
            $(VCLLIB)		\
            $(TOOLSLIB)         \
            $(CPPUHELPERLIB)    \
            $(CPPULIB)          \
            $(VOSLIB)           \
            $(SALLIB)           \
            $(X11LINK_DYNAMIC)

.IF "$(ENABLE_RANDR)" != ""
.IF "$(XRANDR_DLOPEN)" == "FALSE"
SHL4STDLIBS+= $(XRANDR_LIBS)
.ENDIF
.ENDIF

.ENDIF # "$(ENABLE_GTK)" != ""

# KDE plugin
.IF "$(ENABLE_KDE)" != ""
.IF "$(KDE_ROOT)"!=""
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib
.IF "$(OS)$(CPU)" == "LINUXX"
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib64
.ENDIF
.ENDIF
LIB5TARGET=$(SLB)$/ikde_plug_
LIB5FILES=$(SLB)$/kdeplug.lib
SHL5TARGET=vclplug_kde$(DLLPOSTFIX)
SHL5IMPLIB=ikde_plug_
SHL5LIBS=$(LIB5TARGET)
SHL5DEPN=$(SHL2TARGETN)
# libs for KDE plugin
SHL5LINKFLAGS+=$(KDE_LIBS)
SHL5STDLIBS+=-l$(SHL2TARGET)
SHL5STDLIBS+=\
        $(VCLLIB)       \
        $(TOOLSLIB)     \
        $(VOSLIB)       \
        $(SALLIB)       \
        $(X11LINK_DYNAMIC)

.IF "$(ENABLE_RANDR)" != ""
.IF "$(XRANDR_DLOPEN)" == "FALSE"
SHL5STDLIBS+= $(XRANDR_LIBS)
.ENDIF
.ENDIF

.ENDIF # "$(ENABLE_KDE)" != ""

# KDE4 plugin
.IF "$(ENABLE_KDE4)" != ""
.IF "$(KDE4_ROOT)"!=""
EXTRALIBPATHS+=-L$(KDE4_ROOT)$/lib
.ENDIF
LIB6TARGET=$(SLB)$/ikde4_plug_
LIB6FILES=$(SLB)$/kde4plug.lib
SHL6TARGET=vclplug_kde4$(DLLPOSTFIX)
SHL6IMPLIB=ikde4_plug_
SHL6LIBS=$(LIB6TARGET)
SHL6DEPN=$(SHL2TARGETN)
# libs for KDE4 plugin
SHL6LINKFLAGS+=$(KDE4_LIBS)
SHL6STDLIBS+=-l$(SHL2TARGET)
SHL6STDLIBS+=\
        $(VCLLIB)       \
        $(PSPLIB)	\
        $(TOOLSLIB)     \
        $(VOSLIB)       \
        $(SALLIB)   \
        $(X11LINK_DYNAMIC)

.IF "$(ENABLE_RANDR)" != ""
.IF "$(XRANDR_DLOPEN)" == "FALSE"
SHL6STDLIBS+= $(XRANDR_LIBS)
.ENDIF
.ENDIF

.ENDIF # "$(ENABLE_KDE4)" != ""

.ENDIF # UNX

# --- Allgemein ----------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/vcl.component

.IF "$(OS)" == "MACOSX"
my_platform = .macosx
.ELIF "$(OS)" == "WNT"
my_platform = .windows
.END

$(MISC)/vcl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        vcl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt vcl$(my_platform).component
