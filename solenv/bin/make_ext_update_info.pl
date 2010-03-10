:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#here the definition for d would be written into dependencies. The reason is that when the event handler
#for the element is called, we can only find out the namespace but not the prefix. So we cannot
#distinguish if the namespace is used because the element was prefixed or because it uses the default
#namespace.
use warnings;
use strict;

use XML::Parser;
use Getopt::Long;
use Carp;

sub getUpdateInfoFileName($);
sub writeUpdateInformationData($);
sub findAttribute($$);
sub getNotDefPrefs($$$);
sub collectPrefixes($$$$);
sub determineNsDefinitions($$$);
sub determineNsDefinitionForItem($$$);

my $inDescription = 0;
my $inDependencies = 0;
my $inIdentifier = 0;
my $inVersion = 0;
my $descNS = "http://openoffice.org/extensions/description/2006";
                   my $indent;
my $identifier;
my $version;

#contains prefixes and the corresponding namespaces which are used in the <dependencies>
#element and all children of the description.xml
my @usedNsInDependencies;

#Maps  prefix to namespaces which are valid in <dependencies>. That is, they are
#either defined in <dependencies> or in the hirarchy above <dependencies>
my %validPrefsInDep;
#Contains the prefixes which are defined in <dependencies>
my @newPrefsInDep;
#Contains the prefixes/namespaces which need to be defined in <dependencies> but which are currently
#not. For example a prefix is defined in the parent and is used in a child of <dependencies>
my %notDefInDep;

#prefix used in start and end element
my $prefix;

#The default namespace valid in <dependencies>
my $defNsInDep;
#The prefix which we use for the default namespace used in <dependencies>
my $generatedPrefix;

my $helptext =
"make_ext_update_info.pl produces an update information file for an extension. ".
"It will use a dummy URL as URL for the extension update unless a URL has been ".
"provided with the --update_url option. The name of the update ".
"information file, which must be provided with the --out switch, should be formed ".
"according to this scheme: \n\n".
"extension_identifier.update.xml\n\n".
"extension_identifier should correspond to the extension identifier. In some cases ".
"this may not be possible because the identifier may contain characters which are not ".
"allowd in file names.\n\n".
"usage:\n".
"perl make_ext_update_info.pl [--help][--update_url url] --out update_information_file description.xml \n\n".
"Options: \n".
"--help - prints the help message and exits \n".
"--out file - the update information file to be written including the path \n".
"--update-url url - inserts the url under the <update-download> element. It may be necessary to enclose the urls in quotes in case they contain characters such as \"?\". ".
"It can be used multiple times\n\n";

#handling of arguments
my $help = 0;
my $out;
my @update_urls;
if (!GetOptions('help|?' => \$help,
                'out=s' => \$out,
                'update-url=s'=> \@update_urls))
{
    print $helptext;
    exit -1;
}
my $cArgs = scalar @ARGV;
die "You need to provide a description.xml\n\n$helptext" if $cArgs ==0;
die "You need to provide the name of the update information file ".
    "with the --out switch.\n" unless ($out);
die "Too many arguments. \n\n$helptext" if $cArgs > 1;
print $helptext if $help;


#open the update information file for writing
my $FH;
open $FH, "> $out" or die $!;

#write the xml header and root element
print $FH '<?xml version="1.0" encoding="UTF-8"?>', "\n";
print $FH '<description xmlns="http://openoffice.org/extensions/update/2006"', "\n";
print $FH '    xmlns:xlink="http://www.w3.org/1999/xlink">', "\n";

#obtain from description.xml the data for the update information
writeUpdateInformationData($ARGV[0]);
#We will die if there is no <version> or <identifier> in the description.xml
die "Error: The description.xml does not contain a <identifier> element.\n" unless $identifier;
die "Error: The description.xml does not contain a <version> element. \n" unless $version;

#write the write the update-download element and the children.
#the indention of <update-download> corresponds to that of <version>
print $FH ' 'x$indent, '<update-download>', "\n";
#check if update-urls have been provided through --update-url option
if (scalar @update_urls)
{
    my $urlIndent = $indent > 8 ? 8 : 2 * $indent;
    #use provided urls
    for (@update_urls)
    {
        print $FH ' 'x$urlIndent, '<src xlink:href="'.$_.'" />', "\n";
    }
}
else
{
    #use dummy update url
    print $FH ' 'x8, '<src xlink:href="http://extensions.openoffice.org/testarea/dummy.oxt" />', "\n";
}
print $FH ' 'x$indent, '</update-download>', "\n";

print $FH '</description>', "\n";
close $FH;

exit 0;



