:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



use Cwd;
use File::Temp qw/ tempfile tempdir /;

my $verbosity = 1;  # will be adjusted to the value of $Env{VERBOSE} below

# platforms which are not supported anymore, and thus can be filtered from the svn:ignore list
my %obsolete_platforms = (
    );
    # (none so far)

# platforms whose output trees should appear in all modules' svn:ignore list
my @platforms = (
        "common",
        "unxlngi6",
        "unxlngx6",
        "unxsols4",
        "unxsolu4",
        "unxsoli4",
        "wntmsci12",
        "unxmacxi",
        "unxmacxp",
        "unxmacci",
        "unxmaccx",
        "unxubit8",
        "unxaixp",
        "unxbsda",
        "unxbsdi2",
        "unxbsdi",
        "unxbsds",
        "unxfbsdi",
        "unxfbsd",
        "unxfbsdx",
        "unxhpgr",
        "unxhpxr",
        "unxirgm",
        "unxirxm3",
        "unxirxm",
        "unxlnga",
        "unxlngm68k",
        "unxlngmips",
        "unxlngp",
        "unxlngppc4",
        "unxlngppc64",
        "unxlngppc",
        "unxlngr",
        "unxlngs3904",
        "unxlngs390x",
        "unxlngs",
        "unxlnxi",
        "unxsogi",
        "unxsogs"
    );


# .........................................................................
# retrieves the repository URL of the SVN working copy in the current directory
sub retrieve_repo_url
{
    open( SVN, "svn info 2>&1 |" );
    my @result = <SVN>;
    close( SVN );

    foreach (@result)
    {
        chomp;
        next if ( ! /^URL: / );
        s/^URL: //;
        return $_;
    }
    return undef;
}

# .........................................................................
# gets the "modules" below the given SVN repository URL, by calling "svn list"
sub get_modules
{
    my @modules = ();

    open( SVN, "svn list $_ 2>&1 |" );
    my @result = <SVN>;
    close( SVN );

    foreach (@result)
    {
        chomp;
        s/\/$//;
        push @modules, $_;
    }

    return @modules;
}

# .........................................................................
sub set_ignore_property
{
    my ($repo_url, @modules) = @_;

    # max length of a module name
    my $max_len = 0;
    foreach ( @modules ) { $max_len = length( $_ ) if ( length( $_ ) > $max_len ); }

    my $updated = 0;

    my $current = 0;
    my $count = $#modules + 1;
    foreach $module ( @modules )
    {
        ++$current;

        # print progress
        if ( $verbosity > 1 )
        {
            my $progress = "$module ";
            $progress .= "(" . $current . "/" . $count . ")";

            my $dots = 3 + ( $max_len - length($module) );
            $dots += int( digits( $count ) ) - int( digits( $current ) );

            $progress .= ( "." x $dots );
            $progress .= " ";

            print STDOUT $progress;
        }
        elsif ( $verbosity > 0 )
        {
            print STDOUT ".";
        }

        # retrieve the current ignore list
        open( SVN, "svn propget svn:ignore $module 2>&1 |" );
        my @ignore_list = <SVN>;
        close( SVN );

        # the last item in the list is an empty string, usually. Don't let it confuse the below
        # code
        my $supposed_empty = pop @ignore_list;
        chomp( $supposed_empty );
        push( @ignore_list, $supposed_empty ) if ( !( $supposed_empty =~ /^$/ ) );

        # filter out obsolte entries
        my @stripped_ignore_list = ();
        foreach $ignore_entry (@ignore_list)
        {
            chomp( $ignore_entry );
            next if ( $ignore_entry =~ /^$/ );

            if  (   ( exists $obsolete_platforms{$ignore_entry} )
                ||  ( exists $obsolete_platforms{"$ignore_entry.pro"} )
                )
            {
                next;
            }
            push @stripped_ignore_list, $ignore_entry;
        }
        my $removed = $#ignore_list - $#stripped_ignore_list;
        @ignore_list = @stripped_ignore_list;

        # append the platforms which should appear in the ignore list
        my %ignore_list = ();
        foreach (@ignore_list) { $ignore_list{$_} = 1; }
        foreach $platform_entry ( @platforms )
        {
            $ignore_list{$platform_entry} = 1;
            $ignore_list{"$platform_entry.pro"} = 1;
        }
        my @extended_ignore_list = keys %ignore_list;
        my $added = $#extended_ignore_list - $#ignore_list;
        @ignore_list = @extended_ignore_list;

        if ( $removed || $added )
        {
            # create a temporary file taking the new content of the svn_ignore property
            my $temp_dir = tempdir( CLEANUP => 1 );
            my ($fh, $filename) = tempfile( DIR => $dir );
            open( IGNORE, ">$filename" );
            print IGNORE join "\n", @ignore_list;
            close( IGNORE );

            # actually set the property
            open( SVN, "svn propset -F $filename svn:ignore $module 2>&1 |" );

            ++$updated;
        }

        # statistics
        print STDOUT "done (removed/added: $removed/$added)\n" if $verbosity > 1;
    }

    print STDOUT "\n" if $verbosity eq 1;
    print STDOUT "$updated module(s) updated\n" if $verbosity > 0;
}

# .........................................................................
sub digits
{
    my ($number, $base) = @_;
    $base = 10 if !defined $base;
    return log($number)/log($base);
}

# .........................................................................
# 'main'

# initialize verbosity
my $verbose = $ENV{VERBOSE};
if ( defined $verbose )
{
    $verbose = uc( $verbose );
    $verbosity = 2 if ( $verbose eq "TRUE" );
    $verbosity = 0 if ( $verbose eq "FALSE" );
}

# work on the current directory
my $working_copy_root = cwd();
die "current directory does not contain an SVN working copy" if !-d $working_copy_root . "/\.svn";

# retrieve repository URL
my $repo_url = retrieve_repo_url();
die "unable to retrieve repository URL" if !defined $repo_url;
print STDOUT "repository URL: $repo_url\n" if $verbosity > 1;

# list modules
my @modules = get_modules( $repo_url );
print STDOUT "processing " . ( $#modules + 1 ) . " modules\n" if $verbosity > 0;

# process modules, by setting the svn:ignore property
set_ignore_property( $repo_url, @modules );
