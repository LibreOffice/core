#!/usr/bin/env perl

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

=head1 NAME

    download_external_libraries.pl - Load missing tarballs specified in main/external_libs.lst.

=head1 SYNOPSIS

    For downloading external libraries (typically from the main/bootstrap script):

    download_external_libraries(<data-file-name>);

=head1 DESCRIPTION

    The contents of the main/external_libs.lst file are used to determine the
    external library tarballs that are missing from ext_sources/.

    Individual libraries can be ignored depending on the values of environment variables.

    Format of the main/external_libs.lst file:

    The file is line based.
    Comments start with a # and go to the end of the line and are ignored.
    Lines that are empty or contain only spaces and/or comments are ignored.

    All other lines can have one of two forms:
    - A variable definition of the form <name>=<value>.
    - A conditional block start in the form "if (<expression>)"

    Variables defined in a conditional block are only visible in this block and
    replace the definition of global variables and variables earlier in the same
    block.
    Some variables have special names:
    - MD5 is the expected MD5 checksum of the library tarball.
    - SHA1 is the expected SHA1 checksum of the library tarball.
    - URL1 to URL9 specify from where to download the tarball.  The urls are tried in order.
      The first successful download (download completed and checksum match) stops the iteration.

    Expressions are explained below in the comment of EvaluateExpression().

    A library is only regarded if its conditional expression evaluates to 1.

    Example:

    DefaultSite=http://some-internet-site.org
    if ( true )
        MD5 = 0123456789abcdef0123456789abcdef
        name = library-1.0.tar.gz
        URL1 = http://some-other-internet-site.org/another-name.tgz
        URL2 = $(DefaultSite)$(MD5)-$(name)

    This tries to load a library first from some-other-internet-site.org and if
    that fails from some-internet-site.org.  The library is stored as $(MD5)-$(name)
    even when it is loaded as another-name.tgz.

=cut


use strict;

use File::Spec;
use File::Path;
use File::Basename;
use Digest::MD5;
use Digest::SHA;
use URI;
use LWP::UserAgent;

my $Debug = 1;

my $LocalEnvironment = undef;
my $GlobalEnvironment = {};
my @Missing = ();




=head3 ProcessDataFile

    Read the data file, typically named main/external_libs.lst, find the external
    library tarballs that are not yet present in ext_sources/ and download them.

=cut
sub ProcessDataFile ($)
{
    my $filename = shift;

    my $destination = $ENV{'TARFILE_LOCATION'};

    die "can not open data file $filename" if ! -e $filename;

    my $current_selector_value = 1;
    my @URLHeads = ();
    my @download_requests = ();

    open my $in, $filename;
    while (my $line = <$in>)
    {
        # Remove leading and trailing space and comments
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        $line =~ s/\s*#.*$//;

        # Ignore empty lines.
        next if $line eq "";

        # An "if" statement starts a new block.
        if ($line =~ /^\s*if\s*\(\s*(.*?)\s*\)\s*$/)
        {
            ProcessLastBlock();

            $LocalEnvironment = { 'selector' => $1 };
        }

        # Lines of the form name = value define a local variable.
        elsif ($line =~ /^\s*(\S+)\s*=\s*(.*?)\s*$/)
        {
            if (defined $LocalEnvironment)
            {
                $LocalEnvironment->{$1} = $2;
            }
            else
            {
                $GlobalEnvironment->{$1} = $2;
            }
        }
        else
        {
            die "can not parse line $line\n";
        }
    }

    ProcessLastBlock();

    Download(\@download_requests, \@URLHeads);
}




=head3 ProcessLastBlock

    Process the last definition of an external library.
    If there is not last block, true for the first "if" statement, then the call is ignored.

