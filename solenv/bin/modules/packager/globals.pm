#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: globals.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:24:10 $
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

package packager::globals;

############################################
# Global settings
############################################

BEGIN
{
    $prog="packager";

    $packlistname = "pack.lst";
    $compiler = "";
    $ignoreerrors = 0;

    $logging = 0;
    $logfilename = "packager_logfile.log";  # the default logfile name for global errors
    @logfileinfo = ();

    $plat = $^O;

    if (( $plat =~ /MSWin/i ) || (( $plat =~ /cygwin/i ) && ( $ENV{'USE_SHELL'} eq "4nt" )))
    {
        $separator = "\\";
        $iswin = 1;
        $isunix = 0;
    }
    else
    {
        $separator = "/";
        $iswin = 0;
        $isunix = 1;
    }

}

1;
