: # -*- perl -*-
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#
# mkdir - a perl script to substitute mkdir -p
# accepts "/", ":", and "\" as delimiters of subdirectories
# options -p (for compatibility)
#         -mode mode
#
# Copyright (c) 2000 Sun Microsystems, Inc.

$MODE = 00777 ;

while ( $#ARGV > 0 ) {
    if ( $ARGV[0] eq "-mode" ) {
        $MODE = oct $ARGV[1] ;
        shift @ARGV ;
        } ;
    shift @ARGV ;
    } ;

$ARGV[0] =~ s?\\|:?/?g ;
@SUBDIRS = split "/", $ARGV[0] ;

# absolute path UNIX
if ( $SUBDIRS[0] eq "" ) {
    chdir '/' ;
    shift @SUBDIRS ;
}
# absolute path WINDOWS
if ( $#SUBDIRS > 1 ) {
    if ( $SUBDIRS[1] eq "" ) {
        if ( $SUBDIRS[0] =~ /\w/ ) {
            chdir "$SUBDIRS[0]:\\" ;
            shift @SUBDIRS ;
            shift @SUBDIRS ;
        } ;
    } ;
}


while (@SUBDIRS) {
    if ( -e $SUBDIRS[0] ) {
        if ( ! -d $SUBDIRS[0] ) {
            die "file exists\n" }
        }
    else {
        mkdir $SUBDIRS[0], $MODE or die "Can't create directory $SUBDIRS[0]"
        }
    chdir $SUBDIRS[0] or die "Can't cd to $SUBDIRS[0]" ;
    shift @SUBDIRS ;
    } ;
