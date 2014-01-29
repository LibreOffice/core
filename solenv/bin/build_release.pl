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
use XML::LibXML;
use open OUT => ":utf8";
use LWP::Simple;
use Digest;
use Digest::MD5;
use Digest::SHA;

use Carp::always;

=head1 NAME

    build_release.pl - Tool for batch release builds and uploads and the creation of wiki pages that list install sets.

=head1 SYNOPSIS

    build_release.pl <command> {option} <release-description.xml>

    comands:
        build      builds all install sets as requested by the XML file and supported by the platform.
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

=head1 XML file format

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
        base-url="http://people.apache.org/~hdu/developer-snapshots/snapshot/win32"
        archive-platform="Win_x86"
        word-size="32"
        package-types="exe"
        />
    ... more platforms

  <build
      target="openoffice"
      platform-list="all"
      language-list="all">
  </build>
  ... more build entries

  <wiki>

    # Each wiki package will generate one row per language.
    # For each language there will be one package block.
    <package
        display-name="Full Install"
        archive-name1=""
        archive-name2="install"
        language-list="all"
        platform-list="all"
        table="main"
        />
    ... more packages
  </wiki>

</release>

=cut



my %PlatformDescriptors = (
    "wntmsci12" => {
        'name' => "windows",
        'extension' => "exe",
        'pack-platform' => "Win_x86",
        'package-formats' => ["msi"]
    },
    "unxmaccxi" => {
        'name' => "unxmaccx_x86-64",
        'extension' => "dmg",
        'pack-platform' => "MacOS_x86",
        'package-formats' => ["dmg"]
    },
    "unxlngi6" => {
        'name' => "Linux_x86",
        'extension' => "tar.gz",
        'pack-platform' => "Linux_x86",
        'package-formats' => ["deb", "rpm"]
    },
    "unxlngx6" => {
        'name' => "Linux_x86-64",
        'extension' => "tar.gz",
        'pack-platform' => "Linux_x86-64",
        'package-formats' => ["deb", "rpm"]
    }
);
my %ProductDescriptors = (
    "openoffice" => {
        'pack-name' => "install",
        'product-name-tail' => ""
    },
    "oolanguagepack" => {
        'pack-name' => "langpack",
        'product-name-tail' => "_languagepack"
    }
);
my %EnUSBasedLanguages = (
    'ast' => 1
    );



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
            else
            {
                printf STDERR "unknown option $argument %s\n", $argument;
                $error = 1;
            }
        }
        elsif ( ! defined $command)
        {
            $command = $argument;
            if ($command !~ /^(build|upload|wiki)$/)
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
    if ($command =~ /^(build|wiki)$/)
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
        'output-filename' => $output_filename
    };
}




sub PrintUsageAndExit ()
{
    print STDERR "usage: $0 <command> {option} <release-description.xml>\n";
    print STDERR "    comands:\n";
    print STDERR "        build\n";
    print STDERR "        upload\n";
    print STDERR "        wiki     create a download page in MediaWiki syntax\n";
    print STDERR "    options:\n";
    print STDERR "    -j <count>    maximum number of build processes\n";
    print STDERR "    -k            keep going if there are recoverable errors\n";
    print STDERR "    -u <path>     upload destination\n";
    print STDERR "    -l            check links on wiki page, write broken links as plain text\n";
    print STDERR "    -ld           check links on wiki page, mark broken links\n";
    print STDERR "    -o <filename> filename of the output (wiki: wiki page, build: makefile)\n";
    exit(1);
}




sub Trim ($)
{
    my ($text) = @_;
    $text =~ s/^\s+|\s+$//g;
    return $text;
}




sub ReadReleaseDescription ($$)
{
    my ($filename, $context) = @_;

    my $document = XML::LibXML->load_xml('location' => $filename);
    my $root = $document->documentElement();

    # Initialize the release description.
    my $release = {
        'name' => $root->getAttribute("name"),
        'version' => $root->getAttribute("version"),
        'builds' => [],
        'languages' => {},
        'language-ids' => [],
        'platforms' => {},
        'platform-ids' => [],
        'wiki-packages' => [],
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
        foreach my $platform_descriptor (ProcessPlatformDescription($platform_element, $context))
        {
            $release->{'platforms'}->{$platform_descriptor->{'id'}} = $platform_descriptor;
            push @{$release->{'platform-ids'}}, $platform_descriptor->{'id'};
        }
    }
    printf "%d platforms\n", scalar keys %{$release->{'platforms'}};

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
            for my $wiki_package_element ($wiki_element->getChildrenByTagName("package"))
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




sub ProcessBuildDescription ($$$)
{
    my ($build_element, $context, $release_descriptor) = @_;

    my $target_name = $build_element->getAttribute("target");
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
        printf "including build %s\n", $target_name;
    }
    else
    {
        printf "skipping build %s: no platform match\n", $target_name;
        printf "none of the platforms %s matches %s\n",
            join(", ", keys %{$release_descriptor->{'platforms'}}),
            $current_platform;
        return;
    }

    my @languages = CheckLanguageSet($context, @$languages);

    return {
        'target' => $target_name,
        'language-list' => \@languages
    };
}




