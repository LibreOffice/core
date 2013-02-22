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

use IO::File;
use Cwd;
use File::Spec;
use File::Spec::Functions;
use File::Temp;
use File::Path;

$TempDir = "";


# all the XML package generation is a blatant rip from AF's
# write-calc-doc.pl


###############################################################################
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


###############################################################################
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

    print $OUT "      <draw:page draw:name=\"page1\" draw:style-name=\"dp1\" draw:master-page-name=\"Default\">\n";
    print $OUT "       <office:forms form:automatic-focus=\"false\" form:apply-design-mode=\"false\"/>\n";
    print $OUT "       <draw:rect draw:style-name=\"gr1\" draw:text-style-name=\"P1\" draw:id=\"id$slideNum\" draw:layer=\"layout\" svg:width=\"17.5cm\" svg:height=\"13cm\" svg:x=\"5cm\" svg:y=\"4cm\">\n";
    print $OUT "        <text:p text:style-name=\"P2\">Slide: $slideNum</text:p>\n";
    print $OUT "        <text:p text:style-name=\"P2\">Topic: $titleText</text:p>\n";
    print $OUT "        <text:p text:id=\"textid$slideNum\" text:style-name=\"P2\">Some text to show text effects</text:p>\n";
    print $OUT "       </draw:rect>\n";
    print $OUT "      <anim:par presentation:node-type=\"timing-root\">\n";
    print $OUT "        <anim:seq presentation:node-type=\"main-sequence\">\n";
}


