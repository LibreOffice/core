#!/usr/solar/bin/perl -w

# ***********************************************************************
# Description:
# Filename:    osarch.pm
# Autor:       EG
# Date:        28.10.96
# $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/solenv/bin/modules/osarch.pm,v 1.1.1.1 2000-09-20 14:43:11 hr Exp $
# Copyright:   STAR DIVISION
# **********************************************************************/

package osarch;

$m_str = `uname -m`;
$s_str = `uname -s`;
chop( $m_str, $s_str );

%osTable    = ("SunOS",     "solaris",
               "HP-UX",     "hpux",
               "AIX",       "aix",
               "OS/390",    "s390",
               "Linux",     "linux",
               "FreeBSD",   "freebsd",
               "NetBSD",    "netbsd",
               "OSF1",      "decunix",
               "SCO_SV",    "scoosr5",
#              "SCO_SV",    "scouw21",
               "Darwin",    "macosxp",
               "Mac OS",    "macosxp",
               "SINIX-N",   "sinix",
               "IRIX",      "irix"
               );

%osDefTable = ("SunOS",     "-DSYSV -DSOLARIS",
               "HP-UX",     "-DSYSV -DHPUX",
               "AIX",       "-DSYSV -DAIX",
               "OS/390",    "-DS390 -DS390_ASCII",
               "Linux",     "-DLINUX",
               "FreeBSD",   "-DFREEBSD",
               "NetBSD",    "-DNETBSD",
               "OSF1",      "-DSYSV -DDECUNIX",
               "SCO_SV",    "-DSYSV -DSCO -DSCO_OSR5",
#              "SCO_SV",    "-DSYSV -DSCO -DSCO_UW21",
               "Darwin",    "-DBSD -DMACOSX",
               "Mac OS",    "-DBSD -DMACOSX",
               "SINIX-N",   "-DSYSV -DSINIX",
               "IRIX",      "-DSYSV -DIRIX"
               );

%osDosTable = ("SunOS",     "sol",
               "HP-UX", "hpx",
               "AIX",   "aix",
               "OS/390",    "mvs",
               "Linux", "lng",
               "FreeBSD",   "fbsd",
               "NetBSD",    "bsd",
               "SCO_SV",    "sco",
               "Darwin",    "macx",
               "Mac OS",    "macx",
               "SINIX-N",   "sni",
               "IRIX",      "irx"
               );

%archTable  = ("sun4c",         "sparc",
               "sun4m",         "sparc",
               "sun4u",         "sparc",
               "9000/712",      "hp9000",
               "9000/715",      "hp9000",
               "9000/778",      "hp9000",
               "000029484600",  "rs6000",       # salsa
               "000328144600",  "rs6000",       # lambada
               "00245D29E000",  "rs6000",       # segv
               "00245C46E000",  "rs6000",       # bus
               "002012974600",  "rs6000",       # tango
               "0006404C4C00",  "rs6000",       # jive
               "2003",          "g3",           # ezopen4
               "i86pc",         "x86",          # blauwal
               "i386",          "x86",          # SCO_SV, NetBSD, FreeBSD
               "i486",          "x86",          # tiger
               "i586",          "x86",          # blauwal
               "i686",          "x86",
               "prep",          "ppc",          # hooge
               "ppc",           "ppc",          # ppc-linux
               "alpha",         "alpha",
               "RM400",         "rm400",        # garbo
               "IP22",          "mips",         # voyager
               "IP32",          "mips",         # giotto
               "Power Macintosh",           "ppc",          # NetBSD/arm32
               "arm32",         "arm32"         # NetBSD/arm32
               );

