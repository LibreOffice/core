:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
#
# 2009 Copyright Novell, Inc. & Sun Microsystems, Inc.
#

use IO::File;
use Cwd;
use File::Spec;
use File::Spec::Functions;
use File::Temp;
use File::Path;

$TempDir = "";


# all the XML package generation is a blatant rip from AF's
# write-calc-doc.pl


#   Open a file with the given name.
#   First it is checked if the temporary directory, in which all files for
#   the document are gathered, is already present and create it if it is not.
#   Then create the path to the file inside the temporary directory.
#   Finally open the file and return a file handle to it.
#
sub open_file
{
    my  $filename = pop @_;

    #   Create base directory of temporary directory tree if not alreay
    #   present.
    if ($TempDir eq "")
    {
        $TempDir = File::Temp::tempdir (CLEANUP => 1);
    }

    #   Create the path to the file.
    my $fullname = File::Spec->catfile ($TempDir, $filename);
    my ($volume,$directories,$file) = File::Spec->splitpath ($fullname);
    mkpath (File::Spec->catpath ($volume,$directories,""));

    #   Open the file and return a file handle to it.
    return new IO::File ($fullname, "w");
}


#   Zip the files in the directory tree into the given file.
#
sub zip_dirtree
{
    my  $filename = pop @_;

    my  $cwd = getcwd;
    my  $zip_name = $filename;

    #   We are about to change the directory.
    #   Therefore create an absolute pathname for the zip archive.

    #   First transfer the drive from $cwd to $zip_name.  This is a
    #   workaround for a bug in file_name_is_absolute which thinks
    #   the path \bla is an absolute path under DOS.
    my ($volume,$directories,$file) = File::Spec->splitpath ($zip_name);
    my ($volume_cwd,$directories_cwd,$file_cwd) = File::Spec->splitpath ($cwd);
    $volume = $volume_cwd if ($volume eq "");
    $zip_name = File::Spec->catpath ($volume,$directories,$file);

    #   Add the current working directory to a relative path.
    if ( ! file_name_is_absolute ($zip_name))
    {
        $zip_name = File::Spec->catfile ($cwd, $zip_name);

        #   Try everything to clean up the name.
        $zip_name = File::Spec->rel2abs ($filename);
        $zip_name = File::Spec->canonpath ($zip_name);

        #   Remove .. directories from the middle of the path.
        while ($zip_name =~ /\/[^\/][^\.\/][^\/]*\/\.\.\//)
        {
            $zip_name = $` . "/" . $';
        }
    }

    #   Just in case the zip program gets confused by an existing file with the
    #   same name as the one to be written that file is removed first.
    if ( -e $filename)
    {
        if (unlink ($filename) == 0)
        {
            print "Existing file $filename could not be deleted.\n";
            print "Please close the application that uses it, then try again.\n";
            return;
        }
    }

    #   Finally create the zip file.  First change into the temporary directory
    #   so that the resulting zip file contains only paths relative to it.
    print "zipping [$ZipCmd $ZipFlags $zip_name *]\n";
    chdir ($TempDir);
    system ("$ZipCmd $ZipFlags $zip_name *");
    chdir ($cwd);
}


sub writeHeader
{
    print $OUT qq~<?xml version="1.0" encoding="UTF-8"?>

<office:document-content xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0" xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0" xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0" xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="urn:oasis:names:tc:opendocument:xmlns:meta:1.0" xmlns:number="urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0" xmlns:presentation="urn:oasis:names:tc:opendocument:xmlns:presentation:1.0" xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" xmlns:chart="urn:oasis:names:tc:opendocument:xmlns:chart:1.0" xmlns:dr3d="urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0" xmlns:math="http://www.w3.org/1998/Math/MathML" xmlns:form="urn:oasis:names:tc:opendocument:xmlns:form:1.0" xmlns:script="urn:oasis:names:tc:opendocument:xmlns:script:1.0" xmlns:ooo="http://openoffice.org/2004/office" xmlns:ooow="http://openoffice.org/2004/writer" xmlns:oooc="http://openoffice.org/2004/calc" xmlns:dom="http://www.w3.org/2001/xml-events" xmlns:xforms="http://www.w3.org/2002/xforms" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:smil="urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0" xmlns:anim="urn:oasis:names:tc:opendocument:xmlns:animation:1.0" office:version="1.0">
 <office:scripts/>
 <office:automatic-styles>
  <style:style style:name="dp1" style:family="drawing-page">
   <style:drawing-page-properties presentation:background-visible="true" presentation:background-objects-visible="true" presentation:display-footer="true" presentation:display-page-number="false" presentation:display-date-time="true"/>
  </style:style>
  <style:style style:name="gr1" style:family="graphic" style:parent-style-name="standard">
   <style:graphic-properties draw:textarea-horizontal-align="center" draw:fill="none" draw:stroke="none" draw:textarea-vertical-align="middle"/>
  </style:style>
  <style:style style:name="gr2" style:family="graphic" style:parent-style-name="standard">
   <style:graphic-properties draw:textarea-horizontal-align="center" draw:textarea-vertical-align="middle"/>
  </style:style>
  <style:style style:name="pr1" style:family="presentation" style:parent-style-name="Default-title">
   <style:graphic-properties draw:fill-color="#ffffff" draw:auto-grow-height="true" fo:min-height="3.508cm"/>
  </style:style>
  <style:style style:name="pr2" style:family="presentation" style:parent-style-name="Default-notes">
   <style:graphic-properties draw:fill-color="#ffffff" draw:auto-grow-height="true" fo:min-height="13.367cm"/>
  </style:style>
  <style:style style:name="P1" style:family="paragraph">
   <style:paragraph-properties fo:margin-left="0cm" fo:margin-right="0cm" fo:text-indent="0cm"/>
  </style:style>
  <style:style style:name="P2" style:family="paragraph">
   <style:paragraph-properties fo:margin-left="0.6cm" fo:margin-right="0cm" fo:text-indent="-0.6cm"/>
  </style:style>
  <text:list-style style:name="L1">
   <text:list-level-style-bullet text:level="1" text:bullet-char="●">
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="2" text:bullet-char="●">
    <style:list-level-properties text:space-before="0.6cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="3" text:bullet-char="●">
    <style:list-level-properties text:space-before="1.2cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="4" text:bullet-char="●">
    <style:list-level-properties text:space-before="1.8cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="5" text:bullet-char="●">
    <style:list-level-properties text:space-before="2.4cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="6" text:bullet-char="●">
    <style:list-level-properties text:space-before="3cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="7" text:bullet-char="●">
    <style:list-level-properties text:space-before="3.6cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="8" text:bullet-char="●">
    <style:list-level-properties text:space-before="4.2cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
   <text:list-level-style-bullet text:level="9" text:bullet-char="●">
    <style:list-level-properties text:space-before="4.8cm" text:min-label-width="0.6cm"/>
    <style:text-properties fo:font-family="StarSymbol" style:use-window-font-color="true" fo:font-size="45%"/>
   </text:list-level-style-bullet>
  </text:list-style>
 </office:automatic-styles>
 <office:body>
  <office:presentation>
~;

}

sub writeSlideHeader
{
    my $titleText = pop @_;
    my $slideNum = pop @_;

    print $OUT "    <draw:page draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">\n";
    print $OUT "     <office:forms form:automatic-focus=\"false\" form:apply-design-mode=\"false\"/>\n";
    print $OUT "     <draw:rect draw:style-name=\"gr1\" draw:text-style-name=\"P1\" draw:id=\"id$slideNum\" draw:layer=\"layout\" svg:width=\"17.5cm\" svg:height=\"6cm\" svg:x=\"5cm\" svg:y=\"4cm\">\n";
    print $OUT "      <text:p text:style-name=\"P2\">Slide: $slideNum</text:p>\n";
    print $OUT "      <text:p text:style-name=\"P2\">Path: $titleText</text:p>\n";
    print $OUT "     </draw:rect>\n";
}


sub writeSlideFooter
{
    print $OUT "    <presentation:notes draw:style-name=\"dp1\">\n";
    print $OUT "     <draw:page-thumbnail draw:style-name=\"gr1\" draw:layer=\"layout\" svg:width=\"14.851cm\" svg:height=\"11.138cm\" svg:x=\"3.068cm\" svg:y=\"2.257cm\" draw:page-number=\"1\" presentation:class=\"page\"/>\n";
    print $OUT "     <draw:frame presentation:style-name=\"pr3\" draw:layer=\"layout\" svg:width=\"16.79cm\" svg:height=\"13.116cm\" svg:x=\"2.098cm\" svg:y=\"14.109cm\" presentation:class=\"notes\" presentation:placeholder=\"true\">\n";
    print $OUT "      <draw:text-box/>\n";
    print $OUT "     </draw:frame>\n";
    print $OUT "    </presentation:notes>\n";
    print $OUT "   </draw:page>\n";
}

sub writeFooter
{
    print $OUT qq~   <presentation:settings presentation:full-screen="false"/>
  </office:presentation>
 </office:body>
</office:document-content>
~;

}

sub writePath
{
    my $pathAry = pop @_;
    my $path = $pathAry->[1];
    my $viewBox = $pathAry->[0];

    print $OUT "       <draw:path draw:style-name=\"gr2\" draw:text-style-name=\"P1\" draw:layer=\"layout\" svg:width=\"10cm\" svg:height=\"10cm\" svg:x=\"5cm\" svg:y=\"5cm\" svg:viewBox=\"";
    print $OUT $viewBox;
    print $OUT "\" svg:d=\"";
    print $OUT $path;
    print $OUT "\">\n";
    print $OUT "         <text:p/>\n";
    print $OUT "       </draw:path>\n";
}

sub writeManifest
{
    my $outFile = open_file("META-INF/manifest.xml");

    print $outFile qq~<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE manifest:manifest PUBLIC "-//OpenOffice.org//DTD Manifest 1.0//EN" "Manifest.dtd">
<manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0">
 <manifest:file-entry manifest:media-type="application/vnd.oasis.opendocument.presentation" manifest:full-path="/"/>
 <manifest:file-entry manifest:media-type="text/xml" manifest:full-path="content.xml"/>
</manifest:manifest>
~;

    $outFile->close;
}


#   Print usage information.
#
sub usage   ()
{
    print <<END_OF_USAGE;
usage: $0 <option>* [<SvgD-values>]

output-file-name defaults to polygons.odp.

         -h    Print this usage information.
         -o    output-file-name
END_OF_USAGE
}

#   Process the command line.
#
sub process_command_line
{
    foreach (@ARGV)
    {
        if (/^-h/)
        {
            usage;
            exit 0;
        }
    }

    $global_output_name = "polygons.odp";
    my  $j = 0, $noMoreOptions = 0;
    for (my $i=0; $i<$#ARGV; $i++)
    {
        if ( !$noMoreOptions and $ARGV[$i] eq "-o")
        {
            $i++;
            $global_output_name = $ARGV[$i];
        }
        elsif ( !$noMoreOptions and $ARGV[$i] eq "--")
        {
            $noMoreOptions = 1;
        }
        elsif ( !$noMoreOptions and $ARGV[$i] =~ /^-/)
        {
            print "Unknown option $ARGV[$i]\n";
            usage;
            exit 1;
        }
        else
        {
            push(@paths, [$ARGV[$i],$ARGV[$i+1]]);
            $i++;
        }
    }

    print "output to $global_output_name\n";
}

#   Main

$ZipCmd = $ENV{LOG_FILE_ZIP_CMD};
$ZipFlags = $ENV{LOG_FILE_ZIP_FLAGS};
#   Provide default values for the zip command and it's flags.
if ( ! defined $ZipCmd)
{
    $ZipCmd = "zip" unless defined $ZipCmd;
    $ZipFlags = "-r -q" unless defined $ZipFlags;
}

process_command_line();

writeManifest();

$OUT = open_file( "content.xml" );

writeHeader();

$pathNum=0;
foreach $path (@paths)
{
    writeSlideHeader($pathNum, $path->[1]);
    writePath($path);
    writeSlideFooter();
    $pathNum++;
}

writeFooter();

$OUT->close;

zip_dirtree ($global_output_name);