=cut
sub ProcessLastBlock ()
{
    # Return if no block is defined.
    return if ! defined $LocalEnvironment;

    # Ignore the block if the selector does not match.
    if ( ! EvaluateExpression(SubstituteVariables($LocalEnvironment->{'selector'})))
    {
        printf("ignoring %s because its prerequisites are not fulfilled\n", GetValue('name'));
    }
    else
    {
        my $name = GetValue('name');
        my $checksum = GetChecksum();

        if ( ! IsPresent($name, $checksum))
        {
            AddDownloadRequest($name, $checksum);
        }
    }
}




=head3 AddDownloadRequest($name, $checksum)

    Add a request for downloading the library $name to @Missing.
    Collect all available URL[1-9] variables as source URLs.

=cut
sub AddDownloadRequest ($$)
{
    my ($name, $checksum) = @_;

    print "adding download request for $name\n";

    my $urls = [];
    my $url = GetValue('URL');
    push @$urls, SubstituteVariables($url) if (defined $url);
    for (my $i=1; $i<10; ++$i)
    {
        $url = GetValue('URL'.$i);
        next if ! defined $url;
        push @$urls, SubstituteVariables($url);
    }

    push @Missing, [$name, $checksum, $urls];
}




=head3 GetChecksum()

    When either MD5 or SHA1 are variables in the current scope then return
    a reference to a hash with two entries:
        'type' is either 'MD5' or 'SHA1', the type or algorithm of the checksum,
        'value' is the actual checksum
    Otherwise undef is returned.

=cut
sub GetChecksum()
{
    my $checksum = GetValue("MD5");
    if (defined $checksum && $checksum ne "")
    {
        return { 'type' => 'MD5', 'value' => $checksum };
    }
    elsif (defined ($checksum=GetValue("SHA1")) && $checksum ne "")
    {
        return { 'type' => 'SHA1', 'value' => $checksum };
    }
    else
    {
        return undef;
    }
}




=head3 GetValue($variable_name)

    Return the value of the variable with name $variable_name from the local
    environment or, if not defined there, the global environment.

=cut
sub GetValue ($)
{
    my $variable_name = shift;

    my $candidate = $LocalEnvironment->{$variable_name};
    return $candidate if defined $candidate;

    return $GlobalEnvironment->{$variable_name};
}



=head3 SubstituteVariables($text)

    Replace all references to variables in $text with the respective variable values.
    This is done repeatedly until no variable reference remains.

=cut
sub SubstituteVariables ($)
{
    my $text = shift;

    my $infinite_recursion_guard = 100;
    while ($text =~ /^(.*?)\$\(([^)]+)\)(.*)$/)
    {
        my ($head,$name,$tail) = ($1,$2,$3);
        my $value = GetValue($name);
        die "can not evaluate variable $name" if ! defined $value;
        $text = $head.$value.$tail;

        die "(probably) detected an infinite recursion in variable definitions" if --$infinite_recursion_guard<=0;
    }

    return $text;
}




=head3 EvaluateExpression($expression)

    Evaluate the $expression of an "if" statement to either 0 or 1.  It can
    be a single term (see EvaluateTerm for a description), or several terms
    separated by either all ||s or &&s.  A term can also be an expression
    enclosed in parantheses.

=cut
sub EvaluateExpression ($)
{
    my $expression = shift;

    # Evaluate sub expressions enclosed in parantheses.
    while ($expression =~ /^(.*)\(([^\(\)]+)\)(.*)$/)
    {
        $expression = $1 . (EvaluateExpression($2) ? " true " : " false ") . $3;
    }

    if ($expression =~ /&&/ && $expression =~ /\|\|/)
    {
        die "expression can contain either && or || but not both at the same time";
    }
    elsif ($expression =~ /&&/)
    {
        foreach my $term (split (/\s*&&\s*/,$expression))
        {
            return 0 if ! EvaluateTerm($term);
        }
        return 1;
    }
    elsif ($expression =~ /\|\|/)
    {
        foreach my $term (split (/\s*\|\|\s*/,$expression))
        {
            return 1 if EvaluateTerm($term);
        }
        return 0;
    }
    else
    {
        return EvaluateTerm($expression);
    }
}




