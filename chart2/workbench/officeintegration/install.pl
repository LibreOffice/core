:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;

# ------------------------------------------------------------------------------
# This script installs the new chart-libraries and resources.
# This is done by symbolic linking rather than copying.  Thus this installation
# is only necessary once after a fresh office installation.
#
# It also registers all types in the types.rdb and all components in the
# services.rdb.  Finally the TypeDetection.xcu is installed, such that you can
# invoke the new chart with the URL 'private:factory/chart'
# ------------------------------------------------------------------------------

# With this package you get better readable variable names
use English;

# determine automatic project path assuming that this script stayed in its
# default location

$PRJ_PATH = $0;
if( $OSNAME eq "MSWin32" ) {
    chomp( $PRJ_PATH = `cd` );
    $PRJ_PATH =~ s,^(.*)\\workbench\\?.*$,$1,;

    $SVERSION = "$ENV{UserProfile}\\Application Data\\sversion.ini";
    $PATHSEP = "\\";
    $LIBPATTERN = "bin\\*mi*.dll";
    $COPYCMD = "copy";
} else {
    chomp( $PRJ_PATH = `pwd` );
    $PRJ_PATH = $PWD . "/" . $PRJ_PATH if( $PRJ_PATH !~ m,^/, );
    $PRJ_PATH =~ s,^(.*)/workbench/?.*$,$1,;

    $SVERSION = "$ENV{HOME}/.sversionrc";
    $PATHSEP = "/";
    $LIBPATTERN = "lib/lib*li.so";
    $COPYCMD = "cp";
}

print "$OSNAME\n";

# determine office path according to .sversionrc. Note: this only looks for a
# prefix 'StarOffice 8', such that 'StarOffice 8 dbg=...' is still found
# if no 'StarOffice 8' was found, 'OpenOffice.org 2' is searched for

open( SVERSION, "$SVERSION" ) || die( "couldn't find .sversionrc/sversion.ini\n" );
while( <SVERSION> )
{
    if( m,^StarOffice 8.*=file://(.*)$, )
    {
        $OFF_PATH = $1,;
        if( $OSNAME eq "MSWin32" )
        {
            # remove first /
            $OFF_PATH =~ s/^.//;
            # replace path separators
            $OFF_PATH =~ s,/,\\,g;
        }
        last;
    }
    elsif ( m,^OpenOffice.org 2.*=file://(.*)$, )
    {
        $OFF_PATH = $1,;
        if( $OSNAME eq "MSWin32" )
        {
            # remove first /
            $OFF_PATH =~ s/^.//;
            # replace path separators
            $OFF_PATH =~ s,/,\\,g;
        }
        last;
    }
}
close SVERSION;

# ================================================================================

print "Chart2 project is in <$PRJ_PATH>\n";
print "OpenOffice.org 2.0/StarOffice 8 is installed in <$OFF_PATH>\n";
print "Are these assumptions correct? [y/n] ";
if( <STDIN> !~ /^y?$/i )
{
    print "Enter project path [$PRJ_PATH]: ";
    chomp( $path = <STDIN> );
    $PRJ_PATH = $path if( length( $path ) > 0 );

    print "Enter office path [$OFF_PATH]: ";
    chomp( $path = <STDIN> );
    $OFF_PATH = $path if( length( $path ) > 0 );
}

# ----------------------------------------

$MY_OUTPATH = $ENV{OUTPATH};
$MY_RESPATH = "common";

$REGCOMMAND = "register";
#$REGCOMMAND = "revoke";

if( $ENV{PROEXT} ) {
    $MY_OUTPATH .= $ENV{PROEXT};
    $MY_RESPATH .= $ENV{PROEXT};
}

chmod 0664, "$OFF_PATH${PATHSEP}program${PATHSEP}types.rdb";
print "registering types...\n";
print `regmerge $OFF_PATH${PATHSEP}program${PATHSEP}types.rdb / $PRJ_PATH${PATHSEP}$MY_OUTPATH${PATHSEP}bin${PATHSEP}chart2.rdb`;

print "installing shlibs and registering services...\n";
@files = glob( "$PRJ_PATH${PATHSEP}$MY_OUTPATH${PATHSEP}${LIBPATTERN}" );
foreach $dll (@files)
{
    if( $OSNAME eq "MSWin32" ) {
        print `copy "$dll" "$OFF_PATH\\program"`;
        $dll =~ s,\\,/,g;
        $dll =~ m,/([^/]*)$,;
        $copied_dll = "/$dll";
    } else {
        $dll =~ m,/([^/]*)$,;
        $copied_dll = "$OFF_PATH/program/$1";
        if( -l "$copied_dll" ) {
            unlink "$copied_dll";
        }
        symlink "$dll", "$copied_dll";
    }
    chmod 0664, "$OFF_PATH${PATHSEP}program${PATHSEP}services.rdb";
    print `regcomp -$REGCOMMAND -r $OFF_PATH${PATHSEP}program${PATHSEP}services.rdb -c file://$copied_dll`;
    print `regcomp -$REGCOMMAND -r $OFF_PATH${PATHSEP}program${PATHSEP}types.rdb -c file://$copied_dll`;
}

print "installing resources\n";
@files = glob( "$PRJ_PATH${PATHSEP}$MY_RESPATH${PATHSEP}bin${PATHSEP}*.res" );
foreach $res (@files)
{
    if( $OSNAME eq "MSWin32" ) {
        print `copy "$res" "$OFF_PATH\\program\\resource"`;
    } else {
        $res =~ m,/([^/]*)$,;
        $copied_res = "$OFF_PATH/program/resource/$1";
        if( -l "$copied_dll" ) {
            unlink "$copied_res";
        }
        symlink "$res", "$copied_res";
    }
}

print "installing filter...\n";
print `$COPYCMD $PRJ_PATH${PATHSEP}workbench${PATHSEP}officeintegration${PATHSEP}TypeDetection.xcu $OFF_PATH${PATHSEP}user${PATHSEP}registry${PATHSEP}data${PATHSEP}org${PATHSEP}openoffice${PATHSEP}Office`;
