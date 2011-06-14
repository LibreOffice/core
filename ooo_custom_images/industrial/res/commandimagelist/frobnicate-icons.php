#!/usr/bin/php4 
<?php
#$renderer = "rsvg";
$renderer = "inkscape";

/*

If you're a hacker, please look away

*/

$render_sizes = array("16"=>"sc_","24"=>"lc_");
$render_dpis = array("16"=>"60", "24"=>"90");

$svgs = `find . -name "*.svg"`;
$svgs = explode("\n",$svgs);

echo "* rendering PNGs\n\n";
foreach ($svgs as $line) {
	if ($line) {
		$file = eregi_replace("^./lc_(.*)\.svg","\\1",$line);
		//echo "\n" .  $file . "\n\n";	
		foreach ($render_sizes as $size => $prefix) {
			$SVG = "lc_$file.svg";
			$PNG = "$prefix$file.png";
			//delete older version
			if (file_exists($PNG)) unlink($PNG);
			echo "$SVG => $PNG\n";
			if ($renderer == "inkscape") {
				$exec = "inkscape -z -e $PNG -f $SVG ";
				$exec .="-w $size -h $size";
				exec($exec);
			} else {
				exec("rsvg -w $size -h $size $SVG $PNG\n");
			}
		}
	}
}

exit;
?>
