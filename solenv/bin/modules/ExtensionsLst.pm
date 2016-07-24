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

package ExtensionsLst;

use File::Spec;
use LWP::UserAgent;
use Digest::MD5;

use base 'Exporter';
our @EXPORT = qw(DownloadExtensions GetExtensionList);


=head1 NAME

    ExtensionLst.pm - Functionality for the interpretation of the main/extensions.lst file.

=head1 SYNOPSIS

    For downloading extensions during build setup:

    use ExtensionsLst;
    ExtensionsLst::DownloadExtensions();

    For including extensions into the pack set:

    use ExtensionsLst;
    ExtensionsLst::GetExtensionList(@language_list);

=head1 DESCRIPTION

    The contents of the extensions.lst file are used at two times in
    the process of building pack sets.

    Once at the beginning right after configure is run the
    DownloadExtensions() function determines the list of extensions
    that are not present locally and downloads them.

    The second time is after all modules are built (and the locally
    built extensions are present) and the pack sets are created.  For
    every language (or sets of lanugages) a set of extensions is
    collected and included into the pack set.

    The content of the extensions.lst file is ignored when the --with-extensions option is given to configure.

=cut


# Number of the line in extensions.lst that is currently being processed.
my $LineNo = 0;

# Set to 1 to get a more verbose output, the default is 0.
my $Debug = 0;


=head3 Prepare
    Check that some environment variables are properly set and then return the file name
    of the 'extensions.lst' file, typically located in main/ beside 'ooo.lst'.
=cut
sub Prepare ()
{
    die "can not access environment varianle SRC_ROOT" if ! defined $ENV{'SRC_ROOT'};
    die "can not determine the platform: INPATH is not set" if ! defined $ENV{'INPATH'};
    die "can not determine solver directory: OUTDIR is not set" if ! defined $ENV{'OUTDIR'};
    die "can not determine download directory: TARFILE_LOCATION is not set" if ! defined $ENV{'TARFILE_LOCATION'};

    my $candidate = File::Spec->catfile($ENV{SRC_ROOT}, "extensions.lst");
    die "can not read file $candidate" if ! -r $candidate;
    return $candidate;
}



=head 3 EvaluateOperator
    Evaluate a single test statement like 'language = en.*'.
    Special handling for operators '=', '==', and 'eq' which are all mapped to '=~'.
    Therefore the right hand side may be a perl regexp.  It is prefixed with '^'.

    Other operators are at the moment only supported in the way that they are evaluated via eval().
=cut
sub EvaluateOperator ($$$)
{
    my ($left,$operator,$right) = @_;

    my $result;

    if ($operator =~ /^(=|==|eq)$/)
    {
        if ($left =~ /^$right$/)
        {
            $result = 1;
        }
        else
        {
            $result = 0;
        }
    }
    elsif (eval($left.$operator.$right))
    {
        $result = 1;
    }
    else
    {
        $result = 0;
    }

    return $result;
}




=head EvaluateTerm
    Evaluate a string that contains a simple test term of the form
    left operator right
    with arbitrary spacing allowed around and between the three parts.

    The left hand side is specially handled:

    - When the left hand side is 'language' then it is replaced by
    any of the given languages in turn.  When the term evaluates to true for any of the languages then
    true is returned.  False is returned only when none of the given languages matches.

    - When the left hand side consists only of upper case letters, digits, and '_' then it is
    interpreted as the name of a environment variable.  It is replaced by its value before the term
    is evaluated.

    - Any other left hand side is an error (at the moment.)