sub ProcessPlatformDescription ($$)
{
    my ($element, $context) = @_;

    my $descriptor = {};
    # Mandatory tags.
    foreach my $id ("id", "display-name", "base-url", "archive-platform", "word-size", "package-types")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "wiki/platform has no attribute $id" unless defined $descriptor->{$id};
    }
    $descriptor->{"extension"} = $element->getAttribute("extension");

    # Split package-types at ';' into single package-type entries.
    my @descriptors = ();
    foreach my $package_type (split(/;/, $descriptor->{'package-types'}))
    {
        push @descriptors, {
            %$descriptor,
            'package-type' => $package_type
        };
    }

    return @descriptors;
}




sub ProcessWikiPackageDescription ($$$)
{
    my ($element, $context, $release_descriptor) = @_;

    my $descriptor = {};
    # Mandatory tags.
    foreach my $id ("archive-name2", "display-name", "table")
    {
        $descriptor->{$id} = $element->getAttribute($id);
        die "wiki/package has no attribute $id" unless defined $descriptor->{$id};
        die "wiki/package attribute $id is empty" unless $descriptor->{$id} !~ /^\s*$/;
    }
    # Optional tags.
    foreach my $id ("archive-name1", "link-tooltip", "link-URL", "download-extension")
    {
        $descriptor->{$id} = $element->getAttribute($id);
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
        printf STDERR "    there are languages that where not configured via --with-lang:\n";
        printf STDERR "        %s\n", join(", ", @missing_languages);
        if ($context->{'keep-going'})
        {
            printf "    available languages:\n";
            printf "        %s\n", join(", ", @present_languages);
        }
        else
        {
            die;
        }
    }

    return @present_languages;
}




sub WriteMakefile ($$)
{
    my ($release_description, $output_filename) = @_;

    my $path = $ENV{'SRC_ROOT'} . "/instsetoo_native/util";
    open my $make, ">", $output_filename;

    # Write dependencies of 'all' on the products in all languages.
    print $make "all .PHONY : \\\n";
    for my $build (@{$release_description->{'builds'}})
    {
        for my $language_id (@{$build->{'language-list'}})
        {
            printf $make "    %s_%s \\\n", $build->{'target'}, $language_id;
        }
    }
    printf $make "\n\n";

    # Write rules that chain dmake in instsetoo_native/util.
    for my $build (@{$release_description->{'builds'}})
    {
        for my $language_id (@{$build->{'language-list'}})
        {
            printf $make "%s_%s :\n", $build->{'target'}, $language_id;
            printf $make "\tdmake \$@ release=t\n";
        }
    }

    close $make;
}




