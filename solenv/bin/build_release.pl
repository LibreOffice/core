#!/usr/bin/perl
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

use strict;
use warnings;
use XML::LibXML;
use open OUT => ":utf8";
use LWP::Simple;
use Digest;
use Digest::MD5;
use Digest::SHA;
use File::Temp;
use File::Path;

use Carp::always;

=head1 NAME

    build_release.pl - Tool for batch release builds and uploads and the creation of wiki pages that list install sets.

=head1 SYNOPSIS

    build_release.pl <command> {option} <release-description.xml>

    comands:
        build      builds all install sets as requested by the XML file and supported by the platform.
        build-missing
                   build only those install sets that have not been built earlier.
        upload     upload install sets to a local or remote (via ssh with public/private key)
                   directory structure.  Uploads install sets that where build on other platforms.
        wiki       create a wiki (MediaWiki syntax) snippet that references all install sets at the upload
                   location.  Includes install sets that where built and/or uploaded from other
                   platforms and machines.
    options:
        -j <count>    maximum number of build processes
        -k            keep going if there are recoverable errors
        -u <path>     upload destination
        -l            check links on wiki page, write broken links as plain text
        -ld           check links on wiki page, mark broken links
        -o <filename> filename of the output (wiki: wiki page, build: makefile)
        -n <number>   maximal number of upload tries, defaults to 5.
        -d            dry-run

    Typical calls are:
       build_release.pl build -j4 instsetoo_native/util/aoo-410-release.xml
           for building the installation sets, language packs and patches for the 4.1 release.

       build_release.pl upload -u me@server:path -n 3 instsetoo_native/util/aoo-410-release.xml
           to upload the previously built installation sets etc.

       build_release.pl wiki -o /tmp/wiki.txt instsetoo_native/util/aoo-410-release.xml
           to create an updated wiki page with installation sets etc built at several
           places and uploaded to several locations.


=head1 XML file format

The release description could look like this:

<release
    name="snapshot"
    version="4.1.0">

    <language
        id="ast"                            # As specified by 'configure --with-lang'
        english-name="Asturian"
        local-name="Asturianu"
        />
    ... more languages

    <platform
        id="wntmsci12.pro"
        display-name="Windows"
        archive-platform="Win_x86"
        word-size="32"
        package-types="msi"
        extension="exe"
        />
    ... more platforms

    <download
        platform-id="wntmsci12.pro"
        base-url="http://people.apache.org/~somebody/developer-snapshots/snapshot/win32"
        />

    <package
        id="openoffice"
        target="openoffice"
        display-name="Full Install"
        archive-name="Apache_OpenOffice_%V_%P_install%T_%L.%E"
        />

    <build
        package-id="openoffice"
        platform-list="all"
        language-list="all"
        />
    ... more build entries

  <wiki>
    <package-ref
        package-id="openoffice"
        language-list="all"
        platform-list="all"
        table="main"
        />
    ... more packages
  </wiki>

</release>

A single <release> tag contains any number of

<language>  id
                The language id used internally by the build process, eg de, en-US
            english-name
                The english name of the language, eg german
            local-name
                The language name in that language, eg Deutsch

    Each listed language is expected to have been passed to configure via --with-lang
    The set of languages defines for which languages to
          build installation sets, language packs etc. (build command)
          upload installation sets, etc (upload command)
          add rows in the wiki page (wiki command)

<platform>  id
                The platform id that is used internally by the build process, eg wntmsci12.pro
                Note that <p>.pro and <p> are treated as two different platforms.
            display-name
                Name which is printed in the wiki table.
            archive-platform
                Platform name as used in the name of the installation set, eg Win_x86
            word-size
                Bit size of the installation sets, etc, typically either 32 or 64
            package-types
                Semicolon separated list of package types, eg "msi" or "deb;rpm"
            add-package-type-to-archive-name
                For deb and rpm archives it is necessary to add the package type to the archive name.
            extension
                Extension of the archive name, eg "exe" or "tar.gz"

    For the build command only those <platform> elements are used that match the platform on which this
    script is run.

<download>
            platform-id
                Reference to one of the <platform> elements and has to match the id attribute of that platform.
            base-url
                URL head to which the name of the downloadable installation set etc. is appended.
                Eg. http://people.apache.org/~somebody/developer-snapshots/snapshot/win32

    Defines one download source that is referenced in the wiki page.  Multiple <download> elements
    per platform are possible.  Earlier entires are preferred over later ones.

<package>
            id
                Internal name that is used to reference the package.
            target
                Target name recognized by instsetoo_native/util/makefile.mk, eg openoffice or oolanguagepack.
            display-name
                Name of the package that is shown in the wiki page, eg "Full Install" or "Langpack".
            archive-name
                Template of the archive name.
                %V version
                %P archive package name
                %T package type
                %L language
                %E extension.

    Defines a downloadable and distributable package, eg openoffice for the binary OpenOffice installation set.

<build>     target
            platform-list
                Semicolon separated list of platforms for which to build the target.
                Ignores all platforms that don't match the  platform on which this script is executed.
                The special value 'all' is a shortcut for all platforms listed by <platform> elements.
            language-list
                Semicolon separated list of languages for which the build the target.
                The special value 'all' is a shortcut for all languages listed by <language> elements.

    Defines the sets of targets, plaforms and languages which are to be built.

<wiki>
    <package-ref>
            package-id
                The id of the referenced package.
            platform-list
                See <build> tag for explanation.
            language-list
                See <build> tag for explanation.
            table
                Specifies the wiki table into which to add the package lines.  Can be "main" or "secondary".

=cut



my %EnUSBasedLanguages = (
    'ast' => 1
    );