%archDefTable=("sun4c",         "-DSPARC -DSUN -DSUN4", # hawai
               "sun4m",         "-DSPARC -DSUN -DSUN4", # broccoli
               "sun4u",         "-DSPARC -DSUN -DSUN4", # broccoli
               "9000/712",      "-DHP9000",     # tuborg
               "9000/715",      "-DHP9000",     # koepi
               "9000/778",      "-DHP9000",     # jever, flens
               "000029484600",  "-DRS6000",     # salsa
               "000328144600",  "-DRS6000",     # lambada
               "00245D29E000",  "-DRS6000",     # segv
               "00245C46E000",  "-DRS6000",     # segv
               "002012974600",  "-DRS6000",     # tango
               "0006404C4C00",  "-DRS6000",     # jive
               "2003",          "-DG3",         # ezopen4
               "i86pc",         "-DX86 -DINTEL",# gepard
               "i386",          "-DX86",        # SCO_SV, NetBSD, FreeBSD
               "i486",          "-DX86",        # tiger
               "i586",          "-DX86",        # blauwal
               "i686",          "-DX86",        #
               "prep",          "-DPPC",        # hooge
               "ppc",           "-DPPC",        # ppc-linux
               "alpha",         "-DALPHA",      # mars
               "RM400",         "-DMIPS -DRM400",   # ekberg, garbo
               "IP22",          "-DMIPS",           # voyager
               "IP32",          "-DMIPS",           # giotto
               "Power Macintosh",       "-DPPC",        # NetBSD/arm32
               "arm32",         "-DARM32"       # NetBSD/arm32
               );

%archDosTable=("sun4c",         "s",        # hawai
               "sun4m",         "s",        # broccoli
               "sun4u",         "s",        # broccoli
               "9000/712",      "r",        # tuborg
               "9000/715",      "r",        # koepi
               "9000/778",      "r",        # jever, flens
               "000029484600",  "p",        # salsa
               "000328144600",  "p",        # lambada
               "00245D29E000",  "p",        # segv
               "00245C46E000",  "p",        # segv
               "002012974600",  "p",        # tango
               "0006404C4C00",  "p",        # jive
               "2003",          "g",        # ezopen4
               "i86pc",         "i",        # gepard
               "i386",          "i",        # SCO_SV, NetBSD, FreeBSD
               "i486",          "i",        # tiger
               "i586",          "i",        # blauwal
               "i686",          "i",
               "prep",          "p",        # hooge
               "ppc",           "p",        # ppc-linux
               "alpha",         "a",        # mars
               "RM400",         "m",        # ekberg, garbo
               "IP22",          "m",        # voyager
               "IP32",          "m",        # giotto
               "Power Macintosh",           "p",            # NetBSD/arm32
               "arm32",         "a"         # NetBSD/arm32
               );

$main::solarDef     = $osDefTable{ $s_str }.' '.$archDefTable{ $m_str };

$main::solarOS      = $osTable{ $s_str };
$main::solarMT      = $archTable{ $m_str };

$main::solarArch    = $main::solarOS.$main::solarMT;
$main::solarOldArch = $main::solarArch; # for compatibility

$main::solarDOS     = 'unx'.$osDosTable{ $s_str }. $archDosTable{ $m_str };

sub main::osarch
{
    local ( $flag ) = @_;

    $perlFlag   = $flag eq 'perl';
    $defFlag    = $flag eq 'def';
    $oldFlag    = $flag eq 'old';
    $dirFlag    = $flag eq 'dir';
    $dosFlag    = $flag eq 'dos';

    local( $result ) = ( '' );

    if ( $perlFlag )
    {
        $result .= '$solarDef     = '."'$main::solarDef';\n";
        $result .= '$solarArch    = '."'$main::solarArch';\n";
        $result .= '$solarOldArch = '."'$main::solarOldArch';\n";
        $result .= '$solarOS      = '."'$main::solarOS';\n";
        $result .= '$solarMT      = '."'$main::solarMT';\n";
        $result .= '$solarDOS     = '."'$main::solarDOS';";
    }
    elsif ( $defFlag )
    {
        $result = $main::solarDef;
    }
    elsif ( $dirFlag )
    {
        $result = "$main::solarOS-$main::solarMT";
    }
    elsif ( $dosFlag )
    {
        $result = $main::solarDOS;
    }
    else
    {
        $result = $oldFlag ? $main::solarOldArch : $main::solarArch;
    }

    return $result;
}

1;