sub start_handler
{
    my $parser = shift;
    my $name = shift;

    if ($name eq "description"
        && $descNS eq $parser->namespace($name))
    {
        $inDescription = 1;
    }
    elsif ($inDescription
           && $name eq "version"
           && $descNS eq  $parser->namespace($name))
    {
        $inVersion = 1;
        $version = 1;
        $indent = $parser->current_column();
        print $FH " "x$indent, $parser->original_string();
    }
    elsif ($inDescription
           && $name eq "identifier"
           && $descNS eq  $parser->namespace($name))
    {
        $inIdentifier = 1;
        $identifier = 1;
        print $FH " "x$parser->current_column(), $parser->original_string();
    }
    elsif ($inDescription
           && $name eq "dependencies"
           && $descNS eq  $parser->namespace($name))
    {
        $inDependencies = 1;
        my $dep = $parser->original_string();
        #add the additional namespace definitions, which we have discovered during the first
        #parsing
        #cut of the closing > or /> from the start element, so we can append the namespace definitions
        $dep =~ /(\s*<.*) ((\s*\/>)|(\s*>))/x;
        my $dep1 = $1;
        $dep1.= " xmlns:".$_.'="'.$notDefInDep{$_}.'"' for (keys %notDefInDep);
        $dep1.= $2;
        print $FH " "x$parser->current_column(), $dep1;
    }
    elsif ($inDependencies)
    {
        #$prefix is global because we need to use it in the end element as well.
        $prefix = "";
        my $fullString;
        my $orig = $parser->original_string();
        #Split up the string so we can insert the prefix for the element.
        # <OpenOffice.org-minimal-version>
        # <d:OpenOffice.org-minimal-version>
        $orig=~/(\s*<)(.*?)\s/x;
        #in $2 is the element name, look for the prefix
        if ($2 !~/(.*?):/ && $parser->namespace($name)) {
            #no prefix, that is element uses default namespace.
            #Now check if the default namespace in <dependencies> is the same as the one in this
            #element. If not, then the default ns was defined "after" <dependencies>. Because all
            #children of <dependencies> are copied into the update information, so will this default
            #namespace definition. Hence this element will have the same default namespace in the
            #update information.
            my $defNsDep = $validPrefsInDep{"#default"};
            #we must have #default, see the if statement above
            my $defNsCur = $parser->expand_ns_prefix("#default");

            if ($defNsDep eq $defNsCur) {
                #Determine if there is in <dependency> a prefix defined (only valid there and need not
                #directly defined in this element). If there is no prefix defined then we will
                #add a new definition to <dependencies>.
                for (keys %validPrefsInDep) {
                    if (($validPrefsInDep{$_} eq $defNsDep) && $_ ne "#default") {
                        $prefix = $_; last;
                    }
                }
                if (! $prefix) {
                    #If there was no prefix, we will add new prefix definition to <dependency>
                    #Which prefix this is has been determined during the first parsing.
                    for (keys %notDefInDep) {
                        if (($notDefInDep{$_} eq $defNsCur) && $_ ne "#default") {
                            $prefix = $_; last;
                        }
                    }
                }
                #die if we have no prefix
                confess "No prefix defined for default namespace " unless $prefix;
                #get the full part after <
                $orig=~/(\s*<)(.*)/x;
                $fullString= $1.$prefix.":".$2;
            }

        }
        $fullString = $orig unless $fullString;

        # We record anything within <dependencies> </dependencies>.
        print $FH $fullString;
    }
}

sub end_handler
{
    my $parser = shift;
    my $name = shift;

    if ($name eq "description"
        && $descNS eq  $parser->namespace($name))
    {
        $inDescription = 0;
    }
    elsif ($inDescription
           && $name eq "version"
           && $descNS eq  $parser->namespace($name))
    {
        $inVersion = 0;
        print $FH  $parser->original_string(), "\n";
    }
    elsif ($inDescription
           && $name eq "identifier"
           && $descNS eq  $parser->namespace($name))
    {
        $inIdentifier = 0;
        print $FH $parser->original_string(), "\n";
    }
    elsif($inDescription
          && $name eq "dependencies"
          && $descNS eq $parser->namespace($name))
    {
        $inDependencies = 0;
        print $FH $parser->original_string(), "\n";
    }
    elsif ($inDependencies)
    {
        my $orig = $parser->original_string();
        #$orig is empty if we have tags like this: <name />
        if ($orig && $prefix) {
            $orig=~/(\s*<\/)(.*)/x;
            $orig= $1.$prefix.":".$2;
        }
        print $FH $orig;
    }
}

#We write the complete content between start and end tags of
# <identifier>, <version>, <dependencies>
sub default_handler
{
    my $parser = shift;
    my $name = shift;
    if ($inIdentifier || $inVersion) {
        print $FH $parser->original_string();
    } elsif ($inDependencies) {
        print $FH  $parser->original_string();
    }

}  # End of default_handler