sub ProcessCommandline (@);
sub PrintUsageAndExit ();
sub Trim ($);
sub ReadReleaseDescription ($$);
sub ProcessBuildDescription ($$$);
sub ProcessPlatformDescription ($$);
sub ProcessDownloadDescription ($$);
sub ProcessPackageDescription ($$);
sub ProcessWikiPackageDescription ($$$);
sub ProcessLanguageDescription ($$);
sub PostprocessLanguageList ($$);
sub PostprocessPlatformList ($$);
sub CheckLanguageSet ($@);
sub WriteMakefile ($$);
sub Upload ($$);
sub PrepareUploadArea ($@);
sub UploadFilesViaRsync ($$@);
sub CollectDownloadSets ($);
sub ProvideChecksums ($@);
sub IsOlderThan ($$);
sub GetInstallationPackageName ($$$$);
sub ResolveTemplate ($$$$$);
sub GetCurrentPlatformDescriptor ($);
sub Wiki ($$);
sub GetTableList ($);
sub GetPackagesForTable ($$);
sub GetLanguagesForTable ($@);
sub GetPlatformsForTable ($@);
sub WriteDownloadLinks ($$$$$$$);
sub FindDownload ($$$$$);
sub CreateLink ($$$);
sub CheckLink ($);
sub SignFile ($$);

sub ProcessCommandline (@)
{
    my @arguments = @_;

    my $command = undef;
    my $description_filename = undef;
    my $max_process_count = 1;
    my $keep_going = 0;
    my $upload_destination = undef;
    my $check_links = 0;
    my $mark_broken_links = 0;
    my $output_filename = undef;
    my $max_upload_count = 5;
    my $build_only_missing = 0;
    my $dry_run = 0;

    my $error = 0;
    while (scalar @arguments > 0)
    {
        my $argument = shift @arguments;
        if ($argument =~ /^-/)
        {
            if ($argument eq "-j")
            {
                $max_process_count = shift @arguments;
            }
            elsif ($argument eq "-u")
            {
                $upload_destination = shift @arguments;
                $upload_destination =~ s/(\\|\/)$//;
            }
            elsif ($argument eq "-k")
            {
                $keep_going = 1;
            }
            elsif ($argument eq "-l")
            {
                $check_links = 1;
            }
            elsif ($argument eq "-ld")
            {
                $check_links = 1;
                $mark_broken_links = 1;
            }
            elsif ($argument eq "-o")
            {
                $output_filename = shift @arguments;
            }
            elsif ($argument eq "-n")
            {
                $max_upload_count = shift @arguments;
            }
            elsif ($argument eq "-d")
            {
                $dry_run = 1;
            }
            else
            {
                printf STDERR "unknown option $argument %s\n", $argument;
                $error = 1;
            }
        }
        elsif ( ! defined $command)
        {
            $command = $argument;
            if ($command eq "build-missing")
            {
                $command = "build";
                $build_only_missing = 1;
            }
            elsif ($command !~ /^(build|build-missing|upload|wiki)$/)
            {
                printf STDERR "unknown command '%s'\n", $command;
                $error = 1;
            }
        }
        else
        {
            $description_filename = $argument;
            if ( ! -f $description_filename)
            {
                print STDERR "can not open release description '%s'\n", $description_filename;
                $error = 1;
            }
        }
    }

    if ( ! defined $description_filename)
    {
        $error = 1;
    }
    if (! defined $command)
    {
        printf STDERR "ERROR: no command\n";
        $error = 1;
    }
    elsif ($command =~ /^(wiki)$/)
    {
        if ( ! defined $output_filename)
        {
            printf STDERR "ERROR: no output filename\n",
            $error = 1;
        }
    }

    if ($error)
    {
        PrintUsageAndExit();
    }

    return {
        'command' => $command,
        'filename' => $description_filename,
        'max-process-count' => $max_process_count,
        'keep-going' => $keep_going,
        'upload-destination' => $upload_destination,
        'check-links' => $check_links,
        'mark-broken-links' => $mark_broken_links,
        'output-filename' => $output_filename,
        'max-upload-count' => $max_upload_count,
        'build-only-missing' => $build_only_missing,
        'dry-run' => $dry_run
    };
}




sub PrintUsageAndExit ()
{
    print STDERR "usage: $0 <command> {option} <release-description.xml>\n";
    print STDERR "    comands:\n";
    print STDERR "        build\n";
    print STDERR "        build-missing\n";
    print STDERR "        upload\n";
    print STDERR "        wiki     create a download page in MediaWiki syntax\n";
    print STDERR "    options:\n";
    print STDERR "    -j <count>    maximum number of build processes\n";
    print STDERR "    -k            keep going if there are recoverable errors\n";
    print STDERR "    -u <path>     upload destination\n";
    print STDERR "    -l            check links on wiki page, write broken links as plain text\n";
    print STDERR "    -ld           check links on wiki page, mark broken links\n";
    print STDERR "    -o <filename> filename of the output (wiki: wiki page, build: makefile)\n";
    print STDERR "    -n <number>   maximal number of upload tries, defaults to 5.\n";
    print STDERR "    -d            dry run\n";
    exit(1);
}




=head2 Trim ($text)

    Remove leading and trailing space from the given string.

=cut
sub Trim ($)
{
    my ($text) = @_;
    $text =~ s/^\s+|\s+$//g;
    return $text;
}




=head2 ReadReleaseDescription ($$)

    Read the release description from $filename.

