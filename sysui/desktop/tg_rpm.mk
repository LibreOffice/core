#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_rpm.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-10 14:57:56 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"

ALLTAR : $(RPMFILES)

$(RPMFILES) : $(SPECFILE)
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$(@:b:s/-/ /:1)-$(SPECFILE:b)-*
    $(RPM) -bb $< $(RPMMACROS) \
        --buildroot $(ABSLOCALOUT)$/misc$/$(@:b) \
        --define "_builddir $(COMMONMISC)$/$(@:b:s/-/ /:1)" \
        --define "productname $(PRODUCTNAME.$(@:b:s/-/ /:1))" \
        --define "pkgprefix $(@:b:s/-/ /:1)" \
        --define "unixfilename $(UNIXFILENAME.$(@:b:s/-/ /:1))" \
        --define "iconprefix $(ICONPREFIX.$(@:b:s/-/ /:1))" \
        --define "version $(PKGVERSION.$(@:b:s/-/ /:1))" \
        --define "release $(PKGREV)"
.ENDIF
