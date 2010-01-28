:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: fast_merge.pl,v $
#
#   $Revision: 1.1.2.2 $
#
#   last change: $Author: ihi $ $Date: 2007/07/20 10:37:53 $
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

use strict;
use Class::Struct;
use Getopt::Long;
use File::Temp;
use File::Path;

my @files;
my @file_names;
my $module_name = '';
my @current;
my @buffer;
my $last_file;
my $last_path;
my $last_localize_file;
my $first_run = "1";
my $sdf_filename;
my $merge_dir;
my $WIN;
my $state = "none";

if ( defined $ENV{USE_SHELL} && $ENV{USE_SHELL} eq '4nt' ) { $WIN = 'TRUE'; }
else { $WIN = ''; }

$SIG{INT}  = 'inthandler';
$SIG{QUIT} = 'quithandler';

struct ( sdf_obj =>
{
    module      => '$',
    file        => '$',
    dir         => '$',
    FILEHANDLE  => '$',
    line        => '$',
    endoffile   => '$'
}
);

parse_options();
my $lock_file   = $merge_dir."/lock.mk";
$lock_file =~ s/\//\\/g , if ( $WIN ) ;
acquire_lock();
read_sdf_file_names();
init();
my $reference;
my $path ;
my $localize_file;
while( hasLines() )
{
    @current = ();
    foreach ( @files )
    {
        push @current , $_;
    }

    $reference = getNextIdentifier( );

    @current = ();
    foreach ( @files )
    {
        if( $_->module eq $reference->module && $_->dir eq $reference->dir )
        {
            push @current , $_ ;
        }
    }
    write_lines();
}
if( $#current+1 ne 0 )
{
    ( $path , $localize_file ) = make_paths();
    add_to_buffer();
    write_buffer( $path , $localize_file );
}
release_lock();
exit( 0 );

##########################################################################################
sub acquire_lock
{
    if( -e $lock_file ){
        $state = "blocked";
        print "WARNING: Lock file '$lock_file' 'found, waiting ....\n";
        my $cnt = 0;
        sleep 10 , while( -e $lock_file && $cnt++ < 180 );
        exit( 0 );
    }else
    {
        $state = "locked";
        print "Writing lock file '$lock_file'\n";
        open FILE, ">$lock_file" or die "Can't create lock file '$lock_file'";
        print FILE "L10N_LOCK=YES" ;
        close ( FILE );
    }
}
sub release_lock
{
    print "Deleting lock file '$lock_file'\n";
    unlink $lock_file, if( -e $lock_file );
    $state = "none";
}
sub inthandler
{
    release_lock() , if( $state eq "locked" );
    exit( -1 );
}
sub quithandler
{
    release_lock() , if( $state eq "locked" );
    exit( 0 );
}

sub init
{
    foreach my $file ( @file_names )
    {
        my $obj = new sdf_obj;
        open my $FILEHANDLE , "<$file" or die "Can't open file '$file'";
        $obj->FILEHANDLE ( $FILEHANDLE ) ;
        getNextSdfObj( $obj );
        push @files, $obj ;
        print "Open file '$file'\n";
    }
}

# get the next module/file
sub getNextIdentifier
{
    my @sorted = sort {
        return $a->module.$a->dir cmp $b->module.$b->dir;
    } @current ;
    return shift @sorted;
}

# update the obj with the next line
sub getNextSdfObj
{
    my $obj             = shift;
    my $line = readline ( $obj->FILEHANDLE );
    if ( $line eq undef )
    {
        $obj->endoffile( "true" );
    }
    else
    {
        $line =~ /^(([^\t]*)\t([^\t]*).*)/o ;
        if( defined $1 && defined $2 && defined $3 )
        {
            $obj->line  ( $1 );
            $obj->module( $2 );
            $obj->file  ( $3 );
            $obj->dir   ( getDir( $3 ) );
        }
        else
        {
            $obj->line  ( "" );
            $obj->module( "" );
            $obj->file  ( "" );
            $obj->dir   ( "" );
        }
    }
    return $obj;
}
sub getNextSdfObjModule
{
    my $obj             = shift;
    while( !$obj->endoffile )
    {
        my $line = readline ( $obj->FILEHANDLE );
        if ( $line eq undef )
        {
            $obj->endoffile( "true" );
        }
        else
        {
            $line =~ /^(([^\t]*)\t([^\t]*).*)/o ;
            if( defined $1 && defined $2 && defined $3 )
            {
                $obj->line  ( $1 );
                $obj->module( $2 );
                $obj->file  ( $3 );
                $obj->dir   ( getDir( $3 ) );
            }
            else
            {
                $obj->line  ( "" );
                $obj->module( "" );
                $obj->file  ( "" );
                $obj->dir   ( "" );
            }
            return $obj , if( $obj->module eq $module_name )
        }
    }
    #return $obj;
}
sub getDir
{
    my $path     = shift ;
    $path        =~ s/\//\\/g;
    my @tmp_path = split /\\/ , $path;
    pop @tmp_path;
    $path        = join  '\\' , @tmp_path;
    return $path;
}

sub hasLines
{
    my $hasLines  = "";
    my @tmpfiles;
    foreach ( @files )
    {
        push @tmpfiles , $_, if( !$_->endoffile );
    }
    @files = @tmpfiles;
    return $#files+1;
}

sub make_paths
{
    my $localizeFile = $merge_dir."\\".$current[ 0 ]->module."\\".$current[ 0 ]->file;
    my $path = getDir( $localizeFile );
    if ( !$WIN ) { $path =~ s/\\/\//g; }

    $localizeFile = $path."\\localize.sdf";
    if ( !$WIN ) { $localizeFile =~ s/\\/\//g; }

    return ( $path , $localizeFile );
}
sub write_lines
{
    if( $first_run ){
        add_to_buffer();
        my( $path , $localize_file ) = make_paths();
        $last_path = $path;
        $last_localize_file = $localize_file;
        mkpath $path;
        write_buffer( $path , $localize_file );
        $first_run = '';
    }
    else
    {
        return , if ( $#current+1 eq 0 );
        my( $path , $localize_file ) = make_paths();
        if( $path eq $last_path )
        {
            add_to_buffer();
        }
        else
        {
            mkpath $path;
            write_buffer( $last_path , $last_localize_file );
            add_to_buffer();
            $last_path = $path;
            $last_localize_file = $localize_file;
        }
    }
}
sub add_to_buffer
{
    my $plainline;
    my $afile;
    my $amodule;
    foreach my $elem ( @current )
    {
        do {
        $amodule=$elem->module;
        $afile=$elem->file;
        $plainline=$elem->line;
        push @buffer, $plainline;
        getNextSdfObj( $elem );
        } while ( !$elem->endoffile && $amodule eq $elem->module && $afile eq $elem->file );
    }
}
sub write_buffer
{
    my $path            = shift;
    my $localize_file   = shift;
    my $cnt             = $#buffer+1;
    print "Write to $path $cnt lines\n";
    open FILE , ">>$localize_file" or die "Can't open file '$localize_file'\n";
    foreach ( @buffer )
    {
        print FILE $_."\n";
    }
    @buffer = ();
}
sub parse_options
{
    my $success = GetOptions( 'sdf_files=s' => \$sdf_filename , 'merge_dir=s' => \$merge_dir ); #, 'module=s' => \$module_name );
    if( ! ( $sdf_filename && $merge_dir && $success ) )
    {
        usage();
        exit( -1 );
    }
}

sub usage
{
    print "Usage: fast_merge -sdf_files <file containing sdf file names> -merge_dir <directory>\n" ;
}

sub read_sdf_file_names
{
    open FILE , "<$sdf_filename" or die "Can't open file '$sdf_filename'\n";
    while (  <FILE> )
    {
        push @file_names , split " " , $_ ;
    }
    close ( FILE );
}