sub Upload ($$)
{
    my ($release_description, $context) = @_;

    if ( ! defined $context->{'upload-destination'})
    {
        printf STDERR "ERROR: upload destination is missing\n";
        PrintUsageAndExit();
    }

    my @download_sets = CollectDownloadSets($release_description);
    my @actions = GetCopyActions($release_description, @download_sets);
    foreach my $action (@actions)
    {
        printf "uploading %s to %s/%s\n",
            $action->{'basename'},
            $context->{'upload-destination'},
            $action->{'to'};


        ProvideChecksums($action);
        if ($context->{'upload-destination'} =~ /@/)
        {
            my $destination = $action->{'to'};
            my $server = $context->{'upload-destination'};
            if ($server =~ /^(.*):(.*)$/)
            {
                $server = $1;
                $destination = $2 . "/" . $destination;
            }

            my @path_parts = split(/\//, $destination);
            my @paths = ();
            my $path = undef;
            foreach my $part (@path_parts)
            {
                if (defined $path)
                {
                    $path .= "/" . $part;
                }
                else
                {
                    $path = $part;
                }
                push @paths, $path;
            }
            my $command = sprintf("ssh %s mkdir \"%s\"",
                $server,
                join("\" \"", @paths));
            printf "running command '%s'\n", $command;
            system($command);

            my $command = sprintf("scp %s %s/%s/",
                qx(cygpath -u \"$action->{'from'}\"),
                $context->{'upload-destination'},
                $action->{'to'});
            printf "running command '%s'\n", $command;
            system($command);

            my $command = sprintf("ssh %s md5 \"%s/%s\"",
                $server,
                $destination,
                $action->{'basename'});
            printf "running command '%s'\n", $command;
            system($command);
        }
    }
}




sub CollectDownloadSets ($)
{
    my ($release_description) = @_;

    my @download_sets = ();
    my $platform_descriptor = GetPlatformDescriptor();

    for my $build (@{$release_description->{'builds'}})
    {
        my $product_descriptor = GetProductDescriptor($build->{'target'});
        print $build->{'target'}."\n";
        my @package_formats = @{$platform_descriptor->{'package-formats'}};
        for my $package_format (@package_formats)
        {
            for my $language (@{$build->{'language-list'}})
            {
                my $full_language = $language;
                if ($EnUSBasedLanguages{$language})
                {
                    $full_language = "en-US_".$language;
                }
                my $archive_name = GetInstallationPackageName($build, $language);

                my $source_path = sprintf("%s/instsetoo_native/%s/Apache_OpenOffice%s/%s/install/%s_download",
                    $ENV{'SOLARSRC'},
                    $ENV{'INPATH'},
                    $product_descriptor->{'product-name-tail'},
                    $package_format,
                    $full_language);
                if ( ! -f $source_path."/".$archive_name)
                {
                    printf STDERR "ERROR: can not find download set '%s'\n", $source_path;
                    next;
                }
                push @download_sets, {
                    'source-path' => $source_path,
                    'archive-name' => $archive_name,
                    'platform' => $platform_descriptor->{'pack-platform'}
                };
            }
        }
    }

    return @download_sets;
}




sub ProvideChecksums ($)
{
    my ($action) = @_;

    printf "creating checksums for %s\n", $action->{'basename'};
    my $full_archive_name = $action->{'from'} . "/" . $action->{'basename'};

    my $digest = Digest::MD5->new();
    open my $in, $full_archive_name;
    $digest->addfile($in);
    $action->{"MD5"} = $digest->hexdigest();
    close $in;

    my $digest = Digest::SHA->new("sha256");
    open my $in, $full_archive_name;
    $digest->addfile($in);
    $action->{"SHA256"} = $digest->hexdigest();
    close $in;
}




sub GetCopyActions ($@)
{
    my ($release_description, @download_sets) = @_;

    my $platform_descriptor = GetPlatformDescriptor();

    my @actions = ();

    for my $download_set (@download_sets)
    {
        my $destination_path = sprintf("developer-snapshots/%s/%s",
            $release_description->{'name'},
            $platform_descriptor->{'pack-platform'});

        push @actions, {
            'action'=>'copy',
            'from' => $download_set->{'source-path'},
            'to' => $destination_path,
            'basename' => $download_set->{'archive-name'}
        };
    }

    return @actions;
}




sub GetInstallationPackageName ($$)
{
    my ($build, $language) = @_;

    my $platform_descriptor = GetPlatformDescriptor();
    my $build_descriptor = GetProductDescriptor($build->{'target'});
    my $name = sprintf ("Apache_OpenOffice_%s_%s_%s_%s.%s",
        "4.1.0",
        $platform_descriptor->{'pack-platform'},
        $build_descriptor->{'pack-name'},
        $language,
        $platform_descriptor->{'extension'});
    return $name;
}




sub GetPlatformDescriptor ()
{
    if ( ! defined $ENV{'OUTPATH'})
    {
        printf STDERR "ERROR: solar environment not loaded or broken (OUTPATH not defined)\n";
        die;
    }
    my $descriptor = $PlatformDescriptors{$ENV{'OUTPATH'}};
    if ( ! defined $descriptor)
    {
        printf STDERR "ERROR: platform '%s' is not yet supported\n", $ENV{'OUTPATH'};
        die;
    }
    return $descriptor;
}




sub GetProductDescriptor ($)
{
    my ($product_name) = @_;
    my $descriptor = $ProductDescriptors{$product_name};
    if ( ! defined $descriptor)
    {
        printf STDERR "ERROR: product '%s' is not supported\n", $product_name;
    }
    return $descriptor;
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

        printf "packages: %s\n", join(", ", map {$_->{'display-name'}} @table_packages);
        printf "languages: %s\n", join(", ", map {$_->{'english-name'}} @table_languages);
        printf "platforms: %s\n", join(", ", map {$_->{'id'}} @table_platforms);

        print $out "{| class=\"wikitable\"\n";
        print $out "|-\n";
        print $out "! colspan=\"2\" | Language<br>The names do not refer to countries\n";
        print $out "! Type\n";
        foreach my $platform_descriptor (@table_platforms)
        {
            printf $out "! %s<br>%s bit<br>%s\n",
                $platform_descriptor->{'display-name'},
                $platform_descriptor->{'word-size'},
                uc($platform_descriptor->{'package-type'});
        }

        foreach my $language_descriptor (@table_languages)
        {
            if ($context->{'check-links'})
            {
                $| = 1;
                printf "%s: ", $language_descriptor->{'id'};
            }

            print $out "|-\n";
            printf $out "| rowspan=\"%d\" | %s\n", scalar @table_packages, $language_descriptor->{'english-name'};
            printf $out "| rowspan=\"%d\" | %s\n", scalar @table_packages, $language_descriptor->{'local-name'};

            my $is_first = 1;
            foreach my $package_descriptor (@table_packages)
            {
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
                    WriteDownloadLinks(
                        $out,
                        $context,
                        $release_descriptor,
                        $language_descriptor,
                        $package_descriptor,
                        $platform_descriptor);
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
    foreach my $package_descriptor (@{$release_descriptor->{'wiki-packages'}})
    {
        my $table_name = $package_descriptor->{'table'};
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
    foreach my $package_descriptor (@{$release_descriptor->{'wiki-packages'}})
    {
        if ($package_descriptor->{'table'} eq $table_name)
        {
            push @packages, $package_descriptor;
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


sub WriteDownloadLinks ($$$$$)
{
    my ($out,
        $context,
        $release_descriptor,
        $language_descriptor,
        $package_descriptor,
        $platform_descriptor) = @_;

    # Check if the current language and platform match the package.
    if (defined $package_descriptor->{'platforms'}->{$platform_descriptor->{'id'}}
        && defined $package_descriptor->{'languages'}->{$language_descriptor->{'id'}})
    {
        my $archive_package_name = "";
        my $package_type = $platform_descriptor->{'package-type'};
        my $extension = $package_type;
        if (defined $platform_descriptor->{'extension'})
        {
            $extension = $platform_descriptor->{'extension'};
        }
        if (defined $package_descriptor->{'download-extension'})
        {
            $extension = $package_descriptor->{'download-extension'};
        }
        $archive_package_name = "-".$package_type if ($package_type =~ /deb|rpm/);
        my $archive_name = sprintf("Apache_OpenOffice%s_%s_%s_%s%s_%s.%s",
            $package_descriptor->{'archive-name1'},
            $release_descriptor->{'version'},
            $platform_descriptor->{'archive-platform'},
            $package_descriptor->{'archive-name2'},
            $archive_package_name,
            $language_descriptor->{'id'},
            $extension);

        my $url = $platform_descriptor->{'base-url'} . "/". $archive_name;
        printf $out
            "| align=\"center\" | %s%s%s<br><br>%s%s %s<br>%s%s\n",
            $bold_text_start,
            CreateLink($url, $extension, $context),
            $bold_text_end,
            $small_text_start,
            CreateLink($url.".asc", "ASC", $context),
            CreateLink($url.".md5", "MD5", $context),
            CreateLink($url.".sha256", "SHA256", $context),
            $small_text_end;
    }
    else
    {
        printf $out "|\n";
    }
}




sub CreateLink ($$$)
{
    my ($url, $text, $context) = @_;

    my $is_link_broken = 0;
    if ($context->{'check-links'})
    {
        my $head = LWP::Simple::head($url);
        $is_link_broken = ! $head;
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




my $context = ProcessCommandline(@ARGV);
my $release_description = ReadReleaseDescription($context->{'filename'}, $context);
if ($context->{'command'} eq "build")
{
    WriteMakefile($release_description, $context->{'output-filename'});
#    open my $make, "|-", sprintf("make -C \"%s\" -j%d -f -", $path, $max_process_count);
}
elsif ($context->{'command'} eq "upload")
{
    Upload($release_description, $context);
}
elsif ($context->{'command'} eq "wiki")
{
    Wiki($release_description, $context);
}
