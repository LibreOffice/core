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
~;

}

sub writeSlideStyles
{
    my $mode = pop @_;
    my $direction = pop @_;
    my $transitionSubType = pop @_;
    my $transitionType = pop @_;
    my $slideNum = pop @_;

    print $OUT "  <style:style style:name=\"dp",$slideNum,"\" style:family=\"drawing-page\">\n";
    print $OUT "   <style:drawing-page-properties presentation:transition-type=\"automatic\" presentation:duration=\"PT00H00M01S\" presentation:background-visible=\"true\" presentation:background-objects-visible=\"true\" draw:fill=\"solid\" draw:fill-color=\"#ff",$slideNum % 2 ? "ff99" : "cc99","\" smil:type=\"",$transitionType,"\" smil:subtype=\"",$transitionSubType,"\" ",$direction == 0 ? "" : "smil:direction=\"reverse\" ",$mode == 0 ? "" : "smil:mode=\"out\"","/>\n";
    print $OUT "  </style:style>\n";
}

sub writeIntermediate
{
    print $OUT qq~  <style:style style:name="gr1" style:family="graphic">
   <style:graphic-properties style:protect="size"/>
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

sub writeSlide
{
    my $mode = pop @_;
    my $direction = pop @_;
    my $transitionSubtype = pop @_;
    my $transitionType = pop @_;
    my $slideNum = pop @_;

    print $OUT "   <draw:page draw:name=\"page",$slideNum,"\" draw:style-name=\"dp",$slideNum,"\" draw:master-page-name=\"Default\" presentation:presentation-page-layout-name=\"AL1T19\">";

    print $OUT "    <draw:frame presentation:style-name=\"pr1\" draw:layer=\"layout\" svg:width=\"25.199cm\" svg:height=\"3.256cm\" svg:x=\"1.4cm\" svg:y=\"0.962cm\" presentation:class=\"title\">\n";
    print $OUT "     <draw:text-box>\n";
    print $OUT "      <text:p text:style-name=\"P1\">Transition “",$slideNum-1,"”</text:p>\n";
    print $OUT "     </draw:text-box>\n";
    print $OUT "    </draw:frame>\n";
    print $OUT "    <draw:frame presentation:style-name=\"pr2\" draw:layer=\"layout\" svg:width=\"25.199cm\" svg:height=\"13.609cm\" svg:x=\"1.4cm\" svg:y=\"4.914cm\" presentation:class=\"outline\">\n";
    print $OUT "     <draw:text-box>\n";
    print $OUT "      <text:list text:style-name=\"L2\">\n";
    print $OUT "       <text:list-item>\n";
    print $OUT "        <text:p text:style-name=\"P2\">Transition: ",$transitionType,"</text:p>\n";
    print $OUT "       </text:list-item>\n";
    print $OUT "      </text:list>\n";
    print $OUT "      <text:list text:style-name=\"L2\">\n";
    print $OUT "       <text:list-item>\n";
    print $OUT "        <text:list>\n";
    print $OUT "         <text:list-item>\n";
    print $OUT "          <text:p text:style-name=\"P3\">Subtype: ",$transitionSubtype,"</text:p>\n";
    print $OUT "         </text:list-item>\n";
    print $OUT "        </text:list>\n";
    print $OUT "       </text:list-item>\n";
    print $OUT "      </text:list>\n";
    print $OUT "      <text:list text:style-name=\"L2\">\n";
    print $OUT "       <text:list-item>\n";
    print $OUT "        <text:list>\n";
    print $OUT "         <text:list-item>\n";
    print $OUT "          <text:p text:style-name=\"P3\">Direction: ",$direction == 0 ? "forward" : "reverse","</text:p>\n";
    print $OUT "         </text:list-item>\n";
    print $OUT "        </text:list>\n";
    print $OUT "       </text:list-item>\n";
    print $OUT "      </text:list>\n";
    print $OUT "      <text:list text:style-name=\"L2\">\n";
    print $OUT "       <text:list-item>\n";
    print $OUT "        <text:list>\n";
    print $OUT "         <text:list-item>\n";
    print $OUT "          <text:p text:style-name=\"P3\">Mode: ",$mode == 0 ? "in" : "out","</text:p>\n";
    print $OUT "         </text:list-item>\n";
    print $OUT "        </text:list>\n";
    print $OUT "       </text:list-item>\n";
    print $OUT "      </text:list>\n";
    print $OUT "     </draw:text-box>\n";
    print $OUT "    </draw:frame>\n";
    print $OUT "    <presentation:notes draw:style-name=\"dp2\">\n";
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
                  "fromBottom",
                  "fromBottomRight",
                  "fromBottomLeft",
                  "fromTopRight",
                  "fromTopLeft"]],

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

$slideNum=1;
foreach $transitionRef (@$transitionsRef)
{
    $transitionType = @$transitionRef[0];

    foreach $subtype (@{$transitionRef->[1]})
    {
        if( $global_gen_all != 0 )
        {
            writeSlideStyles($slideNum++,
                             $transitionType,
                             $subtype,
                             0, 0);
            writeSlideStyles($slideNum++,
                             $transitionType,
                             $subtype,
                             1, 0);
            writeSlideStyles($slideNum++,
                             $transitionType,
                             $subtype,
                             0, 1);
            writeSlideStyles($slideNum++,
                             $transitionType,
                             $subtype,
                             1, 1);
        }
        else
        {
            writeSlideStyles($slideNum++,
                             $transitionType,
                             $subtype,
                             0, 0);
        }
    }
}

writeIntermediate();

$slideNum=1;
foreach $transitionRef (@$transitionsRef)
{
    $transitionType = @$transitionRef[0];

    foreach $subtype (@{$transitionRef->[1]})
    {
        if( $global_gen_all != 0 )
        {
            writeSlide($slideNum++,
                       $transitionType,
                       $subtype,
                       0, 0);
            writeSlide($slideNum++,
                       $transitionType,
                       $subtype,
                       1, 0);
            writeSlide($slideNum++,
                       $transitionType,
                       $subtype,
                       0, 1);
            writeSlide($slideNum++,
                       $transitionType,
                       $subtype,
                       1, 1);
        }
        else
        {
            writeSlide($slideNum++,
                       $transitionType,
                       $subtype,
                       0, 0);
        }
    }
}

writeFooter();

$OUT->close;

zip_dirtree ($global_output_name);