#sax handler used for the first parsing to recognize the used prefixes in <dependencies > and its
#children and to find out if we need to define a new prefix for the current default namespace.
sub start_handler_infos
{
    my $parser = shift;
    my $name = shift;
    if ($name eq "description"
        && $descNS eq $parser->namespace($name)) {
        $inDescription = 1;
    }
    elsif ($inDescription
           && $name eq "dependencies"
           && $descNS eq  $parser->namespace($name)) {
        $inDependencies = 1;
        #build the map of prefix/namespace which are valid in <dependencies>
        my @cur = $parser->current_ns_prefixes();
        for (@cur) {
            $validPrefsInDep{$_} = $parser->expand_ns_prefix($_);
        }
        #remember the prefixes defined in <dependencies>
        @newPrefsInDep = $parser->new_ns_prefixes();

        collectPrefixes($parser, $name, \@_, \@usedNsInDependencies);
        return if  $generatedPrefix;

        #determine if need to create a new prefix for the current element if it uses a default ns.
        #Split up the string so we can see if there is a prefix used
        # <OpenOffice.org-minimal-version>
        # <d:OpenOffice.org-minimal-version>
        my $orig = $parser->original_string();
        $orig=~/(\s*<)(.*?)\s/x;
        #in $2 is the element name, look for the prefix
        if ($2 !~/(.*?):/ && $parser->namespace($name)) {
            #no prefix, that is element uses default namespace.
            #Now check if the default namespace in <dependencies> is the same as the one in this
            #element. If not, then the default ns was defined "after" <dependencies>. Because all
            #children of <dependencies> are copied into the update information, so will this default
            #namespace definition. Hence this element will have the same default namespace in the
            #update information.
            my $defNsDep = $validPrefsInDep{"#default"};
            #we must have #default, see the if statement above
            my $defNsCur = $parser->expand_ns_prefix("#default");

            if ($defNsDep eq $defNsCur) {
                #Determine if there is in <dependency> a prefix defined (only valid there and need not
                #directly defined in this element). If there is no prefix defined then we will
                #add a new definition to <dependencies>.
                for (keys %validPrefsInDep) {
                    if (($validPrefsInDep{$_} eq $defNsDep) && $_ ne "#default") {
                        $prefix = $_; last;
                    }
                }

                if (! $prefix) {

                    #define a new prefix
                    #actually there can be only onle prefix, which is the case when the element
                    #uses the same default namespace as <dependencies> otherwise, the default
                    #namespace was redefined by the children of <dependencies>. These are completely
                    #copied and still valid in the update information file
                    $generatedPrefix = "a";
                    $defNsInDep = $defNsDep;
                }
            }
        }

    }
    elsif ($inDependencies) {
        determineNsDefinitions($parser, $name, \@_);
        collectPrefixes($parser, $name, \@_, \@usedNsInDependencies);
    }
}
#sax handler used for the first parsing to recognize the used prefixes in <dependencies > and its
#children
sub end_handler_infos
{
    my $parser = shift;
    my $name = shift;

    if ($name eq "description"
        && $descNS eq  $parser->namespace($name)) {
        $inDescription = 0;
    }
    elsif($inDescription
          && $name eq "dependencies"
          && $descNS eq $parser->namespace($name)) {
        $inDependencies = 0;
    }
}

sub writeUpdateInformationData($)
{
    my $desc = shift;
    {
        #parse description xml to collect information about all used
        #prefixes and names within <dependencies>

        my $parser = new XML::Parser(ErrorContext => 2,
                                     Namespaces => 1);
        $parser->setHandlers(Start => \&start_handler_infos,
                             End => \&end_handler_infos);

        $parser->parsefile($desc);


    }
    #remove duplicates in the array containing the prefixes
    if ($generatedPrefix) {
        my %hashtmp;
        @usedNsInDependencies = grep(!$hashtmp{$_}++, @usedNsInDependencies);

        #check that the prefix for the default namespace in <dependencies> does not clash
        #with any other prefixes
        my $clash;
        do {
            $clash = 0;
            for (@usedNsInDependencies) {
                if ($_ eq $generatedPrefix) {
                    $generatedPrefix++;
                    $clash = 1; last;
                }
            }
        } while ($clash);
        $notDefInDep{$generatedPrefix} = $defNsInDep;
    }
    #if $notDefInDep contains the prefix #default then we need to add the generated prefix as well

    #add the special prefix for the default namespace into the map of prefixes that will be
    #added to the <dependencies> element in the update information file


    ($inDependencies, $inDescription) = (0,0);
    {
        my $parser = new XML::Parser(ErrorContext => 2,
                                     Namespaces => 1);
        $parser->setHandlers(
                             Start => \&start_handler,
                             End => \&end_handler,
                             Default => \&default_handler);
        $parser->parsefile($desc);
    }
}