=cut
sub ReadReleaseDescription ($$)
{
    my ($filename, $context) = @_;

    my $document = XML::LibXML->load_xml('location' => $filename);
    my $root = $document->documentElement();

    # Initialize the release description.
    my $release = {
        'name' => $root->getAttribute("name"),
        'version' => $root->getAttribute("version"),
        'previous-version' => $root->getAttribute("previous-version"),
        'builds' => [],
        'languages' => {},
        'language-ids' => [],
        'platforms' => {},
        'downloads' => [],
        'packages' => {},
        'platform-ids' => [],
        'wiki-packages' => []
    };

    # Process the language descriptions.
    for my $language_element ($root->getChildrenByTagName("language"))
    {
        my $language_descriptor = ProcessLanguageDescription($language_element, $context);
        $release->{'languages'}->{$language_descriptor->{'id'}} = $language_descriptor;
        push @{$release->{'language-ids'}}, $language_descriptor->{'id'};
    }
    printf "%d languages\n", scalar keys %{$release->{'languages'}};

    # Process the platform descriptions.
    for my $platform_element ($root->getChildrenByTagName("platform"))
    {
        my $platform_descriptor = ProcessPlatformDescription($platform_element, $context);
        $release->{'platforms'}->{$platform_descriptor->{'id'}} = $platform_descriptor;
        push @{$release->{'platform-ids'}}, $platform_descriptor->{'id'};
    }
    printf "%d platforms\n", scalar keys %{$release->{'platforms'}};

    # Process the package descriptions.
    for my $package_element ($root->getChildrenByTagName("package"))
    {
        my $package_descriptor = ProcessPackageDescription($package_element, $context);
        $release->{'packages'}->{$package_descriptor->{'id'}} = $package_descriptor;
    }
    printf "%d packages\n", scalar keys %{$release->{'packages'}};

    # Platform specific the package descriptions.
    for my $package_element ($root->getChildrenByTagName("platform-package"))
    {
        my $package_descriptor = ProcessPlatformPackageDescription($package_element, $context);
        my $key = $package_descriptor->{'platform-id'} . "/" . $package_descriptor->{'package-id'};
        $release->{'platform-packages'}->{$key} = $package_descriptor;
    }
    printf "%d platform packages\n", scalar keys %{$release->{'platform-packages'}};

    # Process the download descriptions.
    for my $download_element ($root->getChildrenByTagName("download"))
    {
        my $download_descriptor = ProcessDownloadDescription($download_element, $context);
        push @{$release->{'downloads'}}, $download_descriptor;
    }
    printf "%d downloads\n", scalar @{$release->{'downloads'}};

    if ($context->{'command'} =~ /^(build|upload)$/)
    {
        # Process the build descriptions.
        for my $build_element ($root->getChildrenByTagName("build"))
        {
            push @{$release->{'builds'}}, ProcessBuildDescription($build_element, $context, $release);
        }
        printf "%d build targets\n", scalar @{$release->{'builds'}};
    }

    if ($context->{'command'} eq "wiki")
    {
        for my $wiki_element ($root->getChildrenByTagName("wiki"))
        {
            for my $wiki_package_element ($wiki_element->getChildrenByTagName("package-ref"))
            {
                my $wiki_package = ProcessWikiPackageDescription(
                    $wiki_package_element,
                    $context,
                    $release);
                push @{$release->{'wiki-packages'}}, $wiki_package;
            }
        }
        printf "%d wiki packages\n", scalar @{$release->{'wiki-packages'}};
    }

    return $release;
}




=head ProcessBuildDescription ($build_element, $context, $release_descriptor)

    Process one <build> element.

    If its platform-list does not match the current platform then the <build> element is ignored.

=cut
sub ProcessBuildDescription ($$$)
{
    my ($build_element, $context, $release_descriptor) = @_;

    my $package_id = $build_element->getAttribute("package-id");
    my $languages = PostprocessLanguageList($build_element->getAttribute("language-list"), $release_descriptor);
    my $platforms = PostprocessPlatformList($build_element->getAttribute("platform-list"), $release_descriptor);

    # Check if the platform matches any for which the product shall be built.
    my $current_platform = $ENV{'INPATH'};
    my $is_platform_match = 0;
    foreach my $platform_id (@$platforms)
    {
        if ($platform_id eq $current_platform)
        {
            $is_platform_match=1;
            last;
        }
    }
    if ($is_platform_match)
    {
        printf "including build %s\n", $package_id;
    }
    else
    {
        printf "skipping build %s: no platform match\n", $package_id;
        printf "none of the platforms %s matches %s\n",
        join(", ", keys %{$release_descriptor->{'platforms'}}),
        $current_platform;
        return;
    }

    my @languages = CheckLanguageSet($context, @$languages);

    return {
        'package-id' => $package_id,
        'platform-list' => $platforms,
        'language-list' => \@languages
    };
}





=head2 ProcessPlatformDescription ($element, $context)

    Process one <platform> element.

    The corresponding platform descriptor is returned as a hash.

=cut
sub ProcessPlatformDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};
    # Mandatory tags.
    foreach my $id ("id", "display-name", "archive-platform", "word-size", "package-types")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "release/platform has no attribute $id" unless defined $descriptor->{$id};
    }
    # Optional tags.
    foreach my $id ("extension", "add-package-type-to-archive-name")
    {
        my $value = $element->getAttribute($id);
        $descriptor->{$id} = $value if defined $value;
    }

    $descriptor->{'add-package-type-to-archive-name'} = 0
        unless defined $descriptor->{'add-package-type-to-archive-name'};
    $descriptor->{'package-types'} = [split(/;/, $descriptor->{'package-types'})];

    return $descriptor;
}




=head2 ProcessDownloadDescription ($element, $context)

    Process one <download> element.

    The corresponding download descriptor is returned as a hash.

=cut
sub ProcessDownloadDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};

    # Mandatory tags.
    foreach my $id ("platform-id", "base-url")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "release/download has no attribute $id" unless defined $descriptor->{$id};
    }

    return $descriptor;
}




=head2 ProcessPackageDescription ($element, $context)

    Process one <package> element.

    The corresponding package descriptor is returned as a hash.

=cut
sub ProcessPackageDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};

    # Mandatory tags.
    foreach my $id ("id", "target", "archive-path", "archive-name", "display-name")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "release/package has no attribute $id" unless defined $descriptor->{$id};
        die "release/package attribute $id is empty" unless $descriptor->{$id} !~ /^\s*$/;
    }
    # Optional tags.
    foreach my $id ("link-tooltip", "link-URL", "download-extension")
    {
        my $value = $element->getAttribute($id);
        $descriptor->{$id} = $value if defined $value;
    }

    return $descriptor;
}




=head2 ProcessPlatformPackageDescription ($element, $context)

    Process one <platform-package> element.

    The corresponding package descriptor is returned as a hash.