=head3 EvaluateTerm($term)

    Evaluate the $term to either 0 or 1.
    A term is either the literal "true", which evaluates to 1, or an expression
    of the form NAME=VALUE or NAME!=VALUE.  NAME is the name of an environment
    variable and VALUE any string.  VALUE may be empty.

=cut
sub EvaluateTerm ($)
{
    my $term = shift;

    if ($term =~ /^\s*([a-zA-Z_0-9]+)\s*(==|!=)\s*(.*)\s*$/)
    {
        my ($variable_name, $operator, $given_value) = ($1,$2,$3);
        my $variable_value = $ENV{$variable_name};
        $variable_value = "" if ! defined $variable_value;

        if ($operator eq "==")
        {
            return $variable_value eq $given_value;
        }
        elsif ($operator eq "!=")
        {
            return $variable_value ne $given_value;
        }
        else
        {
            die "unknown operator in term $term";
        }
    }
    elsif ($term =~ /^\s*true\s*$/i)
    {
        return 1;
    }
    elsif ($term =~ /^\s*false\s*$/i)
    {
        return 0;
    }
    else
    {
        die "term $term is not of the form <environment-variable> (=|==) <value>";
    }
}




=head IsPresent($name, $given_checksum)

    Check if an external library tar ball with the basename $name already
    exists in the target directory TARFILE_LOCATION.  The basename is
    prefixed with the MD5 or SHA1 checksum.
    If the file exists then its checksum is compared to the given one.

=cut
sub IsPresent ($$)
{
    my ($name, $given_checksum) = @_;

    my $filename = File::Spec->catfile($ENV{'TARFILE_LOCATION'}, $given_checksum->{'value'}."-".$name);
    return 0 unless -f $filename;

    # File exists.  Check if its checksum is correct.
    my $checksum;
    if ( ! defined $given_checksum)
    {
        print "no checksum given, can not verify\n";
        return 1;
    }
    elsif ($given_checksum->{'type'} eq "MD5")
    {
        my $md5 = Digest::MD5->new();
        open my $in, $filename;
        $md5->addfile($in);
        $checksum = $md5->hexdigest();
    }
    elsif ($given_checksum->{'type'} eq "SHA1")
    {
        my $sha1 = Digest::SHA->new("1");
        open my $in, $filename;
        $sha1->addfile($in);
        $checksum = $sha1->hexdigest();
    }
    else
    {
        die "unsupported checksum type (not MD5 or SHA1)";
    }

    if ($given_checksum->{'value'} ne $checksum)
    {
        # Checksum does not match.  Delete the file.
        print "$name exists, but checksum does not match => deleting\n";
        unlink($filename);
        return 0;
    }
    else
    {
        printf("%s exists, %s checksum is OK\n", $name, $given_checksum->{'type'});
        return 1;
    }
}




=head3 Download

    Download a set of files specified by @Missing.

    For http URLs there may be an optional checksum.  If it is present then downloaded
    files that do not match that checksum lead to abortion of the current process.
    Files that have already been downloaded are not downloaded again.

=cut
sub Download ()
{
    my $download_path = $ENV{'TARFILE_LOCATION'};

    if (scalar @Missing > 0)
    {
        printf("downloading %d missing tar ball%s to %s\n",
               scalar @Missing, scalar @Missing>0 ? "s" : "",
               $download_path);
    }
    else
    {
        print "all external libraries present\n";
        return;
    }

    # Download the missing files.
    my $all_downloaded = 1;
    for my $item (@Missing)
    {
        my ($name, $checksum, $urls) = @$item;

        my $downloaded = 0;
        foreach my $url (@$urls)
        {
            $downloaded = DownloadFile(
                defined $checksum
                    ? $checksum->{'value'}."-".$name
                    : $name,
                $url,
                $checksum);
            last if $downloaded
        }
        $all_downloaded &&= $downloaded;
    }
    die "some needed files could not be downloaded!" if !$all_downloaded;
}




