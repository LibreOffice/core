#!/usr/bin/env php
<?php

function getFilesList($directory, $extension) {
    $arrayItems = array();
    $skipByExclude = false;
    $handle = opendir($directory);
    if ($handle) {
        while (false !== ($file = readdir($handle))) {
            if ($file !== "." && $file !== "..") {
                if (is_dir($directory. DIRECTORY_SEPARATOR . $file)) {
                    $arrayItems = array_merge($arrayItems, getFilesList($directory. DIRECTORY_SEPARATOR . $file, $extension));
                } else {
                    $file = $directory . DIRECTORY_SEPARATOR . $file;
                    if (substr($file, -4) === $extension)
                        $arrayItems[] = $file;
                }
            }
        }
    }
    closedir($handle);
    return $arrayItems;
}

function addMode(&$mode, $haystack, $needle, $letter) {
    $mode .= (strpos($haystack, $needle) === FALSE) ? " " : $letter;
}

function analyseFile($fileName) {
    global $allSlots;

    $lines = file($fileName);
    $index = 0;
    $count = count($lines);
    while ($index<$count) {
        $aLine = $lines[$index];
        if(substr($aLine, 0, 12) === "// Slot Nr. ") {
            $tmp = explode(':', $aLine);
            $slotId = trim($tmp[1]);
            $index++;
            $aLine = $lines[$index];
            $isEnumSlot = strpos($aLine, 'SFX_NEW_SLOT_ENUM') !== FALSE;
            $tmp = explode(',', $aLine);
            $slotRID = $tmp[1];

            $index += $isEnumSlot ? 4 : 3;
            $aLine = $lines[$index];
            $mode = "";
            addMode($mode, $aLine, "CACHABLE",      "C");
            addMode($mode, $aLine, "AUTOUPDATE",    "U");
            addMode($mode, $aLine, "MENUCONFIG",    "M");
            addMode($mode, $aLine, "TOOLBOXCONFIG", "T");
            addMode($mode, $aLine, "ACCELCONFIG",   "A");
            $index += $isEnumSlot ? 1 : 3;
            $aLine = $lines[$index];
            if (strpos($aLine, '"') === FALSE) {
                $index++;
                $aLine = $lines[$index];
            }
            $tmp = explode('"', $aLine);
            $slotName = ".uno:" . $tmp[1];
            if (array_key_exists($slotName, $allSlots))
                $slotDescription = $allSlots[$slotName];
            else
                $slotDescription = "No description";
            if (!array_key_exists($slotName, $allSlots)) {
                $allSlots[$slotName] = new StdClass;
                $allSlots[$slotName]->slotId = $slotId;
                $allSlots[$slotName]->slotRID = $slotRID;
                $allSlots[$slotName]->mode = $mode;
                $allSlots[$slotName]->slotDescription = "";
            }
        }
        $index++;
    }
}

function analyseXCU($fileName)
{
    global $allSlots;

    $lines = file($fileName);
    $index = 0;
    $count = count($lines);

    while ($index < $count)
    {
        $aLine = $lines[$index];
        if (strpos($aLine, '<node oor:name=".uno:') !== FALSE) {
            $tmp = explode( '"', $aLine );
            $slotName = $tmp[1];
            $found = false;
            while ($index < $count
                && strpos($aLine, '<value xml:lang="en-US">') === FALSE ) {
                $index++;
                $aLine = $lines[$index];
            }
            $aLine = str_replace('<value xml:lang="en-US">', '', $aLine);
            $aLine = trim(str_replace('</value>', '', $aLine));
            if (array_key_exists($slotName, $allSlots))
                $allSlots[$slotName]->slotDescription = str_replace('~', '', $aLine);
        }
        $index++;
    }
}


if (count($argv) != 2) {
    print "Syntax error: ids.php module\n\n";
    print "  Module is one of:\n";
    print "     basslots\n";
    print "     scslots\n";
    print "     sdgslots\n";
    print "     sdslots\n";
    print "     sfxslots\n";
    print "     smslots\n";
    print "     svxslots\n";
    print "     swslots\n";
    exit(1);
}

$allSlots = array();

foreach (getFilesList("./workdir/SdiTarget", ".hxx") as $sdiFile)
    if (basename($sdiFile, ".hxx") === $argv[1])
        analyseFile($sdiFile);

foreach (getFilesList("officecfg/registry/data/org/openoffice/Office/UI", ".xcu") as $xcuFile)
    analyseXCU($xcuFile);

ksort($allSlots);
foreach ($allSlots as $name => $props) {
    printf("|-\n| %s\n| %s\n| %d\n| %s\n| %s\n",
        $name, $props->slotRID, $props->slotId, $props->mode, $props->slotDescription);
}
print ("|-\n");

?>