# param 1: name of the attribute we look for
# param 2: array of name value pairs, the first subscript is the attribute and the second
# is the value.
sub findAttribute($$)
{
    my ($name, $args_r) = @_;
    my @args = @{$args_r};
    my $value;
    while (my $attr = shift(@args))
    {
        if ($attr eq $name) {
            $value = shift(@args);
            die "href attribut has no valid URL" unless $value;
            last;
        } else { # shift away the following value for the attribute
            shift(@args);
        }
    }
    return $value;
}

#collect the prefixes used in an xml element
#param 1: parser,
#param 2: element name,
#param 3: array of name and values of attributes
#param 4: out parameter, the array containing the prefixes
sub collectPrefixes($$$$)
{
    my $parser = shift;
    my $name = shift;
    my $attr_r = shift;
    my $out_r = shift;
    #get the prefixes which are currently valid
    my @cur = $parser->current_ns_prefixes();
    my %map_ns;
    #get the namespaces for the prefixes
    for (@cur) {
        if ($_ eq '#default') {
            next;
        }
        my $ns = $parser->expand_ns_prefix($_);
        $map_ns{$ns} = $_;
    }
    #investigat ns of element
    my $pref = $map_ns{$parser->namespace($name)};
    push(@{$out_r}, $pref) if $pref;
    #now go over the attributes

    while (my $attr = shift(@{$attr_r})) {
        my $ns = $parser->namespace($attr);
        if (! $ns) {
            shift(@{$attr_r});
            next;
        }
        $pref = $map_ns{$ns};
        push( @{$out_r}, $pref) if $pref;
        shift(@{$attr_r});
    }
    #also add newly defined prefixes
    my @newNs = $parser->new_ns_prefixes();
    for (@newNs) {
        if ($_ eq '#default') {
            next;
        }
        push (@{$out_r}, $_);
    }
}

#The function is called for each child element of dependencies. It finds out the prefixes
#which are used by the children and which are defined by the parents of <dependencies>. These
#would be lost when copying the children of <dependencies> into the update information file.
#Therefore these definitions are collected so that they then can be written in the <dependencies>
#element of the update information file.
#param 1: parser
#param 2: namsepace
#param 3: the @_ received in the start handler
sub determineNsDefinitions($$$)
{
    my ($parser, $name, $attr_r) = @_;
    my @attr = @{$attr_r};

    determineNsDefinitionForItem($parser, $name, 1);

    while (my $attr = shift(@attr)) {
        determineNsDefinitionForItem($parser, $attr, 0);
        shift @attr;
    }
}

#do not call this function for the element that does not use a prefix
#param 1: parser
#param 2: name of the element or attribute
#param 3: 1 if called for an elment name and 0 when called for attribue
sub determineNsDefinitionForItem($$$)
{
    my ($parser, $name) = @_;
    my $ns = $parser->namespace($name);
    if (! $ns) {
        return;
    }
    #If the namespace was not kwown in <dependencies> then it was defined in one of its children
    #or in this element. Then we are done since this namespace definition is copied into the
    #update information.
    my $bNsKnownInDep;
    for ( keys %validPrefsInDep) {
        if ( $validPrefsInDep{$_} eq $ns) {
            $bNsKnownInDep = 1;
            last;
        }
    }
    #If the namespace of the current element is known in <dependencies> then check if the same
    #prefix is used. If not, then the prefix was defined in one of the children of <dependencies>
    #and was assigned the same namespace. Because we copy of children into the update information,
    #this definition is also copied.
    if ($bNsKnownInDep) {
        #create a map of currently valid prefix/namespace
        my %curPrefToNs;
        my @curNs = $parser->current_ns_prefixes();
        for (@curNs) {
            $curPrefToNs{$_} = $parser->expand_ns_prefix($_);
        }
        #find the prefix used in <dependencies> to define the namespace of the current element
        my $validDepPref;
        for (keys %validPrefsInDep) {
            if ($validPrefsInDep{$_} eq $ns) {
                #ignore #default
                next if $_ eq "#default";
                $validDepPref = $_;
                last;
            }
        }
        #find the prefix defined in the current element used for the namespace of the element
        my $curPref;
        for (keys %curPrefToNs) {
            if ($curPrefToNs{$_} eq $ns) {
                #ignore #default
                next if $_ eq "#default";
                $curPref = $_;
                last;
            }
        }
        if ($curPref && $validDepPref && ($curPref eq $validDepPref)) {
            #If the prefixes and ns are the same, then the prefix definition of <dependencies> or its
            #parent can be used. However, we need to find out which prefixed are NOT defined in
            #<dependencies> so we can add them to it when we write the update information.
            my $bDefined = 0;
            for (@newPrefsInDep) {
                if ($curPref eq $_) {
                    $bDefined = 1;
                    last;
                }
            }
            if (! $bDefined) {
                $notDefInDep{$curPref} = $ns;
            }
        }
    }
}