=head3 DownloadFile($name,$URL,$checksum)

    Download a single external library tarball.  It origin is given by $URL.
    Its destination is $(TARFILE_LOCATION)/$checksum-$name.

=cut
sub DownloadFile ($$$)
{
    my $name = shift;
    my $URL = shift;
    my $checksum = shift;

    if (defined $checksum)
    {
        system(
            $ENV{'JAVAINTERPRETER'},
            "-cp",
            File::Spec->catfile(
                File::Spec->catfile($ENV{'SOLARENV'}, $ENV{'INPATH'}),
                "class"),
            "AOOJavaDownloader",
            $name,
            $URL,
            $checksum->{'type'},
            $checksum->{'value'});
    }
    else
    {
        system(
            $ENV{'JAVAINTERPRETER'},
            "-cp",
            File::Spec->catfile(
                File::Spec->catfile($ENV{'SOLARENV'}, $ENV{'INPATH'}),
                "class"),
            "AOOJavaDownloader",
            $name,
            $URL);
    }

    my $rc = $? >> 8;
    if ($rc == 0)
    {
        return 1;
    }
    elsif ($rc == 1)
    {
        return 0;
    }
    else
    {
        exit $rc;
    }
}




=head3 CheckDownloadDestination ()

    Make sure that the download destination $TARFILE_LOCATION does exist.  If
    not, then the directory is created.

=cut
sub CheckDownloadDestination ()
{
    my $destination = $ENV{'TARFILE_LOCATION'};
    die "ERROR: no destination defined! please set TARFILE_LOCATION!" if ($destination eq "");

    if ( ! -d $destination)
    {
        File::Path::make_path($destination);
        die "ERROR: can't create \$TARFILE_LOCATION" if  ! -d $destination;
    }
}




=head3 ProvideSpecialTarball ($url,$name,$name_converter)

    A few tarballs need special handling.  That is done here.

=cut
sub ProvideSpecialTarball ($$$)
{
    my $url = shift;
    my $name = shift;
    my $name_converter = shift;

    return unless defined $url && $url ne "";

    # See if we can find the executable.
    my ($SOLARENV,$OUTPATH,$EXEEXT) =  ($ENV{'SOLARENV'},$ENV{'OUTPATH'},$ENV{'EXEEXT'});
    $SOLARENV = "" unless defined $SOLARENV;
    $OUTPATH = "" unless defined $OUTPATH;
    $EXEEXT = "" unless defined $EXEEXT;
    if (-x File::Spec->catfile($SOLARENV, $OUTPATH, "bin", $name.$EXEEXT))
    {
        print "found $name executable\n";
        return;
    }

    # Download the source from the URL.
    my $basename = basename(URI->new($url)->path());
    die unless defined $basename;

    if (defined $name_converter)
    {
        $basename = &{$name_converter}($basename);
    }

    # Has the source tar ball already been downloaded?
    my @candidates = glob(File::Spec->catfile($ENV{'TARFILE_LOCATION'}, "*-" . $basename));
    if (scalar @candidates > 0)
    {
        # Yes.
        print "$basename exists\n";
        return;
    }
    else
    {
        # No, download it.
        print "downloading $basename\n";
        DownloadFile($basename, $url, undef);
    }
}





# The main() functionality.

die "usage: $0 <data-file-name>" if scalar @ARGV != 1;
my $data_file = $ARGV[0];
CheckDownloadDestination();
ProcessDataFile($data_file);
ProvideSpecialTarball($ENV{'DMAKE_URL'}, "dmake", undef);
ProvideSpecialTarball(
    $ENV{'EPM_URL'},
    "epm",
    sub{$_[0]=~s/-source//; return $_[0]});