sub writeSlideFooter
{
    print $OUT "       </anim:seq>\n";
    print $OUT "      </anim:par>\n";
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

sub writeTransitionAnimation
{
    my $transitionSubtype = pop @_;
    my $transitionType = pop @_;
    my $slideNum = pop @_;

    print $OUT "          <anim:par smil:begin=\"0s\" smil:fill=\"remove\">\n";
    print $OUT "            <anim:set smil:begin=\"0s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:transitionFilter smil:dur=\"1s\" smil:targetElement=\"textid$slideNum\" smil:type=\"$transitionType\" smil:subtype=\"$transitionSubtype\"/>\n";
    print $OUT "            <anim:set smil:begin=\"0.3s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:transitionFilter smil:begin=\"0.3s\" smil:dur=\"1s\" smil:targetElement=\"id$slideNum\" smil:type=\"$transitionType\" smil:subtype=\"$transitionSubtype\"/>\n";
    print $OUT "          </anim:par>\n";
}

sub writePropertyAnimation
{
    my $propertyEnd = pop @_;
    my $propertyStart = pop @_;
    my $propertyName = pop @_;
    my $slideNum = pop @_;

    print $OUT "          <anim:par smil:begin=\"0s\" smil:dur=\"3s\" smil:fill=\"remove\">\n";
    print $OUT "            <anim:set smil:begin=\"0s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animate smil:begin=\"0s\" smil:dur=\"1s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" smil:attributeName=\"$propertyName\" smil:values=\"$propertyStart;$propertyEnd\" smil:keyTimes=\"0;1\" presentation:additive=\"base\"/>\n";
    print $OUT "            <anim:set smil:begin=\"0.6s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animate smil:begin=\"0.6s\" smil:dur=\"1s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" smil:attributeName=\"$propertyName\" smil:values=\"$propertyStart;$propertyEnd\" smil:keyTimes=\"0;1\" presentation:additive=\"base\"/>\n";
    print $OUT "          </anim:par>\n";
}

sub writeTransformAnimation
{
    my $propertyBy = pop @_;
    my $propertyName = pop @_;
    my $slideNum = pop @_;

    print $OUT "          <anim:par smil:begin=\"0s\" smil:dur=\"3s\" smil:fill=\"remove\">\n";
    print $OUT "            <anim:set smil:begin=\"0s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animateTransform smil:begin=\"0s\" smil:dur=\"1s\" smil:targetElement=\"id$slideNum\" smil:fill=\"hold\" smil:by=\"$propertyBy\" presentation:additive=\"base\" svg:type=\"$propertyName\"/>\n";
    print $OUT "            <anim:set smil:begin=\"0.6s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animateTransform smil:begin=\"0.6s\" smil:dur=\"1s\" smil:targetElement=\"textid$slideNum\" smil:fill=\"hold\" smil:by=\"$propertyBy\" presentation:additive=\"base\" svg:type=\"$propertyName\"/>\n";
    print $OUT "          </anim:par>\n";
}

sub writePathMotionAnimation
{
    my $slideNum = pop @_;

    print $OUT "          <anim:par smil:begin=\"0s\" smil:dur=\"10s\" smil:fill=\"remove\">\n";
    print $OUT "            <anim:set smil:begin=\"0s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animateMotion smil:dur=\"5s\" smil:fill=\"hold\" smil:targetElement=\"id$slideNum\" presentation:additive=\"base\" svg:path=\"m0.0 0.07658c0.0098-0.00493 0.00197-0.00985 0.00295-0.01478 0.00191 0.00 0.00383 0.00 0.00574 0.00-0.00005 0.00033-0.00011 0.00065-0.00016 0.00098-0.00034 0.00276-0.00060 0.00446-0.00077 0.00512-0.00021 0.00086-0.00031 0.00143-0.00031 0.00170 0.00 0.00200 0.00150 0.00369 0.00452 0.00507 0.00301 0.00138 0.00671 0.00206 0.01108 0.00206 0.00438 0.00 0.00816-0.00164 0.01134-0.00493 0.00319-0.00329 0.00478-0.00719 0.00478-0.01170 0.00-0.00514-0.00311-0.01022-0.00935-0.01525-0.00162-0.00129-0.00324-0.00258-0.00486-0.00387-0.00806-0.00651-0.01209-0.01290-0.01209-0.01917s0.0251-0.01148 0.00752-0.01561 0.01131-0.00620 0.01889-0.00620c0.0585 0.00 0.01276 0.00126 0.02072 0.00377-0.00102 0.00512-0.00203 0.01023-0.00305 0.01535-0.00191 0.00-0.00383 0.00-0.00574 0.00 0.00009-0.00052 0.00017-0.00103 0.00026-0.00155 0.00019-0.00195 0.00038-0.00389 0.00057-0.00584 0.00009-0.00062 0.00017-0.00124 0.00026-0.00186-0.00014-0.00183-0.00155-0.00337-0.00424-0.00462-0.00269-0.00126-0.00589-0.00189-0.00961-0.00189-0.00424 0.00-0.00782 0.00144-0.01075 0.00431-0.00293 0.00288-0.00439 0.00640-0.00439 0.01057 0.00 0.00510 0.00334 0.01035 0.01002 0.01576 0.00172 0.00138 0.00345 0.00275 0.00517 0.00413 0.00782 0.00631 0.01173 0.01277 0.01173 0.01938 0.00 0.00675-0.00272 0.01224-0.00816 0.01646-0.00545 0.00422-0.01256 0.00633-0.02134 0.00633-0.00538 0.00-0.01165-0.00105-0.01881-0.00315-0.00064-0.00019-0.00128-0.00038-0.00192-0.00057z\"/>\n";
    print $OUT "            <anim:set smil:begin=\"3.6s\" smil:dur=\"0.001s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" smil:attributeName=\"visibility\" smil:to=\"visible\"/>\n";
    print $OUT "            <anim:animateMotion smil:begin=\"3.6s\" smil:dur=\"5s\" smil:fill=\"hold\" smil:targetElement=\"textid$slideNum\" presentation:additive=\"base\" svg:path=\"m0.0 0.07658c0.0098-0.00493 0.00197-0.00985 0.00295-0.01478 0.00191 0.00 0.00383 0.00 0.00574 0.00-0.00005 0.00033-0.00011 0.00065-0.00016 0.00098-0.00034 0.00276-0.00060 0.00446-0.00077 0.00512-0.00021 0.00086-0.00031 0.00143-0.00031 0.00170 0.00 0.00200 0.00150 0.00369 0.00452 0.00507 0.00301 0.00138 0.00671 0.00206 0.01108 0.00206 0.00438 0.00 0.00816-0.00164 0.01134-0.00493 0.00319-0.00329 0.00478-0.00719 0.00478-0.01170 0.00-0.00514-0.00311-0.01022-0.00935-0.01525-0.00162-0.00129-0.00324-0.00258-0.00486-0.00387-0.00806-0.00651-0.01209-0.01290-0.01209-0.01917s0.0251-0.01148 0.00752-0.01561 0.01131-0.00620 0.01889-0.00620c0.0585 0.00 0.01276 0.00126 0.02072 0.00377-0.00102 0.00512-0.00203 0.01023-0.00305 0.01535-0.00191 0.00-0.00383 0.00-0.00574 0.00 0.00009-0.00052 0.00017-0.00103 0.00026-0.00155 0.00019-0.00195 0.00038-0.00389 0.00057-0.00584 0.00009-0.00062 0.00017-0.00124 0.00026-0.00186-0.00014-0.00183-0.00155-0.00337-0.00424-0.00462-0.00269-0.00126-0.00589-0.00189-0.00961-0.00189-0.00424 0.00-0.00782 0.00144-0.01075 0.00431-0.00293 0.00288-0.00439 0.00640-0.00439 0.01057 0.00 0.00510 0.00334 0.01035 0.01002 0.01576 0.00172 0.00138 0.00345 0.00275 0.00517 0.00413 0.00782 0.00631 0.01173 0.01277 0.01173 0.01938 0.00 0.00675-0.00272 0.01224-0.00816 0.01646-0.00545 0.00422-0.01256 0.00633-0.02134 0.00633-0.00538 0.00-0.01165-0.00105-0.01881-0.00315-0.00064-0.00019-0.00128-0.00038-0.00192-0.00057z\"/>\n";
    print $OUT "          </anim:par>\n";
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


###############################################################################
#   Print usage information.
#
sub usage   ()
{
    print <<END_OF_USAGE;
usage: $0 <option>* [<output-file-name>]

output-file-name defaults to alltransitions.odp.

options: -a    Generate _all_ combinations of type, subtype,
               direction, and mode
         -h    Print this usage information.
END_OF_USAGE
}

###############################################################################
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

    $global_gen_all=0;
    $global_output_name = "alltransitions.odp";

    my  $j = 0;
    for (my $i=0; $i<=$#ARGV; $i++)
    {
        if ($ARGV[$i] eq "-a")
        {
            $global_gen_all=1;
        }
        elsif ($ARGV[$i] =~ /^-/)
        {
            print "Unknown option $ARGV[$i]\n";
            usage;
            exit 1;
        }
        elsif ($#ARGV == $i )
        {
            $global_output_name = $ARGV[$i];
        }
    }

    print "output to $global_output_name\n";
}

$transitionsRef = [

                ["barWipe",
                 ["leftToRight",
                  "topToBottom"]],

                ["blindsWipe",
                 ["vertical",
                  "horizontal"]],

                ["boxWipe",
                 ["topLeft",
                  "topRight",
                  "bottomRight",
                  "bottomLeft",
                  "topCenter",
                  "rightCenter",
                  "bottomCenter",
                  "leftCenter"]],

                ["fourBoxWipe",
                 ["cornersIn",
                  "cornersOut"]],

                ["barnDoorWipe",
                 ["vertical",
                  "horizontal",
                  "diagonalBottomLeft",
                  "diagonalTopLeft"]],

                ["bowTieWipe",
                 ["vertical",
                  "horizontal"]],

                ["miscDiagonalWipe",
                 ["doubleBarnDoor",
                  "doubleDiamond"]],

                ["veeWipe",
                 ["down",
                  "left",
                  "up",
                  "right"]],

                ["barnVeeWipe",
                 ["top",
                  "left",
                  "up",
                  "right"]],

                ["zigZagWipe",
                 ["leftToRight",
                  "topToBottom"]],

                ["barnZigZagWipe",
                 ["vertical",
                  "horizontal"]],

                ["irisWipe",
                 ["rectangle",
                  "diamond"]],

                ["triangleWipe",
                 ["up",
                  "right",
                  "down",
                  "left"]],

                ["arrowHeadWipe",
                 ["up",
                  "right",
                  "down",
                  "left"]],

                ["pentagonWipe",
                 ["up",
                  "down"]],

                ["hexagonWipe",
                 ["horizontal",
                  "vertical"]],

                ["ellipseWipe",
                 ["circle",
                  "horizontal",
                  "vertical"]],

                ["eyeWipe",
                 ["vertical",
                  "horizontal"]],

                ["roundRectWipe",
                 ["horizontal",
                  "vertical"]],

                ["starWipe",
                 ["fourPoint",
                  "fivePoint",
                  "sixPoint"]],

                ["miscShapeWipe",
                 ["heart",
                  "keyhole"]],

                ["clockWipe",
                 ["clockwiseTwelve",
                  "clockwiseThree",
                  "clockwiseSix",
                  "clockwiseNine"]],

                ["pinWheelWipe",
                 ["oneBlade",
                  "twoBladeVertical",
                  "twoBladeHorizontal",
                  "threeBlade",
                  "fourBlade",
                  "eightBlade"]],

                ["singleSweepWipe",
                 ["clockwiseTop",
                  "clockwiseRight",
                  "clockwiseBottom",
                  "clockwiseLeft",
                  "clockwiseTopLeft",
                  "counterClockwiseBottomLeft",
                  "clockwiseBottomRight",
                  "counterClockwiseTopRight"]],

                ["fanWipe",
                 ["centerTop",
                  "centerRight",
                  "top",
                  "right",
                  "bottom",
                  "left"]],

                ["doubleFanWipe",
                 ["fanOutVertical",
                  "fanOutHorizontal",
                  "fanInVertical",
                  "fanInHorizontal"]],

                ["doubleSweepWipe",
                 ["parallelVertical",
                  "parallelDiagonal",
                  "oppositeVertical",
                  "oppositeHorizontal",
                  "parallelDiagonalTopLeft",
                  "parallelDiagonalBottomLeft"]],

                ["saloonDoorWipe",
                 ["top",
                  "left",
                  "bottom",
                  "right"]],

                ["windshieldWipe",
                 ["right",
                  "up",
                  "vertical",
                  "horizontal"]],

                ["snakeWipe",
                 ["topLeftHorizontal",
                  "topLeftVertical",
                  "topLeftDiagonal",
                  "topRightDiagonal",
                  "bottomRightDiagonal",
                  "bottomLeftDiagonal"]],

                ["spiralWipe",
                 ["topLeftClockwise",
                  "topRightClockwise",
                  "bottomRightClockwise",
                  "bottomLeftClockwise",
                  "topLeftCounterClockwise",
                  "topRightCounterClockwise",
                  "bottomRightCounterClockwise",
                  "bottomLeftCounterClockwise"]],

                ["parallelSnakesWipe",
                 ["verticalTopSame",
                  "verticalBottomSame",
                  "verticalTopLeftOpposite",
                  "verticalBottomLeftOpposite",
                  "horizontalLeftSame",
                  "horizontalRightSame",
                  "horizontalTopLeftOpposite",
                  "horizontalTopRightOpposite",
                  "diagonalBottomLeftOpposite",
                  "diagonalTopLeftOpposite"]],

                ["boxSnakesWipe",
                 ["twoBoxTop",
                  "twoBoxLeft",
                  "twoBoxRight",
                  "fourBoxVertical",
                  "fourBoxHorizontal"]],

                ["waterfallWipe",
                 ["verticalLeft",
                  "verticalRight",
                  "horizontalLeft",
                  "horizontalRight"]],

               ["pushWipe",
                 ["fromLeft",
                  "fromTop",
                  "fromRight",
                  "fromBottom",
                  "fromBottomRight",
                  "fromBottomLeft",
                  "fromTopRight",
                  "fromTopLeft",
                  "combHorizontal",
                  "combVertical"]],

                ["slideWipe",
                 ["fromLeft",
                  "fromTop",
                  "fromRight",
                  "fromBottom"]],

                ["fade",
                 ["crossfade",
                  "fadeToColor",
                  "fadeFromColor",
                  "fadeOverColor"]],

                ["randomBarWipe",
                 ["vertical",
                  "horizontal"]],

                ["checkerBoardWipe",
                 ["down",
                  "across"]],

                ["dissolve",
                 ["default"]]
];

$propertiesRef = [
                    [ "value", "color", "#000000", "#FF0000" ],

                    [ "string", "font-family",  "Helvetica",  "Times New Roman" ],

                    [ "value", "font-size", "1pt", "1.5pt" ],

                    [ "string", "font-style", "normal", "italic" ],

                    [ "string", "text-underline", "none", "solid" ],

                    [ "string", "font-weight", "normal", "bold" ],

                    [ "value", "fill-color", "#000000", "#00FF00" ],

                    [ "string", "fill", "none", "solid" ],

                    [ "value", "height", "0.5*height", "height" ],

                    [ "value", "stroke-color", "#000000", "#0000FF" ],

                    [ "string", "stroke", "none", "solid" ],

                    [ "value", "opacity", "0.0", "0.9" ],

                    [ "value", "rotate", "0", "90" ],

                    [ "value", "skewX", "0", "-1" ],

                    [ "value", "skewY", "0", "-1" ],

                    [ "string", "visibility", "hidden", "visible" ],

                    [ "value", "width", "0.5*width", "width" ],

                    [ "value", "x", "x-0.1", "x+0.1" ],

                    [ "value", "y", "y-0.1", "y+0.1" ]
    ];

$transformsRef = [
    ["translate", "0.5*width,0.5*height"],
    ["scale", "0.5*width,0.5*height"],
    ["rotate", "270"],
    ["skewX", "-1"],
    ["skewY", "1"]
];


###############################################################################
#   Main
###############################################################################

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

$transitionNum=0;
writeSlideHeader($transitionNum, "Transition effects");

foreach $transitionRef (@$transitionsRef)
{
    $transitionType = @$transitionRef[0];

    foreach $subtype (@{$transitionRef->[1]})
    {
        writeTransitionAnimation($transitionNum,
                                 $transitionType,
                                 $subtype);
    }
}

writeSlideFooter();

writeSlideHeader(++$transitionNum, "Property effects");

foreach $propertyRef (@$propertiesRef)
{
    $propertyType = @$propertyRef[0];

    if( $propertyType eq "value" )
    {
        writePropertyAnimation( $transitionNum, @$propertyRef[1], @$propertyRef[2], @$propertyRef[3] );
    }
    elsif( $propertyType eq "string" )
    {
    }
    else
    {
        die "Unexpected case";
    }
}

writeSlideFooter();

writeSlideHeader(++$transitionNum, "Transformation effects");

foreach $transformRef (@$transformsRef)
{
    writeTransformAnimation( $transitionNum, @$transformRef[0], @$transformRef[1] );
}

writeSlideFooter();

writeSlideHeader(++$transitionNum, "Path motion effects");
writePathMotionAnimation($transitionNum);
writeSlideFooter();

    print $OUT qq~
    <draw:page draw:name="page1" draw:style-name="dp1" draw:master-page-name="Default">
       <office:forms form:automatic-focus="false" form:apply-design-mode="false"/>
       <draw:rect draw:style-name="gr1" draw:text-style-name="P1" draw:id="id10000" draw:layer="layout" svg:width="17.5cm" svg:height="13cm" svg:x="5cm" svg:y="4cm">
        <text:p text:style-name="P2">Slide: 4</text:p>
        <text:p text:style-name="P2">Topic: Misc effects</text:p>
        <text:p text:id="textid10001" text:style-name="P2">Some text to show accelerate effects</text:p>
        <text:p text:id="textid10002" text:style-name="P2">Some text to show decelerate effects</text:p>
        <text:p text:id="textid10003" text:style-name="P2">Some text to show additive effects</text:p>
        <text:p text:id="textid10004" text:style-name="P2">Some text to show autoreverse effects</text:p>
        <text:p text:id="textid10005" text:style-name="P2">Some text to show key value effects</text:p>
        <text:p text:id="textid10006" text:style-name="P2">Some text to show discrete key value effects</text:p>
        <text:p text:id="textid10007" text:style-name="P2">Some text to show formula effects</text:p>
       </draw:rect>
      <anim:par presentation:node-type="timing-root">
        <anim:seq presentation:node-type="main-sequence">

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10001" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animate smil:begin="0s" smil:dur="10s" smil:fill="hold" smil:targetElement="textid10001" smil:accelerate="0.5" smil:attributeName="x" smil:by="0.3" presentation:additive="base"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10002" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animate smil:begin="0s" smil:dur="10s" smil:fill="hold" smil:targetElement="textid10002" smil:decelerate="0.5" smil:attributeName="x" smil:by="0.3" presentation:additive="base"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10003" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animate smil:begin="0s" smil:dur="3s" smil:fill="hold" smil:targetElement="textid10003" smil:attributeName="x" smil:to="0.3" presentation:additive="sum"/>
            <anim:animate smil:begin="0s" smil:dur="6s" smil:fill="hold" smil:targetElement="textid10003" smil:attributeName="x" smil:to="0.3" presentation:additive="sum"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10004" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animate smil:begin="0s" smil:dur="5s" smil:fill="hold" smil:targetElement="textid10004" smil:attributeName="y" smil:from="0.3" smil:to="0.8" smil:autoReverse="true" presentation:additive="base"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10005" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animateTransform smil:begin="0s" smil:dur="10s" smil:fill="hold" smil:targetElement="textid10005" smil:values="0.5,0.5;0.8,0.5;0.8,0.8;0.5,0.5" smil:keyTimes="0;0.3;0.6;1" presentation:additive="base" svg:type="translate"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10006" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animateTransform smil:begin="0s" smil:dur="10s" smil:fill="hold" smil:targetElement="textid10006" smil:values="0.5,0.5;0.8,0.5;0.8,0.8;0.5,0.5" smil:keyTimes="0;0.3;0.6;1" smil:calcMode="discrete" presentation:additive="base" svg:type="translate"/>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:targetElement="textid10007" smil:attributeName="visibility" smil:to="visible"/>
            <anim:animate smil:begin="0s" smil:dur="3s" smil:fill="hold" smil:targetElement="textid10007" smil:attributeName="y" smil:values="0;1" smil:keyTimes="0;1" anim:formula="y+0.3*height*sin(5*pi*\$)" presentation:additive="base"/>
          </anim:par>
~;

writeSlideFooter();

# iterate, single paragraphs, word, lines, sentences, characters

    print $OUT qq~
    <draw:page draw:name="page1" draw:style-name="dp1" draw:master-page-name="Default">
       <office:forms form:automatic-focus="false" form:apply-design-mode="false"/>
       <draw:rect draw:style-name="gr1" draw:text-style-name="P1" draw:id="id20000" draw:layer="layout" svg:width="17.5cm" svg:height="13cm" svg:x="5cm" svg:y="4cm">
        <text:p text:style-name="P2">Slide: 5</text:p>
        <text:p text:style-name="P2">Topic: Text effects</text:p>
        <text:p text:id="textid20001" text:style-name="P2">Some text to show iterated single paragraph</text:p>
        <text:p text:id="textid20002" text:style-name="P2">Some text to show iterated word-by-word effects</text:p>
        <text:p text:id="textid20003" text:style-name="P2">Some text to show iterated letter-by-letter effects</text:p>
        <text:p text:id="textid20004" text:style-name="P2">Some more text</text:p>
        <text:p text:id="textid20005" text:style-name="P2">Some more text</text:p>
        <text:p text:id="textid20006" text:style-name="P2">Some more text</text:p>
        <text:p text:id="textid20007" text:style-name="P2">Some more text</text:p>
        <text:p text:id="textid20008" text:style-name="P2">Some more text</text:p>
       </draw:rect>
      <anim:par presentation:node-type="timing-root">
        <anim:seq presentation:node-type="main-sequence">

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:iterate smil:begin="0s" smil:fill="hold" smil:targetElement="id20000" anim:iterate-type="by-paragraph" anim:iterate-interval="0.2s">
               <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:attributeName="visibility" smil:to="visible"/>
               <anim:animate smil:begin="0s" smil:dur="2s" smil:fill="hold" smil:decelerate="0.5" smil:attributeName="x" smil:from="1.0" smil:to="x" presentation:additive="base"/>
            </anim:iterate>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:targetElement="id20000" smil:fill="hold" smil:attributeName="visibility" smil:to="visible"/>
            <anim:iterate smil:begin="0s" smil:fill="hold" smil:targetElement="textid20002" anim:iterate-type="by-word" anim:iterate-interval="0.2s">
               <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:attributeName="visibility" smil:to="visible"/>
               <anim:animate smil:begin="0s" smil:dur="2s" smil:fill="hold" smil:decelerate="0.5" smil:attributeName="x" smil:from="1.0" smil:to="x" presentation:additive="base"/>
            </anim:iterate>
          </anim:par>

          <anim:par smil:begin="0s" smil:fill="remove">
            <anim:set smil:begin="0s" smil:dur="0.001s" smil:targetElement="id20000" smil:fill="hold" smil:attributeName="visibility" smil:to="visible"/>
            <anim:iterate smil:begin="0s" smil:fill="hold" smil:targetElement="textid20003" anim:iterate-type="by-letter" anim:iterate-interval="0.2s">
               <anim:set smil:begin="0s" smil:dur="0.001s" smil:fill="hold" smil:attributeName="visibility" smil:to="visible"/>
               <anim:animate smil:begin="0s" smil:dur="2s" smil:fill="hold" smil:decelerate="0.5" smil:attributeName="x" smil:from="1.0" smil:to="x" presentation:additive="base"/>
            </anim:iterate>
          </anim:par>
~;

writeSlideFooter();

writeFooter();

$OUT->close;

zip_dirtree ($global_output_name);