=cut
sub ProcessPlatformPackageDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};

    # Mandatory tags.
    foreach my $id ("platform-id", "package-id")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "release/package has no attribute $id" unless defined $descriptor->{$id};
        die "release/package attribute $id is empty" unless $descriptor->{$id} !~ /^\s*$/;
    }
    # Optional tags.
    foreach my $id ("extension", "package-types")
    {
        my $value = $element->getAttribute($id);
        $descriptor->{$id} = $value if defined $value;
    }
    if (defined $descriptor->{'package-types'})
    {
        $descriptor->{'package-types'} = [split(/;/, $descriptor->{'package-types'})];
    }

    return $descriptor;
}




=head2 ProcessWikiPackageDescription ($element, $context)

    Process one <wiki><package-ref> element.

    The corresponding descriptor is returned as a hash.

=cut
sub ProcessWikiPackageDescription ($$$)
{
    my ($element, $context, $release_descriptor) = @_;

    my $descriptor = {};
    # Mandatory tags.
    foreach my $id ("package-id", "table")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "wiki/package-ref has no attribute $id" unless defined $descriptor->{$id};
        die "wiki/package-ref attribute $id is empty" unless $descriptor->{$id} !~ /^\s*$/;
    }

    $descriptor->{'language-list'} = PostprocessLanguageList(
        $element->getAttribute("language-list"),
        $release_descriptor);
    $descriptor->{'platform-list'} = PostprocessPlatformList(
        $element->getAttribute("platform-list"),
        $release_descriptor);

    $descriptor->{'languages'} = {map{$_=>1} @{$descriptor->{'language-list'}}};
    $descriptor->{'platforms'} = {map{$_=>1} @{$descriptor->{'platform-list'}}};

    return $descriptor;
}




=head2 ProcessLanguageDescription ($element, $context)

    Process one <language> element.

    The corresponding language descriptor is returned as a hash.

=cut
sub ProcessLanguageDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};
    foreach my $id ("id", "english-name", "local-name")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "wiki/language has no attribute $id" unless defined $descriptor->{$id};
    }

    return $descriptor;
}




=head2 PostprocessLanguageList ($language_list, $release_descriptor)

    Process a language list that is given as 'language-list' attribute to some tags.

    If the attribute is missing, ie $language_list is undef, or its value is "all",
    then the returned list of languages is set to all languages defined via <language> elements.

=cut
sub PostprocessLanguageList ($$)
{
    my ($language_list, $release_descriptor) = @_;

    my @matching_languages = ();
    if ( ! defined $language_list
        || $language_list eq "all")
    {
        @matching_languages = sort keys %{$release_descriptor->{'languages'}};
    }
    else
    {
        @matching_languages = split(/;/, $language_list);
    }

    return \@matching_languages;
}




=head2 PostprocessPlatformList ($platform_list, $release_descriptor)

    Process a platform list that is given as 'platform-list' attribute to some tags.

    If the attribute is missing, ie $platform_list is undef, or its value is "all",
    then the returned list of platforms is set to all platforms defined via <platform> elements.

=cut
sub PostprocessPlatformList ($$)
{
    my ($platform_list, $release_descriptor) = @_;

    my @matching_platforms = ();
    if ( ! defined $platform_list
        || $platform_list eq "all")
    {
        @matching_platforms = sort keys %{$release_descriptor->{'platforms'}};
    }
    else
    {
        @matching_platforms = split(/;/, $platform_list);
    }

    return \@matching_platforms;
}




=head2 CheckLanguageSet ($context, @languages)

    Compare the given list of languages with the one defined by the 'WITH_LANG' environment variable.

    This is to ensure that configure --with-lang was called with the same set of languages that are
    listed by the <language> elements.

=cut
sub CheckLanguageSet ($@)
{
    my ($context, @languages) = @_;
    my %configured_languages = map{$_=>1} split(/\s+/, $ENV{'WITH_LANG'});

    my @missing_languages = ();
    my @present_languages = ();
    for my $language (@languages)
    {
        if (defined $configured_languages{$language})
        {
            push @present_languages, $language;
        }
        else
        {
            push @missing_languages, $language;
        }
    }

    if (scalar @missing_languages > 0)
    {
        my $message_head = $context->{'keep-going'} ? "WARNING" : "ERROR";
        printf STDERR "%s: there are languages that where not configured via --with-lang:\n", $message_head;
        printf STDERR "%s:     %s\n", $message_head, join(", ", @missing_languages);
        if ($context->{'keep-going'})
        {
            printf "    available languages:\n";
            printf "        %s\n", join(", ", @present_languages);
        }
        else
        {
            printf STDERR "ERROR: please rerun configure with --with-lang=\"%s\"\n", join(" ", @languages);
            exit(1);
        }
    }

    return @present_languages;
}




=head2 WriteMakefile ($release_descriptor, $context)

    Write a makefile with all targets that match the <build> elements.

    The use of a makefile allows us to use make to handle concurrent building.

    When an output file was specified on the command line (option -o) then the
    makefile is written to that file but make is not run.

    When no output file was specified then the makefile is written to a temporary
    file.  Then make is run for this makefile.

