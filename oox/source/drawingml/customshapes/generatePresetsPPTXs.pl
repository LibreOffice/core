#!/usr/bin/env perl
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

use IO::Handle;

$shapes = loadShapes ();

generatePPTXs($shapes);
exit;


sub generatePPTXs
{
	$shapes = shift;

	foreach $shape (keys %$shapes)
	{
		generatePPTX ($shapes, $shape);
	}

	generatePPTX ($shapes);
}

sub generatePPTX
{
	$shapes = shift;
	$type = shift;

	mkdir ("cshape");
	mkdir ("pptx");
	system ("unzip -qq -o -d cshape cshape.pptx");

	# custom shape(s) slide with preset definition
	generateSlide ($shapes, $type, ">cshape/ppt/slides/slide1.xml", 0);

	$pptx = "../pptx/cshape-" . (defined $type ? $type : "all") . ".pptx";
	system ("cd cshape\nrm -rf ". $pptx . "\nzip -q -r " . $pptx . " .\ncd ..");

	# preset(s) slide, for testing
	generateSlide ($shapes, $type, ">cshape/ppt/slides/slide1.xml", 1);

	$pptx = "../pptx/preset-cshape-" . (defined $type ? $type : "all") . ".pptx";
	system ("cd cshape\nrm -rf ". $pptx . "\nzip -q -r " . $pptx . " .\ncd ..");
}

sub loadShapes()
{
	open (IN, "<presetShapeDefinitions.xml");

	$inside = false;
	my %shapes;

	while (<IN>)
	{
		if (/^  <[^\/]/)
		{
			$inside = true;
			@definition = ();
		}
		else
		{
			if (/^  <\//)
			{
				chomp;
				s/^  <\/([^>]+)>.*/$1/;
				undef $inside;
				$shapes{$_} = [ @definition ];
				#print "added ", $_, "\n";
			}
			else
			{
				if ($inside)
				{
					push @definition, $_;
				}
			}
		}
	}

	close (IN);

	return \%shapes;
}

sub generateSlide
{
	$shapes = shift;
	$type = shift;
	$file = shift;
	$preset = shift;

	open (OUT, $file);

	print OUT "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<p:sld xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">
  <p:cSld>
    <p:spTree>
      <p:nvGrpSpPr>
        <p:cNvPr id=\"1\" name=\"\"/>
        <p:cNvGrpSpPr/>
        <p:nvPr/>
      </p:nvGrpSpPr>
      <p:grpSpPr>
        <a:xfrm>
          <a:off x=\"0\" y=\"0\"/>
          <a:ext cx=\"0\" cy=\"0\"/>
          <a:chOff x=\"0\" y=\"0\"/>
          <a:chExt cx=\"0\" cy=\"0\"/>
        </a:xfrm>
      </p:grpSpPr>
";

	$id = 16;
	$col = 0;
	$row = 0;
	$size = 500000;
	foreach $shape (keys %$shapes)
	{
		if (defined $type) {
			if ($shape ne $type) { next; }
#            <a:ext cx=\"1050925\" cy=\"457200\"/>
#            <a:ext cx=\"1000000\" cy=\"1000000\"/>
			$size *= 10;
			$col = 0.5;
			$row = 0.25;
		}

		if ($col > 15) {
			$col = 0;
			$row ++;
		}

		print OUT "      <p:sp>
        <p:nvSpPr>
          <p:cNvPr id=\"", $id++, "\" name=\"", $shape, "\"/>
          <p:cNvSpPr/>
          <p:nvPr/>
        </p:nvSpPr>
        <p:spPr bwMode=\"auto\">
          <a:xfrm>
            <a:off x=\"" . (350000 + $col++*$size) . "\" y=\"" . (450000 + $row*$size) . "\"/>
            <a:ext cx=\"" . (4*$size/5) . "\" cy=\"" . (4*$size/5) . "\"/>
          </a:xfrm>
";
		if ($preset) {
		    print OUT "          <a:prstGeom prst=\"" . $shape . "\"><a:avLst/></a:prstGeom>
";
		} else {
		    print OUT "          <a:custGeom>
";
		    print OUT @{$shapes->{$shape}};
		    print OUT "          </a:custGeom>
";
		}
		print OUT "          <a:solidFill>
            <a:srgbClr val=\"FFFF7F\"/>
          </a:solidFill>
          <a:ln w=\"19080\">
            <a:solidFill>
              <a:srgbClr val=\"A0A060\"/>
            </a:solidFill>
            <a:miter lim=\"800000\"/>
            <a:headEnd/>
            <a:tailEnd/>
          </a:ln>
        </p:spPr>
      </p:sp>
";
	}

	print OUT "    </p:spTree>
  </p:cSld>
  <p:clrMapOvr>
    <a:masterClrMapping/>
  </p:clrMapOvr>
  <p:timing>
    <p:tnLst>
      <p:par>
        <p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"/>
      </p:par>
    </p:tnLst>
  </p:timing>
</p:sld>
";

	close (OUT);
}