=cut
sub EvaluateTerm ($$)
{
    my $term = shift;
    my $languages = shift;

    my $result;

    if ($term =~ /^\s*(\w+)\s*(\W+)\s*(.*?)\s*$/)
    {
        my ($left,$operator,$right) = ($1,$2,$3);

        if ($operator !~ /^=|==|eq$/)
        {
            die "unsupported operator $operator on line $LineNo";
        }

        die "no right side in condition on line $LineNo ($term)" if ! defined $right;

        if ($left =~ /^[A-Z_0-9]+$/)
        {
            # Uppercase words are interpreted as environment variables.
            my $left_value = $ENV{$left};
            $left_value = "" if ! defined $left_value;

            # We can check whether the condition is fulfilled right now.
            $result = EvaluateOperator($left_value, $operator, $right);
        }
        elsif ($left eq "language")
        {
            if ($right eq "all")
            {
                $result = 1;
            }
            elsif ($#$languages>=0)
            {
                $result = 0;
                for my $language (@$languages)
                {
                    # Unify naming schemes.
                    $language =~ s/_/-/g;
                    $right =~ s/_/-/g;

                    # Evaluate language regexp.
                    $result = EvaluateOperator($language, $operator, $right) ? 1 : 0;
                    last if $result;
                }
            }
            else
            {
                # The set of languages is not yet known.  Return true
                # to include the following entries.
                $result = 1;
            }
        }
        elsif ($left eq "platform")
        {
            if ($right eq "all")
            {
                $result = 1;
            }
            else
            {
                # Evaluate platform regexp.
                $result = EvaluateOperator($ENV{'INPATH'}, $operator, $right) ? 1 : 0;
            }
        }
        else
        {
            die "can not handle left hand side $left on line $LineNo";
        }
    }
    else
    {
        die "syntax error in expression on line $LineNo";
    }

    return $result;
}




=head3 EvaluateSelector
    Evaluate the given expression that is expected to be list of terms of the form
        left-hand-side operator right-hand-side
    that are separated by logical operators
        && ||
    The expression is lazy evaluated left to right.
=cut
sub EvaluateSelector($$);
sub EvaluateSelector($$)
{
    my $expression = shift;
    my $languages = shift;

    my $result = "";

    if ($expression =~ /^\s*$/)
    {
        # Empty selector is always true.
        return 1;
    }
    elsif ($expression =~ /^\s*(.*?)(&&|\|\|)\s*(.*)$/)
    {
        my ($term, $operator) = ($1,$2);
        $expression = $3;

        my $left_result = EvaluateTerm($term, $languages);
        # Lazy evaluation of &&
        return 0 if ($operator eq "&&" && !$left_result);
        # Lazy evaluation of ||
        return 1 if ($operator eq "||" && $left_result);
        my $right_result = EvaluateSelector($expression, $languages);

        if ($operator eq "&&")
        {
            return $left_result && $right_result;
        }
        else
        {
            return $left_result || $right_result;
        }
    }
    elsif ($expression =~ /^\s*(.+?)\s*$/)
    {
        return EvaluateTerm($1, $languages);
    }
    else
    {
        die "invalid expression syntax on line $LineNo ($expression)";
    }
}




=head3 ProcessURL
    Check that the given line contains an optional MD5 sum followed by
    a URL for one of the protocols file, http, https,
    followed by an optional file name (which is necessary when it is not the last part of the URL.)
    Return an array that contains the protocol, the name, the original
    URL, and the MD5 sum from the beginning of the line.
    The name of the URL depends on its protocol:
    - for http(s) the part of the URL after the last '/'.
    - for file URLS it is everything after the protocol://
=cut
sub ProcessURL ($)
{
    my $line = shift;

    # Check that we are looking at a valid URL.
    if ($line =~ /^\s*((\w{32})\s+)?([a-zA-Z]+)(:\/\/.*?\/)([^\/ \t]+)(\s+\"[^\"]+\")?\s*$/)
    {
        my ($md5, $protocol, $url_name, $optional_name) = ($2,$3,$5,$6);
        my $URL = $3.$4.$5;

        die "invalid URL protocol on line $LineNo:\n$line\n" if $protocol !~ /(file|http|https)/;

        # Determine the name.  If an optional name is given then use that.
        if (defined $optional_name)
        {
            die if $optional_name !~ /^\s+\"([^\"]+)\"$/;
            $name = $1;
        }
        else
        {
            if ($protocol eq "file")
            {
                # For file URLs we use everything after :// as name, or the .
                $URL =~ /:\/\/(.*)$/;
                $name = $1;
            }
            else
            {
                # For http and https use the last part of the URL.
                $name = $url_name;
            }
        }

        return [$protocol, $name, $URL, $md5];
    }
    else
    {
        die "invalid URL at line $LineNo:\n$line\n";
    }
}




=head3 ParseExtensionsLst
    Parse the extensions.lst file.

    Lines that contain only spaces or comments or are empty are
    ignored.

    Lines that contain a selector, ie a test enclosed in brackets, are
    evaluated.  The following lines, until the next selector, are
    ignored when the selector evaluates to false.  When an empty list
    of languages is given then any 'language=...' test is evaluated as
    true.

    All other lines are expected to contain a URL optionally preceded
    by an MD5 sum.
=cut
sub ParseExtensionsLst ($$)
{
    my $file_name = shift;
    my $languages = shift;

    open my $in, "$file_name";

    my $current_selector_value = 1;
    my @URLs = ();

    while (<$in>)
    {
        my $line = $_;
        $line =~ s/[\r\n]+//g;
        ++$LineNo;

        # Strip away comments.
        next if $line =~ /^\s*#/;

        # Ignore empty lines.
        next if $line =~ /^\s*$/;

        # Process selectors
        if ($line =~ /^\s*\[\s*(.*)\s*\]\s*$/)
        {
            $current_selector_value = EvaluateSelector($1, $languages);
        }
        else
        {
            if ($current_selector_value)
            {
                push @URLs, ProcessURL($line);
            }
        }
    }

    close $in;

    return @URLs;
}




=head3 Download
    Download a set of files that are specified via URLs.

    File URLs are ignored here because they point to extensions that have not yet been built.

    For http URLs there may be an optional MD5 checksum.  If it is present then downloaded
    files that do not match that checksum are an error and lead to abortion of the current process.
    Files that have already been downloaded are not downloaded again.
=cut
sub Download (@)
{
    my @urls = @_;

    my @missing = ();
    my $download_path = $ENV{'TARFILE_LOCATION'};

    # First check which (if any) extensions have already been downloaded.
    for my $entry (@urls)
    {
        my ($protocol, $name, $URL, $md5sum) = @{$entry};

        # We can not check the existence of file URLs because they point to extensions that
        # have yet to be built.

        next if $protocol ne "http";
        my $candidate = File::Spec->catfile($download_path, $name);
        if ( ! -f $candidate)
        {
            push @missing, $entry;
        }
        elsif (defined $md5sum)
        {
            # Check that the MD5 sum is still correct.
            # The datafile may have been updated with a new version of the extension that
            # still has the same name but a different MD5 sum.
            my $cur_oxt;
            if ( ! open($cur_oxt, $candidate))
            {
                # Can not read the extension.  Download extension again.
                push @missing, $entry;
                unlink($candidate);
            }
            binmode($cur_oxt);
            my $file_md5 = Digest::MD5->new->addfile(*$cur_oxt)->hexdigest;
            close($cur_oxt);
            if ($md5sum ne $file_md5)
            {
                # MD5 does not match.  Download extension again.
                print "extension $name has wrong MD5 and will be updated\n";
                push @missing, $entry;
                unlink($candidate);
            }
        }
    }
    if ($#missing >= 0)
    {
        printf "downloading/updating %d extension%s\n", $#missing+1, $#missing>0 ? "s" : "";
        if ( ! -d $download_path)
        {
            mkdir File::Spec->catpath($download_path, "tmp")
                || die "can not create tmp subdirectory of $download_path";
        }
    }
    else
    {
        print "all downloadable extensions present\n";
        return;
    }

    # Download the missing files.
    for my $entry (@missing)
    {
        my ($protocol, $name, $URL, $md5sum) = @{$entry};

        system(
            $ENV{'JAVAINTERPRETER'},
            "-cp",
            File::Spec->catfile(
                File::Spec->catfile($ENV{'SOLARENV'}, $ENV{'INPATH'}),
                "class"),
            "AOOJavaDownloader",
            $name,
            $URL,
            'MD5',
            $md5sum);

        if ($? != 0)
        {
            die "failed to download $URL";
        }
    }
}




=head3 DownloadExtensions
    This function is intended to be called during bootstrapping.  It extracts the set of extensions
    that will be used later, when the installation sets are built.
    The set of languages is taken from the WITH_LANG environment variable.
=cut
sub DownloadExtensions ()
{
    if (defined $ENV{'ENABLE_BUNDLED_DICTIONARIES'}
         && $ENV{'ENABLE_BUNDLED_DICTIONARIES'} eq "YES")
    {
        my $full_file_name = Prepare();
        my $languages = [ "en_US" ];
        if (defined $ENV{'WITH_LANG'})
        {
            @$languages = split(/\s+/, $ENV{'WITH_LANG'});
            foreach my $l (@$languages)
            {
                print "$l\n";
            }
        }
        my @urls = ParseExtensionsLst($full_file_name, $languages);
        Download(@urls);
    }
    else
    {
        print "bundling of dictionaries is disabled.\n";
    }
}




=head3 GetExtensionList
    This function is intended to be called when installation sets are built.
    It expects two arguments:
        - A protocol selector.  Http URLs reference remotely located
          extensions that will be bundled as-is into the installation
          sets due to legal reasons. They are installed on first start
          of the office.
          File URLs reference extensions whose source code is part of
          the repository.  They are pre-registered when installation
          sets are created.  Their installation is finished when the
          office is first started.
        - A set of languages.  This set determines which extensions
          are returned and then included in an installation set.
=cut
sub GetExtensionList ($@)
{
    my $protocol_selector = shift;
    my @language_list = @_;

    if (defined $ENV{'ENABLE_BUNDLED_DICTIONARIES'}
         && $ENV{'ENABLE_BUNDLED_DICTIONARIES'} eq "YES")
    {
        my $full_file_name = Prepare();
        my @urls = ParseExtensionsLst($full_file_name, \@language_list);

        my @result = ();
        for my $entry (@urls)
        {
            my ($protocol, $name, $URL, $md5sum) = @{$entry};
            if ($protocol =~ /^$protocol_selector$/)
            {
                push @result, $name;
            }
        }

        return @result;
    }
    else
    {
        # Bundling of dictionaires is disabled.
    }

    return ();
}


1;