=cut
sub WriteMakefile ($$)
{
    my ($release_descriptor, $context) = @_;

    my $filename = $context->{'output-filename'};
    if ( ! defined $filename)
    {
        $filename = File::Temp->new();
    }

    # Collect the targets to make.
    my @targets = ();
    foreach my $build (@{$release_descriptor->{'builds'}})
    {
        my $platform_descriptor = GetCurrentPlatformDescriptor($release_descriptor);
        my $package_descriptor = $release_descriptor->{'packages'}->{$build->{'package-id'}};
        my $platform_package_descriptor = GetPlatformPackage(
            $release_descriptor,
            $platform_descriptor,
            $package_descriptor);

        foreach my $language_id (@{$build->{'language-list'}})
        {
            foreach my $package_format (@{$platform_package_descriptor->{'package-types'}})
            {
                my $full_target = sprintf("%s_%s.%s",
                    $package_descriptor->{'target'},
                    $language_id,
                    $package_format);
                if ($context->{'build-only-missing'})
                {
                    my ($archive_path, $archive_name) = GetInstallationPackageName(
                        $release_descriptor,
                        $platform_package_descriptor,
                        $package_format,
                        $language_id);
                    my $candidate = $archive_path . "/" . $archive_name;
                    if (-f $candidate)
                    {
                        printf "download set for %s already exists, skipping\n", $full_target;
                        next;
                    }
                    else
                    {
                        printf "%s  %s\n", $archive_path, $archive_name;
                    }
                }
                push @targets, $full_target;
            }
        }
    }

    # Write the makefile.
    open my $make, ">", $filename;

    # Write dependencies of 'all' on the products in all languages.
    print $make "all .PHONY : \\\n    ";
    printf $make "%s\n", join(" \\\n    ", @targets);
    printf $make "\n\n";

    if ($context->{'dry-run'})
    {
        printf ("adding make fules for\n    %s\n", join("\n    ", @targets));
    }

    # Write rules that chain dmake in instsetoo_native/util.
    foreach my $target (@targets)
    {
        printf $make "%s :\n", $target;
        printf $make "\tdmake \$@ release=t\n";
    }
    close $make;


    if ( ! defined $context->{'output-filename'})
    {
        # Caller wants us to run make.
        my $path = $ENV{'SRC_ROOT'} . "/instsetoo_native/util";
        my $command = sprintf("make -f \"%s\" -C \"%s\" -j%d",
            $filename,
            $path,
            $context->{'max-process-count'});
        if ($context->{'dry-run'})
        {
            printf "would run %s\n", $command;
        }
        else
        {
            printf "running %s\n", $command;
            system($command);
        }
    }
}




sub Upload ($$)
{
    my ($release_descriptor, $context) = @_;

    if ( ! defined $context->{'upload-destination'})
    {
        printf STDERR "ERROR: upload destination is missing\n";
        PrintUsageAndExit();
    }

    my @download_sets = CollectDownloadSets($release_descriptor);

    ProvideChecksums($context, @download_sets);
    my $source_path = PrepareUploadArea($context, @download_sets);
    if ( ! defined $source_path)
    {
        exit(1);
    }
    if ( ! UploadFilesViaRsync($context, $source_path, @download_sets))
    {
        exit(1);
    }
}




=head2 PrepareUploadArea ($context, @download_sets)

    Create a temporary directory with the same sub directory strcuture that is requested in the upload location.
    The files that are to be uploaded are not copied but linked into this temporary directory tree.

    Returns the name of the temporary directory.

=cut
sub PrepareUploadArea ($@)
{
    my ($context, @download_sets) = @_;

    my $tmpdir = File::Temp->newdir();
    foreach my $download_set (@download_sets)
    {
        foreach my $extension ("", ".md5", ".sha256", ".asc")
        {
            my $basename = sprintf("%s%s", $download_set->{'archive-name'}, $extension);
            my $source_path = $download_set->{'source-path'};
            my $source = sprintf("%s/%s", $source_path, $basename);
            my $target_path = sprintf("%s/%s", $tmpdir, $download_set->{'destination-path'});
            my $target = sprintf("%s/%s", $target_path, $basename);
            if ($context->{'dry-run'})
            {
                printf "would create link for %s\n", $basename;
                printf "    %s\n", $source_path;
                printf " to %s\n", $target_path;
            }
            else
            {
                mkpath($target_path);
                unlink $target if ( -f $target);
                my $result = symlink($source, $target);
                if ($result != 1)
                {
                    printf "ERROR: can not created symbolic link to %s\n", $basename;
                    printf "       %s\n", $source;
                    printf "    -> %s\n", $target;
                    return undef;
                }
            }
        }
    }

    return $tmpdir;
}




sub UploadFilesViaRsync ($$@)
{
    my ($context, $source_path, @download_sets) = @_;


    # Collect the rsync flags.
    my @rsync_options = (
        "-L",         # Copy linked files
        "-a",         # Transfer the local attributes and modification times.
        "-c",         # Use checksums to compare source and destination files.
        "--progress", # Show a progress indicator
        "--partial",  # Try to resume a previously failed upload
        );

    # (Optional) Add flags for upload to ssh server
    my $upload_destination = $context->{'upload-destination'};
    if ($upload_destination =~ /@/)
    {
        push @rsync_options, ("-e", "ssh");
    }

    # Set up the rsync command.
    my $command = sprintf("rsync %s \"%s/\" \"%s\"",
        join(" ", @rsync_options),
        $source_path,
        $upload_destination);
    printf "%s\n", $command;

    if ($context->{'dry-run'})
    {
        printf "would run %s up to %d times\n", $command, $context->{'max-upload-count'};
    }
    else
    {
        # Run the command.  If it fails, repeat a number of times.
        my $max_run_count = $context->{'max-upload-count'};
        for (my $run_index=1; $run_index<=$max_run_count && scalar @download_sets>0; ++$run_index)
        {
            my $result = system($command);
            printf "%d %d\n", $result, $?;
            return 1 if $result == 0;
        }
        printf "ERROR: could not upload all files without error in %d runs\n", $max_run_count;
        return 0;
    }
}




sub CollectDownloadSets ($)
{
    my ($release_descriptor) = @_;

    my @download_sets = ();

    foreach my $platform_descriptor (values %{$release_descriptor->{'platforms'}})
    {
        my $platform_path = sprintf("%s/instsetoo_native/%s",
            $ENV{'SOLARSRC'},
            $platform_descriptor->{'id'});
        if ( ! -d $platform_path)
        {
            printf "ignoring missing %s\n", $platform_path;
            next;
        }
        for my $package_descriptor (values %{$release_descriptor->{'packages'}})
        {
            my $platform_package_descriptor = GetPlatformPackage(
                $release_descriptor,
                $platform_descriptor,
                $package_descriptor);
            my @package_formats = @{$platform_descriptor->{'package-types'}};
            for my $package_format (@package_formats)
            {
                for my $language_id (@{$release_descriptor->{'language-ids'}})
                {
                    my ($archive_path, $archive_name) = GetInstallationPackageName(
                        $release_descriptor,
                        $platform_package_descriptor,
                        $package_format,
                        $language_id);
                    my $candidate = $archive_path."/".$archive_name;
                    if ( ! -f $candidate)
                    {
#                        printf STDERR "ERROR: can not find download set '%s'\n", $candidate;
                        next;
                    }
                    printf "adding %s\n", $archive_name;
                    my $download_set = {
                        'source-path' => $archive_path,
                        'archive-name' => $archive_name,
                        'platform' => $platform_descriptor->{'archive-platform'},
                        'destination-path' => sprintf("developer-snapshots/%s/%s",
                            $release_descriptor->{'name'},
                            $platform_descriptor->{'archive-platform'})
                    };
                    printf "    %s\n", $download_set->{'destination-path'};
                    push @download_sets, $download_set;
                }
            }
        }
    }

    return @download_sets;
}




