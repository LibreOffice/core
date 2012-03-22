#!/usr/bin/php4 
<?php
/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/
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