=head2 ProvideChecksums ($context, @download_sets)

    Create checksums in MD5 and SHA256 format and a gpg signature for the given download set.
    The checksums are not created when they already exists and are not older than the download set.

=cut
sub ProvideChecksums ($@)
{
    my ($context, @download_sets) = @_;

    my @asc_requests = ();
    foreach my $download_set (@download_sets)
    {
        printf "%s\n", $download_set->{'archive-name'};
        my $full_archive_name = $download_set->{'source-path'} . "/" . $download_set->{'archive-name'};
        $full_archive_name = Trim(qx(cygpath -u "$full_archive_name"));

        my $md5_filename = $full_archive_name . ".md5";
        if ( ! -f $md5_filename || IsOlderThan($md5_filename, $full_archive_name))
        {
            if ($context->{'dry-run'})
            {
                printf "    would create MD5\n";
            }
            else
            {
                my $digest = Digest::MD5->new();
                open my ($in), $full_archive_name;
                $digest->addfile($in);
                my $checksum = $digest->hexdigest();
                close $in;

                open my ($out), ">", $md5_filename;
                printf $out "%s *%s", $checksum, $download_set->{'archive-name'};
                close $out;

                printf "    created MD5\n";
            }
        }
        else
        {
            printf "    MD5 already exists\n";
        }

        my $sha256_filename = $full_archive_name . ".sha256";
        if ( ! -f $sha256_filename || IsOlderThan($sha256_filename, $full_archive_name))
        {
            if ($context->{'dry-run'})
            {
                printf "    would create SHA256\n";
            }
            else
            {
                my $digest = Digest::SHA->new("sha256");
                open my ($in), $full_archive_name;
                $digest->addfile($in);
                my $checksum = $digest->hexdigest();
                close $in;

                open my ($out), ">", $sha256_filename;
                printf $out "%s *%s", $checksum, $download_set->{'archive-name'};
                close $out;

                printf "    created SHA256\n";
            }
        }
        else
        {
            printf "    SHA256 already exists\n";
        }

        my $asc_filename = $full_archive_name . ".asc";
        if ( ! -f $asc_filename || IsOlderThan($asc_filename, $full_archive_name))
        {
            if ($context->{'dry-run'})
            {
                printf "    would create ASC\n";
            }
            else
            {
                # gpg seems not to be able to sign more than one file at a time.
                # Password has to be provided every time.
                my $command = sprintf("gpg --armor --detach-sig \"%s\"", $full_archive_name);
                print $command;
                my $result = system($command);
                printf "    created ASC\n";
            }
        }
        else
        {
            printf "    ASC already exists\n";
        }
    }
}




=head2 IsOlderThan ($filename1, $filename2)

    Return true (1) if the last modification date of $filename1 is older than (<) that of $filename2.

=cut
sub IsOlderThan ($$)
{
    my ($filename1, $filename2) = @_;

    my @stat1 = stat $filename1;
    my @stat2 = stat $filename2;

    return $stat1[9] < $stat2[9];
}




sub GetInstallationPackageName ($$$$)
{
    my ($release_descriptor, $platform_package_descriptor, $package_format, $language) = @_;

    my $path = ResolveTemplate(
        $platform_package_descriptor->{'archive-path'},
        $release_descriptor,
        $platform_package_descriptor,
        $package_format,
        $language);
    my $name = ResolveTemplate(
        $platform_package_descriptor->{'archive-name'},
        $release_descriptor,
        $platform_package_descriptor,
        $package_format,
        $language);

    return ($path, $name);
}




sub ResolveTemplate ($$$$$)
{
    my ($template, $release_descriptor, $platform_package_descriptor, $package_format, $language) = @_;

    my $archive_package_type = "";
    if ($platform_package_descriptor->{'add-package-type-to-archive-name'} =~ /^(1|true|yes)$/i)
    {
        $archive_package_type = "-".$package_format;
    }
    my $full_language = $language;
    if ($EnUSBasedLanguages{$language})
    {
        $full_language = "en-US_".$language;
    }
    my $extension = $platform_package_descriptor->{'download-extension'};
    if ( ! defined $extension)
    {
        $extension = $platform_package_descriptor->{'extension'};
    }

    my $old_to_new_version_dash = sprintf(
        "v-%s_v-%s",
        $release_descriptor->{'previous-version'},
        $release_descriptor->{'version'});
    $old_to_new_version_dash =~ s/\./-/g;
    my $old_to_new_version_dots = sprintf(
        "%s-%s",
        $release_descriptor->{'previous-version'},
        $release_descriptor->{'version'});


    my $name = $template;

    # Resolve %? template paramters.
    $name =~ s/%V/$release_descriptor->{'version'}/g;
    $name =~ s/%W/$old_to_new_version_dash/g;
    $name =~ s/%w/$old_to_new_version_dots/g;
    $name =~ s/%P/$platform_package_descriptor->{'archive-platform'}/g;
    $name =~ s/%t/$archive_package_type/g;
    $name =~ s/%T/$package_format/g;
    $name =~ s/%l/$full_language/g;
    $name =~ s/%L/$language/g;
    $name =~ s/%E/$extension/g;

    # Resolve $name environment references.
    while ($name =~ /^(.*?)\$([a-zA-Z0-9_]+)(.*)$/)
    {
        $name = $1 . $ENV{$2} . $3;
    }

    return $name;
}




sub GetCurrentPlatformDescriptor ($)
{
    my ($release_descriptor) = @_;

    my $platform_descriptor = $release_descriptor->{'platforms'}->{$ENV{'INPATH'}};
    if ( ! defined $platform_descriptor)
    {
        printf STDERR "ERROR: platform '%s' is not supported\n", $ENV{'INPATH'};
    }
    return $platform_descriptor;
}




sub GetPlatformPackage ($$$)
{
    my ($release_descriptor, $platform_descriptor, $package_descriptor) = @_;
    my $key = sprintf("%s/%s", $platform_descriptor->{'id'}, $package_descriptor->{'id'});

    my $platform_package = $release_descriptor->{'platform-packages'}->{$key};
    $platform_package = {}
        unless defined $platform_package;

    my $joined_descriptor = {
        %$platform_descriptor,
        %$package_descriptor,
        %$platform_package,
        'id' => $key,
        'platform-id' => $platform_descriptor->{'id'},
        'package-id' => $package_descriptor->{'id'}
    };
    return $joined_descriptor;
}




sub Wiki ($$)
{
    my ($release_descriptor, $context) = @_;

    open my $out, ">", $context->{'output-filename'};

    my @table_list = GetTableList($release_descriptor);
    foreach my $table_name (@table_list)
    {
        my @table_packages = GetPackagesForTable($release_descriptor, $table_name);
        my @table_languages = GetLanguagesForTable($release_descriptor, @table_packages);
        my @table_platforms = GetPlatformsForTable($release_descriptor, @table_packages);

        printf "packages: %s\n", join(", ", map {$_->{'package'}->{'display-name'}} @table_packages);
        printf "languages: %s\n", join(", ", map {$_->{'english-name'}} @table_languages);
        printf "platforms: %s\n", join(", ", map {$_->{'id'}} @table_platforms);

        print $out "{| class=\"wikitable\"\n";

        # Write the table head.
        print $out "|-\n";
        print $out "! colspan=\"2\" | Language<br>The names do not refer to countries\n";
        print $out "! Type\n";
        foreach my $platform_descriptor (@table_platforms)
        {
            foreach my $package_type (@{$platform_descriptor->{'package-types'}})
            {
                printf $out "! %s<br>%s bit<br>%s\n",
                $platform_descriptor->{'display-name'},
                $platform_descriptor->{'word-size'},
                uc($package_type);
            }
        }

        foreach my $language_descriptor (@table_languages)
        {
            if ($context->{'check-links'})
            {
                $| = 1;
                printf "%-5s: ", $language_descriptor->{'id'};
            }

            print $out "|-\n";
            printf $out "| rowspan=\"%d\" | %s\n", scalar @table_packages, $language_descriptor->{'english-name'};
            printf $out "| rowspan=\"%d\" | %s\n", scalar @table_packages, $language_descriptor->{'local-name'};

            my $is_first = 1;
            foreach my $wiki_package_descriptor (@table_packages)
            {
                my $package_descriptor = $wiki_package_descriptor->{'package'};

                if ($is_first)
                {
                    $is_first = 0;
                }
                else
                {
                    printf $out "|-\n";
                }

                # Write the name of the package, e.g. Full Install or Langpack.
                if (defined $package_descriptor->{'link-URL'})
                {
                    printf $out "| [%s %s]\n",
                    $package_descriptor->{'link-URL'},
                    $package_descriptor->{'display-name'};
                }
                else
                {
                    printf $out "| %s\n", $package_descriptor->{'display-name'};
                }

                foreach my $platform_descriptor (@table_platforms)
                {
                    my $platform_package_descriptor = GetPlatformPackage(
                        $release_descriptor,
                        $platform_descriptor,
                        $package_descriptor);

                    foreach my $package_type (@{$platform_package_descriptor->{'package-types'}})
                    {
                        WriteDownloadLinks(
                            $out,
                            $release_descriptor,
                            $context,
                            $language_descriptor,
                            $wiki_package_descriptor,
                            $platform_package_descriptor,
                            $package_type);
                    }
                }
            }

            if ($context->{'check-links'})
            {
                printf "\n";
            }
        }

        print $out "|}\n";
    }
    close $out;
}




sub GetTableList ($)
{
    my ($release_descriptor) = @_;

    my %seen_table_names = ();
    my @table_names = ();
    foreach my $wiki_package_descriptor (@{$release_descriptor->{'wiki-packages'}})
    {
        my $table_name = $wiki_package_descriptor->{'table'};
        if ( ! $seen_table_names{$table_name})
        {
            push @table_names, $table_name;
            $seen_table_names{$table_name} = 1;
        }
    }
    return @table_names;
}




sub GetPackagesForTable ($$)
{
    my ($release_descriptor, $table_name) = @_;

    my @packages = ();
    foreach my $wiki_package_descriptor (@{$release_descriptor->{'wiki-packages'}})
    {
        if ($wiki_package_descriptor->{'table'} eq $table_name)
        {
            my $package_descriptor = $release_descriptor->{'packages'}->{
                $wiki_package_descriptor->{'package-id'}};
            $wiki_package_descriptor->{'package'} = $package_descriptor;
            push @packages, $wiki_package_descriptor;
        }
    }
    return @packages;
}




sub GetLanguagesForTable ($@)
{
    my ($release_descriptor, @packages) = @_;

    # Find the languages that are reference by at least one package.
    my %matching_languages = ();
    foreach my $package_descriptor (@packages)
    {
        foreach my $language_id (@{$package_descriptor->{'language-list'}})
        {
            $matching_languages{$language_id} = 1;
        }
    }

    # Retrieve the language descriptors for the language ids.
    my @matching_language_descriptors = ();
    foreach my $language_id (@{$release_descriptor->{'language-ids'}})
    {
        if (defined $matching_languages{$language_id})
        {
            my $language_descriptor = $release_descriptor->{'languages'}->{$language_id};
            if (defined $language_descriptor)
            {
                push @matching_language_descriptors, $language_descriptor;
            }
        }
    }

    return @matching_language_descriptors;
}




sub GetPlatformsForTable ($@)
{
    my ($release_descriptor, @packages) = @_;

    # Find the platforms that are reference by at least one package.
    my %matching_platform_ids = ();
    foreach my $package_descriptor (@packages)
    {
        foreach my $platform_id (@{$package_descriptor->{'platform-list'}})
        {
            $matching_platform_ids{$platform_id} = 1;
        }
    }

    # Retrieve the platform descriptors for the plaform ids.
    my @matching_platform_descriptors = ();
    foreach my $platform_id (@{$release_descriptor->{'platform-ids'}})
    {
        if ($matching_platform_ids{$platform_id})
        {
        print $platform_id."\n";
            push @matching_platform_descriptors, $release_descriptor->{'platforms'}->{$platform_id};
        }
    }

    return @matching_platform_descriptors;
}




my $bold_text_start = "<b>";
my $bold_text_end = "</b>";
my $small_text_start = "<span style=\"font-size:80%\">";
my $small_text_end = "</span>";
my $broken_link_start = "<span style=\"color:#FF0000\">";
my $broken_link_end = "</span>";


sub WriteDownloadLinks ($$$$$$$)
{
    my ($out,
        $release_descriptor,
        $context,
        $language_descriptor,
        $wiki_package_descriptor,
        $platform_package_descriptor,
        $package_type) = @_;

    # Check if the current language and platform match the package.
    my $platform_id = $platform_package_descriptor->{'platform-id'};
    if (defined $wiki_package_descriptor->{'platforms'}->{$platform_id}
        && defined $wiki_package_descriptor->{'languages'}->{$language_descriptor->{'id'}})
    {
        my $archive_package_name = "";
        my $extension = $package_type;
        if (defined $platform_package_descriptor->{'extension'})
        {
            $extension = $platform_package_descriptor->{'extension'};
        }
        if (defined $platform_package_descriptor->{'download-extension'})
        {
            $extension = $platform_package_descriptor->{'download-extension'};
        }
        $archive_package_name = "-".$package_type if ($package_type =~ /deb|rpm/);

        my ($archive_path, $archive_name) = GetInstallationPackageName(
            $release_descriptor,
            $platform_package_descriptor,
            $package_type,
            $language_descriptor->{'id'});

        printf $out "| align=\"center\" | ";
        my $download = FindDownload(
            $context,
            $release_descriptor,
            $platform_package_descriptor,
            $package_type,
            $archive_name);
        if (defined $download)
        {
            my $url = $download->{'base-url'} . "/". $archive_name;
            printf $out "%s%s%s<br><br>%s%s %s<br>%s%s",
            $bold_text_start,
            CreateLink($url, $extension, $context),
            $bold_text_end,
            $small_text_start,
            CreateLink($url.".asc", "ASC", $context),
            CreateLink($url.".md5", "MD5", $context),
            CreateLink($url.".sha256", "SHA256", $context),
            $small_text_end;
        }
        printf $out "\n";
    }
    else
    {
        printf $out "|\n";
    }
}




sub FindDownload ($$$$$)
{
    my ($context,
        $release_descriptor,
        $platform_package_descriptor,
        $package_type,
        $archive_name) = @_;

    foreach my $download (@{$release_descriptor->{'downloads'}})
    {
        if ($download->{'platform-id'} eq $platform_package_descriptor->{'platform-id'})
        {
            my $url = $download->{'base-url'} . "/". $archive_name;
            if ($context->{'check-links'})
            {
                if (CheckLink($url))
                {
                    # URL points to an existing file.
                    printf "+";
                    return $download;
                }
                else
                {
                    # URL is broken.
                    # Try the next download area for the platform.
                    next;
                }
            }
            else
            {
                # Use the URL unchecked.  If there is more than one download area for the platform then only
                # the first is ever used.
                printf ".";
                return $download;
            }
        }
    }

    if ($context->{'check-links'})
    {
        printf "-";
    }

    return undef;
}




sub CreateLink ($$$)
{
    my ($url, $text, $context) = @_;

    my $is_link_broken = 0;
    if ($context->{'check-links'})
    {
        if (CheckLink($url))
        {
            $is_link_broken = 0;
            printf "+";
        }
        else
        {
            $is_link_broken = 1;
            printf "-";
        }
    }
    else
    {
        printf ".";
    }

    if ( ! $is_link_broken)
    {
        return sprintf ("[%s %s]", $url, $text);
    }
    elsif ($context->{'mark-broken-links'})
    {
        return sprintf ("%sbroken%s[%s %s]", $broken_link_start, $broken_link_end, $url, $text);
    }
    else
    {
        return sprintf ("%s", $text);
    }
}




=head2 CheckLink ($url)

    Check if the file referenced by $url can be downloaded.
    This is determined by downloading only the header.

=cut
my $LastCheckedURL = "";
my $LastCheckedResult = undef;
sub CheckLink ($)
{
    my ($url) = @_;

    if ($url ne $LastCheckedURL)
    {
        my $head = LWP::Simple::head($url);
        $LastCheckedURL = $url;
        $LastCheckedResult = !!$head;
    }

    return $LastCheckedResult;
}




sub SignFile ($$)
{
    my ($signature, $filename) = @_;

    my $command = sprintf(
        "gpg --armor --output %s.asc --detach-sig %s",
        $filename,
        $filename);
}




my $context = ProcessCommandline(@ARGV);
my $release_descriptor = ReadReleaseDescription($context->{'filename'}, $context);
if ($context->{'command'} eq "build")
{
    WriteMakefile($release_descriptor, $context);
}
elsif ($context->{'command'} eq "upload")
{
    Upload($release_descriptor, $context);
}
elsif ($context->{'command'} eq "wiki")
{
    Wiki($release_descriptor, $context);
}
